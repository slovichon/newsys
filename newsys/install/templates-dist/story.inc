<?php
	$dbh		= newsys_get_dbh();
	$path		= newsys_build_path();
	$story_id	= $args[1];
	$story		= newsys_story_get($dbh,$story_id);

	if (is_array($story))
	{
		$ts = new TimeStamp($story["create_date"]);

		$date		= $ts->format(newsys_conf("time_format"));
		$author_uri	= "$path/profile.php?user_id="			. $story["author_id"];
		$edit_uri	= "$path/admin/story-edit.php?story_id="	. $story["story_id"];
		$delete_uri	= "$path/admin/story-delete.php?story_id="	. $story["story_id"];
		$author		= $dbh->query("	SELECT
							username
						FROM
							newsys_users
						WHERE
							user_id = '{$story["author_id"]}'",DB_COL);
		$comment_uri	= "#";

		if (newsys_conf("allow_comments"))
			$comment_uri	= "$path/admin/com-add.php?story_id="		. $story["story_id"] . "&amp;parent_comment_id=0";

		$headline	= $story["headline"];
		$overview	= $story["overview"];
		$story		= $story["story"];

		return TRUE;
	}

	return FALSE;
?>
