<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 03:01:00 PM
	 *
	 * comment.inc - story comment functionality
	 */

	define("E_NEWSYS_COM_COM",		1,TRUE);	# No comment
	define("E_NEWSYS_COM_SUBJECT",		2,TRUE);	# No subject
	define("E_NEWSYS_COM_NO_STORY",		4,TRUE);	# Story ID is invalid
	define("E_NEWSYS_COM_NOT_ALLOWED",	8,TRUE);	# Commenting not allowed
	define("E_NEWSYS_COM_MAX_EXCEED",	16,TRUE);	# Maximum number of comments exceeded

	function newsys_comment_add($dbh,$comment)
	{
		$comment["story_id"]	= (int)$comment["story_id"];

		# Validate story identifier
		if
		(
			!$dbh->query("	SELECT
						story_id
					FROM
						newsys_stories
					WHERE
						story_id = {$comment["story_id"]}",DB_COL)
		)
			return array(E_NEWSYS_COM_NO_STORY,array());

		$story		= newsys_story_get($dbh,$comment['story_id']);
		$max_comments	= newsys_conf('max_comments');

		# Validate comment
		$err = E_NEWSYS_NONE;

		if (!$comment["comment"])					$err |= E_NEWSYS_COM_COM;
		if (!$comment["subject"])					$err |= E_NEWSYS_COM_SUBJECT;
		if (!newsys_conf("allow_comments"))				$err |= E_NEWSYS_COM_NOT_ALLOWED;
		if ($max_comments && $story['num_comments'] >= $max_comments)	$err |= E_NEWSYS_COM_MAX_EXCEED;

		# Array containing ids for each field with invalid input
		$user_errors = newsys_fields_validate(NEWSYS_FLD_COMMENT,$comment);

		if ($err != E_NEWSYS_NONE || count($user_errors))
			return array($err,$user_errors);

	#	$comment["story_id"]	= (int)$comment["story_id"];
		$comment["author_id"]	= (int)$comment["author_id"];
		$comment["comment"]	= $dbh->prepare_str(newsys_str_parse($comment["comment"]),SQL_REG);

		# Grab unique comment identifier
		$comment["comment_id"]	= 1 + $dbh->query("	SELECT
									comment_id
								FROM
									newsys_comments
								WHERE
									story_id = ${comment["story_id"]}
								ORDER BY
									comment_id	DESC
								LIMIT
									1",DB_COL);

		$sql_fields		= "";
		$sql_values		= "";

		foreach (newsys_conf("comment_fields") as $user_field_id => $user_field)
		{
			$comment[$user_field_id] = $dbh->prepare_str(newsys_str_parse($comment[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql_fields .= ",ud_$user_field_id";
			$sql_values .= ",'{$comment[$user_field_id]}'";
		}

		# Update database
		$dbh->query("	INSERT INTO newsys_comments
				(
					story_id,
					comment_id,
					author_id,
					subject,
					comment,
					create_date,
					parent_comment_id
					$sql_fields
				) VALUES (
					{$comment["story_id"]},
					{$comment["comment_id"]},
					{$comment["author_id"]},
					'{$comment["subject"]}',
					'{$comment["comment"]}',
					NOW(),
					{$comment["parent_comment_id"]}
					$sql_values
				)",DB_NULL);

		$dbh->query("	UPDATE
					newsys_stories
				SET
					num_comments	= num_comments + 1
				WHERE
					story_id	= {$comment["story_id"]}",DB_NULL);

		# Update cache
		if (newsys_conf("use_cache"))
		{
			/*
			 * If the parent is a comment, link to it, otherwise
			 * it must be a direct comment to a story, the case
			 * in which it must be linked to the story
			 */
			if (newsys_conf("hier_comments") && $comment['parent_comment_id'])
				newsys_cache_add_info($comment["comment_id"],"comment",$comment["story_id"],$comment["parent_comment_id"]);
			else
				newsys_cache_add_info($comment["comment_id"],"story",$comment["story_id"]);

		}

		return array(E_NEWSYS_NONE,array());
	}

	function newsys_comment_get_ancestors($needle,$haystack)
	{
		$ancestors	= array($needle);
		$continue	= 1;

		while ($continue)
		{
			$continue = 0;

			foreach ($haystack as $parent => $children)
				foreach ($children as $child)
					if ($child == $needle)
					{
						array_push($ancestors,$parent);

						# Try again for the parent
						$needle		= $parent;
						$continue	= 1;

						break 2;
					}
		}

		return $ancestors;
	}

	function newsys_comment_delete($dbh,$story_id,$comment_id)
	{
		$story_id	= (int)$story_id;
		$comment_id	= (int)$comment_id;

		# Validate comment
		$comment	= newsys_comment_get($dbh,$story_id,$comment_id);

		if (!is_array($comment))
			return;

		# Grab all comments
		$dbh->query("	SELECT
					comment_id,
					parent_comment_id
				FROM
					newsys_comments
				WHERE
					story_id = $story_id",DB_ROWS);

		$comments	= array();
		$all_comments	= array();

		# Create a list of parent and child comments
		while (list ($i_comment_id,$i_parent_comment_id) = $dbh->fetch_row())
			if (array_key_exists($i_parent_comment_id,$all_comments))
				array_push($all_comments[$i_parent_comment_id],$i_comment_id);
			else
				$all_comments[$i_parent_comment_id] = array($i_comment_id);

		# Single out child comments to the current comment
		foreach ($all_comments as $parent => $children)
			foreach ($children as $child)
				if (in_array($comment_id,newsys_comment_get_ancestors($child,$all_comments)))
					# It's a child of the target comment
					array_push($comments,$child);

#echo "comments to remove: ";print_r($comments);

		# Remove comments
		$dbh->query("	DELETE FROM
					newsys_comments
				WHERE
					story_id	= $story_id AND
					comment_id	IN(" . join(",",$comments) . ")",DB_NULL);

		# Update num_comments in story
		$dbh->query("	UPDATE
					newsys_stories
				SET
					num_comments = num_comments - " . count($comments) . "
				WHERE
					story_id = $story_id",DB_NULL);

		# Clear out cache
		if (newsys_conf("use_cache"))
		{
			foreach ($comments as $i_id)
				newsys_cache_delete("comment",$story_id,$i_id);

			newsys_cache_remove_info($comments,"story",$story_id);

			if (newsys_conf("hier_comments") && $comment['parent_comment_id'])
				newsys_cache_remove_info($comment_id,"comment",$story_id,$comment['parent_comment_id']);
		}

		return;
	}

	function newsys_comment_update($dbh,$comment)
	{
/*
 * Should we check $story_id here for a valid story in the database?
 * Worst case: it doesn't exist, the 'UPDATE' query doesn't affect
 * anything. It all comes down to 'SELECT' vs. 'UPDATE,' except in the case
 * where both queries will be made.
 */
		# Validate comment fields
		$err = E_NEWSYS_NONE;

		if (!$comment["comment"]) $err |= E_NEWSYS_COM_COM;
		if (!$comment["subject"]) $err |= E_NEWSYS_COM_SUBJECT;

		# Array containing IDs for each field with invalid input
		$user_errors = newsys_fields_validate(NEWSYS_FLD_COMMENT,$comment);

		if ($err != E_NEWSYS_NONE || count($user_errors))
			return array($err,$user_errors);

		$comment["story_id"]	= (int)$comment["story_id"];
		$comment["comment_id"]	= (int)$comment["comment_id"];
		$comment["comment"]	= $dbh->prepare_str(newsys_str_parse($comment["comment"]),SQL_REG);

		# Update database
		$sql = "	UPDATE
					newsys_comments
				SET
					mod_date		= NOW(),";

		foreach (newsys_conf("comment_fields") as $user_field_id => $user_field)
		{
			$comment[$user_field_id] = $dbh->prepare_str(newsys_str_parse($comment[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql .= "	ud_$user_field_id	= '{$comment[$user_field_id]}',";
		}

		$sql .= "
					comment			= '{$comment["comment"]}'
				WHERE
					story_id		= {$comment["story_id"]}
				AND	comment_id		= {$comment["comment_id"]}";

		# Update database
		$dbh->query($sql,DB_NULL);

		# Update cache
		if (newsys_conf("use_cache"))
			newsys_cache_expire("comment",$comment["story_id"],$comment["comment_id"]);

		return array(E_NEWSYS_NONE,array());
	}

	function newsys_comment_get($dbh,$story_id,$comment_id)
	{
		$story_id	= (int)$story_id;
		$comment_id	= (int)$comment_id;

		$comment	= $dbh->query("	SELECT
							*
						FROM
							newsys_comments
						WHERE
							story_id	= $story_id AND
							comment_id	= $comment_id",DB_ROW);

		if (is_array($comment))
		{
			$data = array();

			foreach ($comment as $id => $val)
				if (preg_match("/^ud_/",$id))
					$data[preg_replace("/^ud_/","",$id)] = $val;
				else
					$data[$id] = $val;

			$comment = $data;
		}

		return $comment;
	}

	function newsys_get_comment_comments($story_id,$comment_id)
	{
		$story_id	= (int)$story_id;
		$comment_id	= (int)$comment_id;
		$ids		= array();

		if (newsys_conf("use_cache"))
		{
			# See if we can use the cached version
			$ids = @file(newsys_conf("cache_dir") . "/info-comment-$story_id-$comment_id");

			if (!is_array($ids))
				$ids = array();

			# Strip newlines
			foreach ($ids as $key => $value)
				$ids[$key] = $value;

			# IDs should be in reverse order
			$ids = array_reverse($ids);
		} else {
			# Fall back to the database
			$dbh = newsys_get_dbh();

			$dbh->query("	SELECT
						comment_id
					FROM
						newsys_comments
					WHERE
						story_id		= $story_id
					AND	parent_comment_id	= $comment_id
					ORDER BY
						create_date DESC",DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);
		}

		return $ids;
	}
?>
