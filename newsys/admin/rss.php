<?php
	define("NEWSYS_SKIP_CHECK",TRUE,TRUE);
	require_once "newsys/main.inc";

	$newsys_dbh	= newsys_get_dbh();
	$newsys_of	= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)		= newsys_log_in($newsys_dbh,NEWSYS_ADMIN);

	if (newsys_conf("use_rss"))
	{
		if (@$_POST["newsys_t_submitted"])
		{
			/* Non-empty files will be re-created and re-setup */
			foreach (newsys_conf("xml_files") as $newsys_file)
				newsys_unlink($newsys_file);

			echo	newsys_get_template("header"),
				$newsys_of->header("RSS Cleared"),
				$newsys_of->p("The RSS data has been cleared."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Clearing RSS"),
				$newsys_of->p("Are you sure you want to clear the RSS data?"),
				$newsys_of->form
				(
					array(),
					$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Clear",'name' => "newsys_t_submitted",'onclick' => newsys_js_dbl_submit()))
				),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		}
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Error"),
			$newsys_of->p("To use this functionality please enable RSS."),
			newsys_get_template("footer");
	}
?>
