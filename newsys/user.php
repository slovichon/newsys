<?php
	require_once "newsys/main.inc";

	$newsys_format_name	= "";
	$newsys_format_name_p	= "";
	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	$newsys_count		= 0;
	$newsys_what		= "";

	switch (@$_GET["what"])
	{
		case "overview":
		{
			$newsys_format_name	= "Overview";
			$newsys_format_name_p	= "Overviews";
			$newsys_what		= "overview";
			break;
		}

		case "headline":
		default:
		{
			$newsys_format_name	= "Headline";
			$newsys_format_name_p	= "Headlines";
			$newsys_what		= "headline";
		}
	}

	$newsys_username	= $newsys_dbh->prepare_str(@$_GET["username"],SQL_REG);
	$newsys_user_id		= $newsys_dbh->query("	SELECT
								user_id
							FROM
								newsys_users
							WHERE
								username = '$newsys_username'",DB_COL);

	if ($newsys_user_id)
	{
		$newsys_dbh->query("	SELECT
						story_id,
						$newsys_what,
						create_date
					FROM
						newsys_stories
					WHERE
						author_id	= $newsys_user_id
					ORDER BY
						create_date	DESC
					LIMIT
						" . newsys_conf("result_limit"),DB_ROWS);

		echo	newsys_get_template("header"),
			$newsys_of->header("Search Results"),
			$newsys_of->table_start(array('class' => "newsysTable")),
				$newsys_of->table_row(array('class' => "newsysHeader",'value' => $newsys_format_name),array('class' => "newsysHeader",'value' => "Date"));

		$newsys_path = newsys_build_path();

		while ($newsys_story = $newsys_dbh->fetch_row())
		{
			$newsys_count++;
			$newsys_ts		= new TimeStamp($newsys_story["create_date"]);
			$newsys_col_class	= newsys_gen_class();

			echo	$newsys_of->table_row
				(
					array('class' => $newsys_col_class,'value' => $newsys_of->link($newsys_story[$newsys_what],$newsys_path . "/view.php?story_id=" . $newsys_story["story_id"])),
					array('class' => $newsys_col_class,'value' => $newsys_ts->format(newsys_conf("time_format")))
				);
		}

		if (!$newsys_count)
			echo	$newsys_of->table_row
				(
					array('class' => "newsysData1",'colspan' => 2,'value' => "No stories are available from this user.")
				);

		echo	$newsys_of->table_end(),
			newsys_get_template("footer");
	} else {
		# User search
		echo	newsys_get_template("header"),
			$newsys_of->header("User Search"),
			$newsys_of->form
			(
				array('method' => "get"),
				$newsys_of->table
				(
					array('class' => "newsysTable"),
					$newsys_of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => "User Search")),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Username:"),	array('class' => "newsysData1",'value' => $newsys_of->input(array('type' => "text",		'name' => "username")))),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Show:"),	array('class' => "newsysData2",'value' => $newsys_of->input(array('type' => 'select',	'name' => "what", 'options' => array('headline' => "Headlines",'overview' => "Overviews"))))),
					$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Go")) .
															$newsys_of->input(array('type' => "reset", 'class' => "newsysButton",'value' => "Reset"))))
				)
			),
			newsys_get_template("footer");
	}
?>
