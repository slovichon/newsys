<?php
	require_once "newsys/main.inc";

	$newsys_of = newsys_get_of();

	if (@$_POST["ns_submitted"])
	{
		$newsys_dbh		= newsys_get_dbh();
		$newsys_email		= @$_POST["ns_email"];
		$newsys_db_email	= $newsys_dbh->prepare_str($newsys_email,SQL_REG);
		$newsys_username	= $newsys_dbh->query("	SELECT
								username
							FROM
								newsys_users
							WHERE
								email = '$newsys_db_email'",DB_COL);
		$newsys_site_name	= newsys_conf("site_name");
		$newsys_site_uri	= newsys_conf("site_uri");

		if ($newsys_username)
		{
			newsys_mail
			(
				$newsys_email,
				newsys_conf("site_email"),
				"$newsys_site_name Newsys Recovery",
"Your username was requested. Here is the information you inquired:

	Username: $newsys_username

This message was automated and you should not reply to it.

$newsys_site_name
$newsys_site_uri");

			echo	newsys_get_template("header"),
				$newsys_of->header("Username Recovered"),
				$newsys_of->p("Your username has been e-mailed to the address with which you registered."),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("Your username could not be found."),
				newsys_get_template("footer");
		}
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Recovering Username"),
			$newsys_of->p("If you have lost your username, you can fill the form out below to receieve an e-mail to the e-mail address with which you signed up with containing it."),
			$newsys_of->form
			(
				array(),
				$newsys_of->table
				(
					array('class' => "newsysTable"),
					$newsys_of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => "Recover Username")),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "E-mail Address:"),array('class' => "newsysData1",'value' => $newsys_of->input(array('type' => "text",'name' => "ns_email")))),
					$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Recover",'name' => "ns_submitted")) .
															$newsys_of->input(array('type' => "reset", 'class' => "newsysButton",'value' => "Clear"))))
				)
			),
			newsys_get_template("footer");
	}
?>
