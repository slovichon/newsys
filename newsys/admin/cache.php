<?php
	define("NEWSYS_SKIP_CHECK",TRUE,TRUE);
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)		= newsys_log_in($newsys_dbh,NEWSYS_ADMIN);

	if (newsys_conf("use_cache"))
	{
		if (@$_POST["newsys_t_submitted"])
		{
			$newsys_dir = @opendir(newsys_conf("cache_dir"));

			if (!$newsys_dir)
				newsys_handle_error("Could not get directory handle; directory: " . newsys_conf("cache_dir"));

			while ($newsys_file = readdir($newsys_dir))
				if (preg_match("^cache-",$newsys_file))
					newsys_cache_delete($newsys_file);

			closedir($newsys_dir);

			newsys_rebuild_latest_stories();

			echo	newsys_get_template("header"),
				$newsys_of->header("Cache Cleared"),
				$newsys_of->p("The cache has been cleared."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Clearing Cache"),
				$newsys_of->p("Are you sure you want to clear the cache?"),
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
			$newsys_of->p("To use this functionality please enable caching."),
			newsys_of_actions($newsys_of,$newsys_user_type),
			newsys_get_template("footer");
	}
?>
