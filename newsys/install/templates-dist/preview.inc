<?php
	$dbh		= newsys_get_dbh();
	$path		= newsys_build_path();
	$story_id	= $args[1];
	$story		= newsys_story_get($dbh,$story_id);

	if (is_array($story))
	{
		$ts = new TimeStamp($story["create_date"]);

		$date		= $ts->format(newsys_conf("time_format"));
		$num_words	= preg_match_all("/\w+/",newsys_str_parse($story["story"],NEWSYS_STR_NONE),$matches);
		$num_comments	= $dbh->query("	SELECT
							COUNT(*)
						FROM
							newsys_comments
						WHERE
							story_id = '{$story["story_id"]}'",DB_COL);
		$author		= $dbh->query("	SELECT
								username
							FROM
								newsys_users
							WHERE
								user_id = '{$story["author_id"]}'",DB_COL);
		$profile_uri	= "$path/profile.php?user_id=" . $story["author_id"];
		$story_uri	= "$path/view.php?story_id="   . $story["story_id"];
		$headline	= $story["headline"];
		$overview	= $story["overview"];

		return TRUE;
	}

	return FALSE;
?>
