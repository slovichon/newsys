<?php
	$dbh		= newsys_get_dbh();
	$path		= newsys_build_path();
	$story_id	= $args[1];
	$comment_id	= $args[2];
	$comment	= newsys_comment_get($dbh,$story_id,$comment_id);

	if (is_array($comment))
	{
		$ts = new TimeStamp($comment["create_date"]);

		$author			= $dbh->query("	SELECT
								username
							FROM
								newsys_users
							WHERE
								user_id = '{$comment["author_id"]}'",DB_COL);
		$date			= $ts->format(newsys_conf("time_format"));
		$author_uri		= "$path/profile.php?user_id="			. $comment["author_id"];
		$edit_uri		= "$path/admin/com-edit.php?story_id="		. $comment["story_id"] . "&amp;comment_id=" . $comment["comment_id"];
		$delete_uri		= "$path/admin/com-delete.php?story_id="	. $comment["story_id"] . "&amp;comment_id=" . $comment["comment_id"];
		$comment_uri		= "#";

		if (newsys_conf("allow_comments"))
		{
			if (newsys_conf("hier_comments"))
				$comment_uri = "$path/admin/com-add.php?story_id="	. $comment["story_id"] . "&amp;parent_comment_id=" . $comment["comment_id"];
			else
				$comment_uri = "$path/admin/com-add.php?story_id="	. $comment["story_id"] . "&amp;parent_comment_id=0";
		}

		$subject		= $comment["subject"];
		$comment		= $comment["comment"];

		return TRUE;
	}

	return FALSE;
?>
