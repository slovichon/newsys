<?php
	require_once "newsys/main.inc";

	$newsys_of = newsys_get_of();

	if (@$_GET["recover_key"] && @$_GET["user_id"])
	{
		$newsys_dbh		= newsys_get_dbh();
		$newsys_recover_key	= $newsys_dbh->prepare_str($_GET["recover_key"],SQL_REG);
		$newsys_user_id		= (int)$_GET["user_id"];

		if
		(
			$newsys_dbh->query("	SELECT
							user_id
						FROM
							newsys_users
						WHERE
							user_id		= $newsys_user_id
						AND	recover_key	= '$newsys_recover_key'",DB_COL)
		)
		{
			/* Key is authentic, set new password */
			$newsys_dbh->query("	UPDATE
							newsys_users
						SET
							password	= new_password,
							new_password	= NULL,
							recover_key	= NULL
						WHERE
							user_id		= $newsys_user_id",DB_NULL);

			echo	newsys_get_template("header"),
				$newsys_of->header("Password Recovered"),
				$newsys_of->p("Your password has been successfully recovered. You may want to ",$newsys_of->link("log in",newsys_build_path() . "/login.php")," and change it to something you may more easily remember."),
				newsys_get_template("footer");
		} else {
			/* Invalid key */
/*
 * Should we clear the new, generated pass here? Or perhaps
 * increment a counter which will do so after a specified
 * number of times?
 */
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("Please load the URL that was given to you in the sent e-mail message to recover your password."),
				newsys_get_template("footer");
		}
	} else if (@$_POST["ns_submitted"]) {

		$newsys_dbh		= newsys_get_dbh();
		$newsys_username	= $newsys_dbh->prepare_str(@$_POST["username"],SQL_REG);
		list
		(
			$newsys_user_id,
			$newsys_email,

		)			= $newsys_dbh->query("	SELECT
									user_id,
									email
								FROM
									newsys_users
								WHERE
									username = '$newsys_username'",DB_ROW);

		if ($newsys_user_id && $newsys_email)
		{
			# Generate a new password
			$newsys_new_password	= newsys_rand_str(12);
			$newsys_recover_key	= newsys_rand_str(20);

			# We can't have any "'/ characters
			while (preg_match("/[\\'\"]/",$newsys_new_password))
				$newsys_new_password = preg_replace("/['\"\\]/e","newsys_rand_str(1)",$newsys_new_password);

			$newsys_db_new_password	= newsys_crypt($newsys_new_password);

			$newsys_db_new_password	= $newsys_dbh->prepare_str($newsys_db_new_password,	SQL_REG);
			$newsys_db_recover_key	= $newsys_dbh->prepare_str($newsys_recover_key,		SQL_REG);

			$newsys_dbh->query("	UPDATE
							newsys_users
						SET
							new_password	= '$newsys_db_new_password',
							recover_key	= '$newsys_db_recover_key'
						WHERE
							user_id		= '$newsys_user_id'",DB_NULL);

			$newsys_site_name	= newsys_conf("site_name");
			$newsys_site_uri	= newsys_conf("site_uri");

			newsys_mail
			(
				$newsys_email,
				newsys_conf("site_email"),
				"$newsys_site_name Password Recovery",
"Your password information has been requested. We cannot recover your
password so we have instead generated you a new one. You can, however
continue to use your old password to log into your existing newsys
account on $newsys_site_name. In fact, this new password will only
take effect if you load up the following URL:

	" . newsys_build_path(NEWSYS_PATH_ABS) . "/lost-pass.php?user_id=$newsys_user_id&recover_key=" . urlEncode($newsys_recover_key) . "

Once you load up this page, your new password will take effect and
you will no longer be able to use your old one. Your new password is:

	New password: $newsys_new_password

This message was automatically generated and you should not reply to
it.

$newsys_site_name
$newsys_site_uri");

			echo	newsys_get_template("header"),
				$newsys_of->header("Password Recovered"),
				$newsys_of->p("Your new password has been generated and sent to your registration e-mail address."),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("Your password could not be recovered."),
				newsys_get_template("footer");
		}
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Recover Password"),
			$newsys_of->p("If you have lost your password, fill out the following form and a new password will be mailed to the e-mail address with which you signed up."),
			$newsys_of->form
			(
				array(),
				$newsys_of->table
				(
					array('class' => "newsysTable"),
					$newsys_of->table_row(array('colspan' => 2,'class' => 'newsysHeader','value' => "Recover Password")),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Username:"),array('class' => "newsysData1",'value' => $newsys_of->input(array('type' => 'text','name' => "username")))),
					$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Recover",'name' => "ns_submitted")) .
															$newsys_of->input(array('type' => "reset", 'class' => "newsysButton",'value' => "Clear"))))
				)
			),
			newsys_get_template("footer");
	}
?>
