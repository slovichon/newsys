<?php
	define("NEWSYS_SKIP_CHECK",TRUE,TRUE);
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)			= newsys_log_in($newsys_dbh,NEWSYS_COMMENTER);
	$newsys_username	= $newsys_dbh->query("	SELECT
							username
						FROM
							newsys_users
						WHERE
							user_id = $newsys_user_id",DB_COL);

	$newsys_levels	= newsys_conf("levels");

	echo	newsys_get_template("header"),
		$newsys_of->header("Newsys Administration"),
		$newsys_of->p("Welcome to the administration, ",$newsys_of->strong($newsys_username),". "),
		$newsys_of->p
		(
			array('class' => "newsysInfo"),
			"You are logged in with ",
			$newsys_of->strong($newsys_levels[$newsys_user_type]),
			" privileges."
		),
		newsys_of_actions($newsys_of,$newsys_user_type),
		newsys_get_template("footer");
?>
