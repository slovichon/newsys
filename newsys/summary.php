<?php
	require_once "newsys/main.inc";

	$newsys_of = newsys_get_of();

	echo	newsys_get_template("header"),
		$newsys_of->header("Latest Stories");

	$newsys_num_stories	= 0;
	$newsys_ids		= newsys_get_latest_stories();
	$newsys_result_limit	= newsys_conf("result_limit");

	if (is_array($newsys_ids))
	{
		foreach ($newsys_ids as $newsys_id)
		{
			$newsys_id = chop($newsys_id);

			if (!$newsys_id)
				continue;

			$newsys_story = newsys_get_template("preview",$newsys_id);

			if ($newsys_story)
				echo $newsys_story;

			if ($newsys_num_stories++ > $newsys_result_limit)
				break;
		}
	}

	if (!$newsys_num_stories)
		echo $newsys_of->p("There are currently no news stories available.");

	echo	newsys_get_template("footer");
?>
