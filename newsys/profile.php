<?php
	require_once "newsys/main.inc";

	$newsys_user_id	= (int)@$_GET["user_id"];
	$newsys_html	= newsys_get_template("profile",$newsys_user_id);
	$newsys_of	= newsys_get_of();

	if ($newsys_html)
	{
		echo	newsys_get_template("header"),
			$newsys_of->header("User Profile"),
			$newsys_html,
			newsys_get_template("footer");
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Error"),
			$newsys_of->p("The requested user could not be found."),
			newsys_get_template("footer");
	}
?>
