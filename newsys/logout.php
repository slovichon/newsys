<?php
#	define("NEWSYS_SKIP_CHECK",TRUE,TRUE); # maybe crypt_key will have changed?
	require_once "newsys/main.inc";

	$newsys_dbh	= newsys_get_dbh();
	$newsys_of	= newsys_get_of();

	if (newsys_is_logged_in($newsys_dbh))
	{
		newsys_log_out();

		echo	newsys_get_template("header"),
			$newsys_of->header("Logged Out"),
			$newsys_of->p("You have successfully logged out."),
			$newsys_of->p($newsys_of->link("Log Back In",newsys_build_path() . "/login.php")),
			newsys_get_template("footer");
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Error"),
			$newsys_of->p("You must log in before you can log out."),
			newsys_of_login($newsys_of),
			newsys_get_template("footer");
	}
?>
