<?php
	function newsys_t_hier_comments($story_id,$comment_id)
	{
		$child_ids	= @file(newsys_conf("cache_dir") . "/info-comment-$story_id-$comment_id");
		$output		= newsys_get_template("comment",$story_id,$comment_id);

		if (is_array($child_ids))
		{
/*
	Should the output of newsys_get_template() be cached?
*/
			$output .= newsys_get_template('start_comment');

			foreach ($child_ids as $child_id)
			{
				$child_id = chop($child_id);

				if (!$child_id)
					continue;

				$output .= newsys_t_hier_comments($story_id,$child_id);
			}

			$output .= newsys_get_template('end_comment');
		}

		return $output;
	}

	require_once "newsys/main.inc";

	$newsys_of		= newsys_get_of();
	$newsys_story_id	= (int)@$_GET["story_id"];

	if ($newsys_html = newsys_get_template("story",$newsys_story_id))
	{
		echo	newsys_get_template("header"),
			$newsys_of->header("Viewing Story"),
			$newsys_html;

		# Gather list of comment IDs
		$newsys_comment_ids = @file(newsys_conf("cache_dir") . "/info-story-$newsys_story_id");

		/* Display comment and its comments */
		if (is_array($newsys_comment_ids))
		{
			foreach ($newsys_comment_ids as $newsys_comment_id)
			{
				$newsys_comment_id = chop($newsys_comment_id);

				if ($newsys_comment_id)
					echo newsys_t_hier_comments($newsys_story_id,$newsys_comment_id);
			}
		}

		echo	newsys_get_template("footer");
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Error"),
			$newsys_of->p("The requested news story could not be found."),
			newsys_get_template("footer");
	}
?>
