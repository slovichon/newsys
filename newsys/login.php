<?php
#	define("NEWSYS_SKIP_CHECK",TRUE,TRUE); # maybe crypt_key will have changed?
	require_once "newsys/main.inc";

	$newsys_of = newsys_get_of();

	if (@$_POST["newsys_submitted"])
	{
		$newsys_dbh		= newsys_get_dbh();

		# Allow all users to log in
		list
		(
			$newsys_user_id,
			$newsys_user_type

		)		= newsys_log_in($newsys_dbh,NEWSYS_COMMENTER);

		# Gather filename disregarding potentially useless
		# (and dangerous) appendages
		$newsys_redirect	= newsys_conf("sys_root") . preg_replace("/[?#].*$/","",@$_POST["newsys_redir"]);

		if (@$_POST["newsys_redir"] && file_exists($newsys_redirect))
			newsys_redirect($_POST["newsys_redir"]);

		newsys_redirect(newsys_build_path(NEWSYS_PATH_ABS) . "/admin/index.php");
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Log In"),
			newsys_of_login($newsys_of),
			newsys_get_template("footer");
	}
?>
