<?php
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)			= newsys_log_in($newsys_dbh,NEWSYS_REGULAR);
	$newsys_story_id	= (int)@$_REQUEST["story_id"];
	$newsys_author_id	= $newsys_dbh->query("	SELECT
							author_id
						FROM
							newsys_stories
						WHERE
							story_id = $newsys_story_id",DB_COL);

	# Authors can delete their own stories;
	# Admins can delete any story.
	if ($newsys_author_id == $newsys_user_id || $newsys_user_type > NEWSYS_REGULAR)
	{
		if (@$_POST["newsys_t_submitted"])
		{
			newsys_story_delete($newsys_dbh,$newsys_story_id);

			echo	newsys_get_template("header"),
				$newsys_of->header("Story Deleted"),
				$newsys_of->p("This story has been successfully deleted."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");

		} elseif ($newsys_story_id) {

			echo	newsys_get_template("header"),
				$newsys_of->header("Deleting Story"),
				$newsys_of->form
				(
					array(),
					$newsys_of->p("You are about to delete this story. Are you sure?"),
					newsys_get_template("story",$newsys_story_id),
					$newsys_of->input(array('type' => "hidden",'name' => "story_id",'value' => $newsys_story_id)),
					$newsys_of->input(array('type' => "submit",'name' => "newsys_t_submitted",'class' => "newsysButton",'value' => "Delete"))
				),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("The requested story could not be found."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		}
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Deleting Story"),
			$newsys_of->p("Your level of administration does not permit you to delete this story."),
			newsys_of_actions($newsys_of,$newsys_user_type),
			newsys_get_template("footer");
	}
?>
