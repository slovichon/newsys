<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 03:44:43 PM
	 *
	 * story.inc - story routines
	 */
	define("E_NEWSYS_STORY_HEADLINE",	1,TRUE);	# No headline
	define("E_NEWSYS_STORY_STORY",		2,TRUE);	# No story

	function newsys_rebuild_latest_stories()
	{
		global $php_errormsg;

		$dbh	= newsys_get_dbh();

		# Clear out old stories
		$dbh->query("	DELETE FROM
					newsys_last_stories",DB_NULL);

		$dbh->query("	SELECT
					author_id,
					story_id,
					create_date
				FROM
					newsys_stories
				ORDER BY
					create_date DESC
				LIMIT
					" . newsys_conf("result_limit"),DB_ROWS);

		$stories = array();

		# Refresh listing
		while ($story = $dbh->fetch_row())
			array_push($stories,$story);

		foreach ($stories as $story)
			$dbh->query("	INSERT INTO
						newsys_last_stories
					(
						author_id,
						story_id,
						create_date
					) VALUES (
						{$story["author_id"]},
						{$story["story_id"]},
						{$story["create_date"]}
					)",DB_NULL);

		# Update cache
		if (newsys_conf("use_cache"))
		{
/*
 * Should we re-compute the latest stories in the database
 * as well here to eliminate any possible discrepancies?
 */

			$fp	= @fopen(newsys_conf("cache_dir") . "/info-last_stories","w");

			if (!$fp)
				newsys_handle_error("Could not open latest stories cache file");

			# This format expects backwards data
			foreach (array_reverse($stories) as $story)
				fputs($fp,$story["story_id"] . "\n");

			fclose($fp);
		}

		return;
	}

	function newsys_get_latest_stories()
	{
		$ids = array();

		if (newsys_conf("use_cache"))
		{
			# See if we can use the cached version
			$ids = @file(newsys_conf("cache_dir") . "/info-last_stories");

			if (!is_array($ids))
				$ids = array();

			# Remove newlines from IDs
			foreach ($ids as $key => $value)
				$ids[$key] = chop($value);

			# Stories should be reversed
			$ids = array_reverse($ids);
		} else {
			# Fallback to the database
			$dbh = newsys_get_dbh();

			$dbh->query("	SELECT
						story_id
					FROM
						newsys_stories
					ORDER BY
						create_date DESC
					LIMIT
						/* This next part is just in case */
						" . newsys_conf("result_limit"),DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);
		}

		return $ids;
	}

	function newsys_story_get($dbh,$story_id)
	{
		$story_id	= (int)$story_id;
		$story		= $dbh->query("	SELECT
							*
						FROM
							newsys_stories
						WHERE
							story_id = $story_id",DB_ROW);

		if (is_array($story))
		{
			$data = array();

			# User-defined fields will have the `ud_' prefix
			foreach ($story as $id => $val)
				if (preg_match("/^ud_/",$id))
					$data[preg_replace("/^ud_/","",$id)] = $val;
				else
					$data[$id] = $val;

			$story = $data;
		}

		return $story;
	}

	function newsys_story_get_comments($story_id)
	{
		$ids		= array();
		$story_id	= (int)$story_id;

		if (newsys_conf("use_cache"))
		{
			# See if we can use the cached version
			$ids = @file(newsys_conf("cache_dir") . "/info-story-$story_id");

			if (!is_array($ids))
				$ids = array();

			# Remove newlines
			foreach ($ids as $key => $val)
				$ids[$key] = chop($val);

			# Comments should be in reverse-order
			$ids = array_reverse($ids);
		} else {
			# Fallback to the database
			$dbh = newsys_get_dbh();

			$dbh->query("	SELECT
						comment_id
					FROM
						newsys_comments
					WHERE
						story_id		= $story_id AND
						parent_comment_id	= 0",DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);
		}

		return $ids;
	}

	function newsys_story_update($dbh,$story)
	{
		$err = E_NEWSYS_NONE;

		/* Validate story fields */
		if (!$story["overview"])
		{
			/*
			 * Default actions:
			 *      1) Look for newsys_overview_break
			 *      2) Take the first 255 chars/break
			 */

			$t = preg_quote(newsys_conf("overview_break"),"/");

			if (preg_match("/^(.*?)\s*$t/",$story["story"],$matches))
			{
				/* User-defined overview break */
				$story["overview"] = $matches[1];

				/* Remove overview break from regular story */
				$story["story"] = preg_replace("/\s*$t/","",$story["story"]);
			} else {
				/*
				 * Else break it ourself after the word
				 * closest to the 255 char mark
				 */
				$story["overview"] = preg_replace("/<.*?>/","",$story["story"]);

				$broken = FALSE;

				while (strlen($story["overview"]) > 252)
				{
					$broken = TRUE;
					$story["overview"] = preg_replace("/\s*\S+\s*$/","",$story["overview"]);
				}

				if ($broken)
					$story["overview"] .= "...";
			}
		}

		if (!$story["headline"])	$err |= E_NEWSYS_STORY_HEADLINE;
		if (!$story["story"])		$err |= E_NEWSYS_STORY_STORY;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_STORY,$story);

		if ($err != E_NEWSYS_NONE || count($user_errors))
			return array($err,$user_errors);

		$story["story_id"]	= (int)$story["story_id"];
		$story["overview"]	= $dbh->prepare_str(newsys_str_parse($story["overview"],NEWSYS_STR_NONE),SQL_REG);
		$story["story"]		= $dbh->prepare_str(newsys_str_parse($story["story"]),SQL_REG);
		$story["headline"]	= $dbh->prepare_str(newsys_str_parse($story["headline"],NEWSYS_STR_NONE),SQL_REG);

		$sql = "	UPDATE
					newsys_stories
				SET
					overview		= '{$story["overview"]}',
					story			= '{$story["story"]}',
					mod_date		= NOW(),";

		foreach (newsys_conf("story_fields") as $user_field_id => $user_field)
		{
			$story[$user_field_id] = $dbh->prepare_str(newsys_str_parse($story[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql .= "	ud_$user_field_id	= '{$story[$user_field_id]}',";
		}

		$sql .= "
					headline		= '{$story["headline"]}'
				WHERE
					story_id		= {$story["story_id"]}";

		/* Update database */
		$dbh->query($sql,DB_NULL);

		$is_latest = $dbh->query("	SELECT
							author_id
						FROM
							newsys_last_stories
						WHERE
							story_id = {$story["story_id"]}",DB_COL);


		/* Update cache */
		if (newsys_conf("use_cache"))
			newsys_cache_expire("story",$story["story_id"]);

		# Update RSS info
		if ($is_latest && newsys_conf("use_rss"))
		{
/*
			$ts = new TimeStamp();
			$ts->load_current();
			$story['date'] = $ts->get_unix();

*/
			$story['date'] = time();

			newsys_xml_write
			(
				NEWSYS_XML_STORIES,
				newsys_xml_update
				(
					NEWSYS_XML_STORIES,
					$story
				)
			);
		}

		return array(E_NEWSYS_NONE,array());
	}

	function newsys_story_delete($dbh,$story_id)
	{
		$story_id	= (int)$story_id;

		$use_cache	= newsys_conf("use_cache");

		$dbh->query("	DELETE FROM
					newsys_stories
				WHERE
					story_id	= $story_id",DB_NULL);

		if ($use_cache)
		{
			/*
			 * Should we loop through all the templates and
			 * check dependencies for this story and delete
			 * all cached templates?
			 *
			 * Also -- should we update other resources this
			 * affects, such as info-user so the profile gets
			 * updated for things such as 'last story' and
			 * 'num posts'
			 */

			newsys_cache_delete("story",$story_id);

			# Gather up the comments
			$dbh->query("	SELECT
						comment_id
					FROM
						newsys_comments
					WHERE
						story_id = $story_id",DB_ROWS);

			# And remove them
			while (list ($comment_id) = $dbh->fetch_row())
				newsys_cache_delete("comment",$story_id,$comment_id);
		}

		$dbh->query("	DELETE FROM
					newsys_comments
				WHERE
					story_id	= $story_id",DB_NULL);

		$is_latest = $dbh->query("	DELETE FROM
							newsys_last_stories
						WHERE
							story_id	= $story_id",DB_NULL);

		if ($use_cache && $is_latest)
			newsys_rebuild_latest_stories();

		# Update RSS info
		if ($is_latest && newsys_conf("use_rss"))
		{
			newsys_xml_write
			(
				NEWSYS_XML_STORIES,
				newsys_xml_remove
				(
					NEWSYS_XML_STORIES,
					array
					(
						'story_id' => $story_id
					)
				)
			);
		}

		return;
	}

	function newsys_story_add($dbh,$story)
	{
		$err = E_NEWSYS_NONE;

		/* Validate story fields */
		if (!$story["overview"])
		{
			/*
			 * Default actions:
			 *	1) Look for newsys_overview_break
			 *	2) Take the first 255 chars/break
			 */

			$t = preg_quote(newsys_conf("overview_break"),"/");

			if (preg_match("/^(.*?)\s*$t/",$story["story"],$matches))
			{
				/* User-defined overview break */
				$story["overview"] = $matches[1];

				/* Remove overview break from regular story */
				$story["story"] = preg_replace("/\s*$t/","",$story["story"]);
			} else {
				/*
				 * Else break it ourself after the word
				 * closest to the 255 char mark
				 */
				$story["overview"] = preg_replace("/<.*?>/","",$story["story"]);

				$broken = FALSE;

				while (strlen($story["overview"]) > 252)
				{
					$broken = TRUE;
					$story["overview"] = preg_replace("/\s*\S+\s*$/","",$story["overview"]);
				}

				if ($broken)
					$story["overview"] .= "...";
			}
		}

		if (!$story["headline"])	$err |= E_NEWSYS_STORY_HEADLINE;
		if (!$story["story"])		$err |= E_NEWSYS_STORY_STORY;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_STORY,$story);

		if ($err != E_NEWSYS_NONE || count($user_errors))
			return array($err,$user_errors);

		$story["author_id"]	= (int)$story["author_id"];
		$story["overview"]	= $dbh->prepare_str(newsys_str_parse($story["overview"],NEWSYS_STR_NONE),SQL_REG);
		$story["story"]		= $dbh->prepare_str(newsys_str_parse($story["story"]),SQL_REG);
		$story["headline"]	= $dbh->prepare_str(newsys_str_parse($story["headline"],NEWSYS_STR_NONE),SQL_REG);

		$sql_fields		= "";
		$sql_values		= "";

		foreach (newsys_conf("story_fields") as $user_field_id => $user_field)
		{
			$story[$user_field_id] = $dbh->prepare_str(newsys_str_parse($story[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql_fields .= ",ud_$user_field_id";
			$sql_values .= ",'{$story[$user_field_id]}'";
		}

		/* Update database */
		$dbh->query("	INSERT INTO
					newsys_stories
				(
					author_id,
					overview,
					headline,
					story,
					create_date,
					allow_comments
					$sql_fields
				) VALUES (
					{$story["author_id"]},
					'{$story["overview"]}',
					'{$story["headline"]}',
					'{$story["story"]}',
					NOW(),
					" . newsys_conf("allow_comments") . " /* Default value to allow/disallow */
					$sql_values
				)",DB_NULL);

		$story_id	= $dbh->last_insert_id();

		/* Gather oldest story to remove it */
		$old_story_id	= $dbh->query("	SELECT
							story_id
						FROM
							newsys_last_stories
						ORDER BY
							create_date ASC
						LIMIT
							1",DB_COL);

		/* Add new story to latest stories */
		$dbh->query("	INSERT INTO
					newsys_last_stories
				(
					story_id,
					author_id,
					create_date
				) VALUES (
					$story_id,
					{$story["author_id"]},
					NOW()
				)",DB_NULL);

		/* Check if we're going to force a displacement */
		$total = $dbh->query("	SELECT
						COUNT(*)
					FROM
						newsys_stories",DB_NULL);

		/* Remove oldest story */
		$result_limit = newsys_conf("result_limit");

		if ($old_story_id && $total > $result_limit)
			$dbh->query("	DELETE FROM
						newsys_last_stories
					WHERE
						story_id = $old_story_id",DB_NULL);

		/* Update cache */
		if (newsys_conf("use_cache"))
		{
			newsys_cache_add_info($story_id,"last_stories");


#echo $dbh->query("SELECT COUNT(*) FROM newsys_stories",DB_COL);

			if ($old_story_id && $total > $result_limit)
				newsys_cache_remove_info($old_story_id,"last_stories");
#echo $dbh->query("SELECT COUNT(*) FROM newsys_stories",DB_COL);

		}

#echo $dbh->query("SELECT COUNT(*) FROM newsys_stories",DB_COL);

		/* Update XML */
		if (newsys_conf("use_rss"))
		{
			$new_story = $story;

			/*
			 * Don't modify the original story -- not that it would really matter
			 *
			 * Note: we have to modify it to add the not-present 'story id' and
			 * 'timestamp' fields
			 */
			$new_story['story_id']	= $story_id;
/*
			$ts			= new TimeStamp();
			$ts->load_current();
			$new_story['date']	= $ts->get_unix();

*/
			$new_story['date']	= time();

			newsys_xml_write
			(
				NEWSYS_XML_STORIES,
				newsys_xml_add
				(
					NEWSYS_XML_STORIES,
					$new_story
				)
			);
		}

		return array(E_NEWSYS_NONE,array());
	}
?>
