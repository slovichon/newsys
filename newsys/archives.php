<?php
	require_once "newsys/main.inc";

	$newsys_offset		= (int)@$_GET["offset"];
	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	$newsys_num_stories	= 0;
	$newsys_result_limit	= newsys_conf("result_limit");
	$newsys_total		= $newsys_dbh->query("	SELECT
								COUNT(*)
							FROM
								newsys_stories",DB_COL);

	if ($newsys_offset >= $newsys_total)
		$newsys_offset = 0;

	$newsys_display_from	= $newsys_offset + 1;
	$newsys_display_to	= $newsys_offset + $newsys_result_limit;

	if ($newsys_display_to > $newsys_total)
		$newsys_display_to = $newsys_total;
	
	if ($newsys_display_from > $newsys_total)
		$newsys_display_from = $newsys_total;

	echo	newsys_get_template("header"),
		$newsys_of->header("Archives"),
		$newsys_of->table_start(array('class' => "newsysTable")),
			$newsys_of->table_row(array('colspan' => 2,'class' => "newsysHeader",'value' => "Displaying Stories $newsys_display_from - $newsys_display_to"));

	$newsys_dbh->query("	SELECT
					headline,
					story_id,
					create_date
				FROM
					newsys_stories
				ORDER BY
					create_date	DESC
				LIMIT
					$newsys_offset,
					$newsys_result_limit",DB_ROWS);

	$newsys_path = newsys_build_path();

	while ($newsys_story = $newsys_dbh->fetch_row())
	{
		$newsys_col_class	= newsys_gen_class();
		$newsys_ts		= new TimeStamp($newsys_story["create_date"]);

		echo	$newsys_of->table_row
			(
				array("class" => $newsys_col_class,"value" => $newsys_of->link($newsys_story["headline"],$newsys_path . "/view.php?story_id=" . $newsys_story["story_id"])),
				array("class" => $newsys_col_class,"value" => $newsys_ts->format(newsys_conf("time_format")))
			);
	}

	if (!$newsys_num_stories)
		echo 	$newsys_of->table_row
			(
				array('class' => 'newsysData1','value' => "No archives are available at this time.")
			);

	echo		$newsys_of->table_row
			(
				array
				(
					"colspan"	=> 3,
					"class"		=> "newsysFooter",
					"value"		=>	newsys_of_nav_menu
								(
									$newsys_of,
									array
									(
										"offset"	=> $newsys_offset,
										"total"		=> $newsys_total,
										"url"		=> $_SERVER["PHP_SELF"] . "?offset="
									)
								)
				)
			),
		$newsys_of->table_end(),
		newsys_get_template("footer");
?>
