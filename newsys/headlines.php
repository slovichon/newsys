<?php
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	$newsys_format_name	= "";
	$newsys_format_name_p	= "";
	$newsys_action		= "";
	$newsys_num_stories	= 0;

	switch (@$_GET["action"])
	{
		case "story":
		{
			$newsys_format_name	= "Story";
			$newsys_format_name_p	= "Stories";
			$newsys_action		= "story";
			break;
		}

		case "headline":
		default:
		{
			$newsys_format_name	= "Headline";
			$newsys_format_name_p	= "Headlines";
			$newsys_action		= "headline";
		}
	}

	$newsys_dbh->query("	SELECT DISTINCT
					newsys_stories.story_id,
					newsys_stories.$newsys_action,
					newsys_stories.create_date,
					newsys_users.username
				FROM
					newsys_stories,
					newsys_users
				WHERE
					newsys_stories.author_id = newsys_users.user_id
				ORDER BY
					newsys_stories.create_date DESC
				LIMIT
					" . newsys_conf("result_limit"),DB_ROWS);

	$newsys_path = newsys_build_path();

	echo	newsys_get_template("header"),
		$newsys_of->header("Headlines"),
		$newsys_of->table_start(array('class' => "newsysTable")),
			$newsys_of->table_row(array('class' => 'newsysHeader','value' => "User"),array('class' => "newsysHeader",'value' => $newsys_format_name),array('class' => 'newsysHeader','value' => "Date"));

	while ($newsys_story = $newsys_dbh->fetch_row())
	{
		$newsys_num_stories++;
		$newsys_ts		= new TimeStamp($newsys_story["create_date"]);
		$newsys_col_class	= newsys_gen_class();

		echo		$newsys_of->table_row
				(
					array("class" => $newsys_col_class,'value' => $newsys_of->link($newsys_story["username"],$newsys_path . "/user.php?what=$newsys_action&amp;username=" . urlEncode($newsys_story["username"]))),
					array("class" => $newsys_col_class,'value' => $newsys_of->link($newsys_story[$newsys_action],$newsys_path . "/view.php?story_id=" . $newsys_story["story_id"])),
					array("class" => $newsys_col_class,'value' => $newsys_ts->format(newsys_conf("time_format")))
				);
	}

	if (!$newsys_num_stories)
		echo		$newsys_of->table_row(array("colspan" => 3,"class" => "newsysData1","value" => "There currently are no headlines available."));

	echo	$newsys_of->table_end(),
		newsys_get_template("footer");
?>
