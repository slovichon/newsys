<?php
	define("NEWSYS_SKIP_CHECK",TRUE,TRUE);
	require_once "newsys/main.inc";

	$newsys_dbh	= newsys_get_dbh();
	$newsys_of	= newsys_get_of();

	if (@$_GET["cancel_key"] && @$_GET["user_id"])
	{
		$newsys_user_id		= (int)$_GET["user_id"];
		$newsys_cancel_key	= $newsys_dbh->prepare_str($_GET["cancel_key"],SQL_REG);

		if
		(
			$newsys_dbh->query("	SELECT
							user_id
						FROM
							newsys_users
						WHERE
							cancel_key	= '$newsys_cancel_key' AND
							user_id		= $newsys_user_id",DB_COL)
		)
		{
			/*
			 * We may want to clear out the cancel_key field,
			 * but since the account is supposed to be removed,
			 * we shouldn't have to worry about it
			 */
			if (newsys_user_remove($newsys_dbh,$newsys_user_id))
			{
				echo	newsys_get_template("header"),
					$newsys_of->header("Account Cancelled"),
					$newsys_of->p("Your account has been removed."),
					newsys_get_template("footer");
			} else {
				echo	newsys_get_template("header"),
					$newsys_of->header("Error"),
					$newsys_of->p("Your account could not be removed."),
					newsys_get_template("footer");
			}
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("Please load the URL from the e-mail message that was sent to your e-mail address to cancel your account."),
				newsys_get_template("footer");
		}
	} else {
		list
		(
			$newsys_user_id,
			$newsys_user_type

		) = newsys_log_in($newsys_dbh,NEWSYS_COMMENTER);

		if (@$_POST["newsys_t_submitted"])
		{
			$newsys_email		= $newsys_dbh->query("	SELECT
										email
									FROM
										newsys_users
									WHERE
										user_id = $newsys_user_id",DB_COL);

			$newsys_cancel_key	= newsys_rand_str(20);
			$newsys_db_cancel_key	= $newsys_dbh->prepare_str($newsys_cancel_key,SQL_REG);

			$newsys_dbh->query("	UPDATE
							newsys_users
						SET
							cancel_key	= '$newsys_db_cancel_key'
						WHERE
							user_id		= $newsys_user_id",DB_NULL);

			$newsys_path		= newsys_build_path(NEWSYS_PATH_ABS);
			$newsys_site_name	= newsys_conf("site_name");
			$newsys_site_uri	= newsys_conf("site_uri");

			newsys_mail
			(
				$newsys_email,
				newsys_conf("site_email"),
				"$newsys_site_name Newsys Account",
"Your account has been requested to be cancelled. To complete cancellation, point
your Web browser to the following URI:

	$newsys_path/admin/user-cancel.php?user_id=$newsys_user_id&cancel_key=" . urlEncode($newsys_cancel_key) . "

We would like to thank you for using this account.

$newsys_site_name
$newsys_site_uri"
			);

			echo	newsys_get_template("header"),
				$newsys_of->header("Cancelling Account"),
				$newsys_of->p("An e-mail message has been sent to your e-mail address. Consult that message for further instructions on removing your account."),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Cancelling Account"),
				$newsys_of->form
				(
					array(),
					$newsys_of->p("Are you sure you would like to cancel your account?"),
					$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'name' => "newsys_t_submitted",'value' => "Remove Account"))
				),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		}
	}
?>
