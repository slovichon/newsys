<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 03:50:08 PM
	 *
	 * session.inc - session routines
	 */
	define("NEWSYS_AUTH_HTTP",	1,TRUE);
	define("NEWSYS_AUTH_SESSION",	2,TRUE);
	define("NEWSYS_AUTH_COOKIE",	3,TRUE);

	/*
	 * newsys_session_exists() -
	 * checks if a user has been authenticated
 	 */
	function newsys_session_exists($minlevel)
	{
		$dbh = loginsys_get_dbh();
		
		# Get session identifier
		
		
		if (loginsys_session_exists($dbh,$session_id))
		{
		} else {
		}
	}

	function newsys_session_remove($session_id)
	{
		$session_id	= (int)$session_id;
		$dbh		= loginsys_get_dbh();
		
		loginsys_session_remove($dbh,$session_id);
	}

	function newsys_session_add($username,$password)
	{
		$user_id	= newsys_user_get_id_by_name($dbh,$username);
		$dbh		= loginsys_get_dbh();

		return loginsys_session_add($dbh,$user_id,$password);
	}













	function newsys_is_logged_in($dbh)
	{
		$user_id = 0;

		switch (newsys_conf("auth_type"))
		{
			case NEWSYS_AUTH_HTTP:
			{
				$username	= @$_SERVER["PHP_AUTH_USER"];
				$password	= @$_SERVER["PHP_AUTH_PW"];

				$password	= newsys_crypt($password);

				$username	= $dbh->prepare_str($username,SQL_REG);
				$password	= $dbh->prepare_str($password,SQL_REG);

				$user_id	= $dbh->query("	SELECT
									user_id
								FROM
									newsys_users
								WHERE
									username = '$username' AND
									password = '$password'",DB_COL);
				break;
			}

			case NEWSYS_AUTH_COOKIE:
			{
				# Checking posted form data
				if (@$_POST["newsys_submitted"])
				{
					$username		= @$_POST["newsys_user"];
					$password		= @$_POST["newsys_pw"];

					$password		= newsys_crypt($password);
/*
 * NOTE: saving the user's encrypted password in their cookie
 * is NOT a good idea. This entire type of auth is highly
 * frowned upon. Use another type instead.
 */
					setCookie("newsys_user",$username);
					setCookie("newsys_pw",$password);

					$_COOKIE["newsys_user"]	= $username;
					$_COOKIE["newsys_pw"]	= $password;
				}

				$username	= $dbh->prepare_str(@$_COOKIE["newsys_user"],	SQL_REG);
				$password	= $dbh->prepare_str(@$_COOKIE["newsys_pw"],	SQL_REG);
				$user_id	= $dbh->query("	SELECT
									user_id
								FROM
									newsys_users
								WHERE
									username = '$username' AND
									password = '$password'",DB_COL);

				break;
			}

			case NEWSYS_AUTH_SESSION:
			{
				newsys_session_start();

				if (@$_POST["newsys_submitted"])
				{
					$username			= @$_POST["newsys_user"];
					$password			= @$_POST["newsys_pw"];

					$password			= newsys_crypt($password);

					$_SESSION["newsys_user"]	= $username;
					$_SESSION["newsys_pw"]		= $password;
				}

				$username	= $dbh->prepare_str(@$_SESSION["newsys_user"],SQL_REG);
				$password	= $dbh->prepare_str(@$_SESSION["newsys_pw"],SQL_REG);
				$user_id	= $dbh->query("	SELECT
									user_id
								FROM
									newsys_users
								WHERE
									username = '$username' AND
									password = '$password'",DB_COL);

				break;
			}

			default:
			{
				newsys_handle_error("Unknown auth type; type: " . newsys_conf("auth_type"));
			}
		}

		return (int)$user_id;
	}

	function newsys_log_in($dbh,$min_level)
	{
		$user_id	= newsys_is_logged_in($dbh);
		$user_type	= $dbh->query("	SELECT
							type
						FROM
							newsys_users
						WHERE
							user_id = $user_id",DB_COL);

		if ($user_id && $user_type >= $min_level)
		{
			return array($user_id,$user_type);
		} else {
			$of = newsys_get_of();

			switch (newsys_conf("auth_type"))
			{
				case NEWSYS_AUTH_HTTP:
				{
					# Should probably be updated to HTTP/1.1
					header("WWW-Authenticate: Basic realm=\"Newsys Administration\"");
					header("HTTP/1.0 401 Unauthorized");

					echo	newsys_get_template("header"),
						$of->header("Error"),
						$of->p("The username and password you supplied is invalid, or your level of authority is not high enough to access the requested resource.");
						newsys_get_template("footer");

					break;
				}

				case NEWSYS_AUTH_COOKIE:
				{
					if (@$_COOKIE["newsys_user"])
					{
						/* User tried to log in */
						echo	newsys_get_template("header"),
							$of->header("Error"),
							$of->p("The username and password you supplied is invalid, or your level of authority is not high enough to access the requested resource."),
							newsys_of_login($of),
							newsys_get_template("footer");

					} else {
						/* User did not try to log in */
						echo	newsys_get_template("header"),
							$of->header("Log In"),
							newsys_of_login($of),
							newsys_get_template("footer");
					}

					break;
				}

				case NEWSYS_AUTH_SESSION:
				{
					if (@$_SESSION["newsys_user"])
					{
						/* User tried to log in */
						echo	newsys_get_template("header"),
							$of->header("Error"),
							$of->p("The username and password you supplied is invalid, or your level of authority is not high enough to access the requested resource."),
							newsys_of_login($of),
							newsys_get_template("footer");
					} else {
						/* User did not try to log in */
						echo	newsys_get_template("header"),
							$of->header("Log In"),
							newsys_of_login($of),
							newsys_get_template("footer");
					}

					break;
				}

				default:
				{
					newsys_handle_error("Unknown auth type; type: " . newsys_conf("auth_type"));
				}
			}

			exit();
		}
	}

	function newsys_log_out()
	{
		switch (newsys_conf("auth_type"))
		{
			case NEWSYS_AUTH_HTTP:
			{
				header("WWW-Authenticate: Basic realm=\"Newsys Administration\",stale=\"TRUE\"");
				header("HTTP/1.0 401 Unauthorized");

				$_SERVER["PHP_AUTH_USER"]	= "";
				$_SERVER["PHP_AUTH_PW"]		= "";

				break;
			}

			case NEWSYS_AUTH_COOKIE:
			{
				$_COOKIE["newsys_user"]		= "";
				$_COOKIE["newsys_pw"]		= "";

				setCookie("newsys_user");
				setCookie("newsys_pw");

				break;
			}

			case NEWSYS_AUTH_SESSION:
			{
				newsys_session_start();

				$_SESSION["newsys_user"]	= "";
				$_SESSION["newsys_pw"]		= "";

				break;
			}

			default:
			{
				newsys_handle_error("Unknown auth type; type: " . newsys_conf("auth_type"));
			}
		}

		return;
	}
?>
