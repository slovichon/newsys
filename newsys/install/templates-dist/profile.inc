<?php
	$dbh		= newsys_get_dbh();
	$path		= newsys_build_path();
	$user_id	= $args[1];
	$user		= newsys_user_get($dbh,$user_id);

	if (is_array($user))
	{
		$newsys_levels		= newsys_conf("levels");
		$email			= preg_replace("/@/","&lt;at&gt;",$user["email"]);
		$type			= $newsys_levels[$user["type"]];
		$num_stories		= $dbh->query("	SELECT
								COUNT(*)
							FROM
								newsys_stories
							WHERE
								author_id = {$user["user_id"]}",DB_COL);

		$last_story		= $dbh->query("	SELECT
								story_id,
								create_date,
								headline
							FROM
								newsys_stories
							WHERE
								author_id = {$user["user_id"]}
							ORDER BY
								create_date DESC
							LIMIT
								1",DB_ROW);


		if (is_array($last_story))
		{
			$ts			= new TimeStamp($last_story["create_date"]);
			$last_story_uri		= "$path/view.php?story_id=" . $last_story["story_id"];
			$last_story_date	= $ts->format(newsys_conf("time_format"));
			$last_story		= "<a href=\"{$last_story_uri}\">{$last_story["headline"]}</a> " .
							"posted on $last_story_date";
		} else {
			$last_story		= "Never";
		}

		$username		= $user["username"];

		return TRUE;
	}

	return FALSE;
?>
