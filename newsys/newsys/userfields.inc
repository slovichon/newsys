<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 03:58:48 PM
	 *
	 * userfields.inc - routines for handling
	 * user-defined fields in stories, comments,
	 * user profiles, etc.
	 */
	define("NEWSYS_FLD_USER",	1,TRUE);
	define("NEWSYS_FLD_STORY",	2,TRUE);
	define("NEWSYS_FLD_COMMENT",	3,TRUE);

	function newsys_fields_validate($type,$data)
	{
		$var	= NULL;
		$errors	= array();

		switch ($type)
		{
			case NEWSYS_FLD_USER:		$var = newsys_conf("user_fields");	break;
			case NEWSYS_FLD_STORY:		$var = newsys_conf("story_fields");	break;
			case NEWSYS_FLD_COMMENT:	$var = newsys_conf("comment_fields");	break;
			default:
				newsys_handle_error("Invalid type to newsys_validate_fields(); type: $type");
		}

/*
 * A little extra processing here will actually on
 * average save more overall, since the script will
 * be executed multiple times between fixing errors
 * in input, might as well just let the user know
 * everything that's wrong at the same
 */
		# Validate user-defined fields
		foreach ($var as $user_field_id => $user_field)
		{
			if ($user_field["validation"])
			{
				if (!preg_match($user_field["validation"],$data[$user_field_id]))
					if ($user_field["required"])
					{
						array_push($errors,$user_field_id);
					} else {
						$data[$user_field_id] = $user_field["default_value"];
					}
			}
		}

		return $errors;
	}

	function newsys_fields_update()
	{
		global $php_errormsg;

		$vars		=	array
					(
						'user'		=> newsys_conf("user_fields"),
						'story'		=> newsys_conf("story_fields"),
						'comment'	=> newsys_conf("comment_fields")
					);

		$skip		= FALSE;
		$cache_dir	= newsys_conf("cache_dir");

		if (newsys_conf("use_cache"))
		{
			if (file_exists("$cache_dir/info-user_fields"))
			{
				# Make sure the content has changed

				/*
				 * We're already going through the data,
				 * save it to run against the database
				 */
				$changed_vars	=	array
							(
								'user'		=> array('var' => array()),
								'story'		=> array('var' => array()),
								'comment'	=> array('var' => array())
							);
				$anomoly	= FALSE;
				$dbh		= NULL;

				$fp = @fopen("$cache_dir/info-user_fields","r+");

				if (!$fp)
					newsys_handle_error("Cannot open file; file: $cache_dir/info-user_fields");

				while ($line = fgets($fp,NEWSYS_BUF_SIZ))
				{
					$line		= chop($line);
					$old_fields	= @unserialize($line);
					$swt		= FALSE;
					$type		= "";
					$id		= "";
					$var		= "";

					if (!is_array($old_fields))
						continue;

					foreach ($vars as $id => $var)
					{
						$old_val = $anomoly;

						if ($var == $old_fields)
						{
							# Oops, could have made a mistake, reset the values
							$anomoly		= FALSE;
							$changed_vars[$id]	= array();

							# We're done with this variable
							unset($vars[$id]);
							break;
						} else {
							$anomoly		= TRUE;
							$changed_vars[$id]	= $old_fields;
						}

						$anomoly = $old_val;
					}
				}

				if ($anomoly)
				{
					fseek($fp,0,SEEK_SET);
					newsys_fields_write_cache($fp);
					newsys_fields_write_db(NULL,$changed_vars);
				} else {
					/*
					 * There haven't been any changed, no need to check
					 * the database for any changes
					 */
					$skip = TRUE;
				}

				fclose($fp);
			} else {
				/*
				 * Cache is empty we must fallback to the database's cache
				 * and rebuild the file cache regardless
				 */
				newsys_fields_write_cache();
			}
		}

		# Else it will check the database for a change
		if (!$skip)
		{
			$dbh		= newsys_get_dbh();
			$anomoly	= FALSE;
			$changed_vars	=	array
						(
							'user'		=> array(),
							'story'		=> array(),
							'comment'	=> array()
						);
			$old_vars	= array();

			list
			(
				$old_vars['user'],
				$old_vars['story'],
				$old_vars['comment']

			) = $dbh->query("	SELECT
							user_fields_sig,
							story_fields_sig,
							comment_fields_sig
						FROM
							newsys_config",DB_ROW);

			foreach ($old_vars as $id => $old_var)
			{
				$old_fields = @unserialize($old_var);

				if (!is_array($old_fields))
					$old_fields = array('var' => array());

				if ($old_fields != $vars[$id])
				{
					$anomoly = TRUE;
					$changed_vars[$id] = $old_fields;
				}
			}

			if ($anomoly)
				newsys_fields_write_db($dbh,$changed_vars);
		}

		return;
	}

	# We can optionally be given an open file handle to write
	function newsys_fields_write_cache($fp = NULL)
	{
		$already_open	= TRUE;
		$cache_dir	= newsys_conf("cache_dir");

		if (!$fp)
		{
			$fp = @fopen("$cache_dir/info-user-fields","w");

			if (!$fp)
				newsys_handle_error("Serious error! The database has been updated and the cache not be updated; file: $cache_dir/info-user-fields");

#			$already_opened = TRUE;
			$already_opened = FALSE;
		}

		$vars =	array
			(
				newsys_conf("user_fields"),
				newsys_conf("story_fields"),
				newsys_conf("comment_fields")
			);

		foreach ($vars as $var)
			fputs($fp,serialize($var) . "\n");

		# Any old data must be truncated
		if ($already_opened)
			ftruncate($fp,ftell($fp));
		else
			fclose($fp);

		return;
	}

	function newsys_fields_write_db($dbh,$old_vars)
	{
		$sigs = array();
		$vars =	array
			(
				'user'		=> array('var' => newsys_conf("user_fields"),		'db_name' => "newsys_users"),
				'story'		=> array('var' => newsys_conf("story_fields"),		'db_name' => "newsys_stories"),
				'comment'	=> array('var' => newsys_conf("comment_fields"),	'db_name' => "newsys_comments")
			);

		if (!$dbh)
			$dbh = newsys_get_dbh();

		foreach (array_keys($old_vars) as $id)
		{
			# Take note of the changed fields
			$old_fields	= $old_vars[$id]["var"];
			$removed_fields	= array();
			$var_diff	= $vars[$id]["var"];

			foreach ($old_fields as $old_id => $old_field)
			{
				if (@in_array($old_field,$var_diff))
				{
					/*
					 * This field exists in both, remove it
					 * from the diff
					 */
					foreach ($var_diff as $index => $i_field)
						if ($i_field == $old_field)
						{
							/*
							 * By removing it from the difference,
							 * only new, unique fields will exist
							 * in the difference array
							 */
							unset($var_diff[$index]);
							break;
						}
					/*
					 * Else, add it to a changed array, which
					 * will save database data yet perform
					 * minor conversions
					 */
				} else {
					array_push($removed_fields,$old_id);
				}
			}

			/*
			 * Note that we prefix user-defined fields with
			 * `ud_' as to avoid conflicts with our own fields
			 * and hopefully database-specific keywords
			 */
			foreach ($removed_fields as $removed_field)
				$dbh->query("	ALTER TABLE
							{$vars[$id]["db_name"]}
						DROP
							ud_$removed_field",DB_NULL);

			# This contains just new fields
			foreach ($var_diff as $field_id => $added_field)
				$dbh->query("	ALTER TABLE
							{$vars[$id]["db_name"]}
						ADD
							ud_$field_id {$added_field["database_type"]}",DB_NULL);

			$sigs[$id] = $dbh->prepare_str(serialize($vars[$id]),SQL_REG);
		}

		# Update signatures
		# Check to see if one already exists
		if ($dbh->query("	SELECT
						COUNT(*)
					FROM
						newsys_config",DB_COL))
		{
			$dbh->query("	UPDATE
						newsys_config
					SET
						user_fields_sig		= '{$sigs["user"]}',
						story_fields_sig	= '{$sigs["story"]}',
						comment_fields_sig	= '{$sigs["comment"]}'",DB_NULL);
		} else {
			$dbh->query("	INSERT INTO
						newsys_config
					(
						user_fields_sig,
						story_fields_sig,
						comment_fields_sig
					) VALUES (
						'{$sigs["user"]}',
						'{$sigs["story"]}',
						'{$sigs["comment"]}'
					)",DB_NULL);
		}

		return;
	}
?>
