<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 02:39:43 PM
	 *
	 * main.inc - This guy includes all core Newsys
	 * components so you don't have to do them
	 * individually.
	 */

	# Defines
	define("NEWSYS_VERSION",	"-current",	TRUE);
	define("NEWSYS_BUF_SIZE",	1024,		TRUE);
	define("E_NEWSYS_NONE",		0,		TRUE);	# No error

	/*
	 * newsys-config.inc will call this function, so it must be
	 * defined before the config file is included, and it must
	 * be defined here because we can't include an library
	 * file because we don't know where they're at yet.
	 */
	function newsys_conf()
	{
		static $vars = array();
		static $dbh;

		$args = func_get_args();
		$argc = func_num_args();

		if ($argc == 2)
		{
			# Setting a configuration directive
			$vars[$args[0]] = $args[1];

		} elseif ($argc != 1) {

			if (function_exists('newsys_handle_error'))
				newsys_handle_error("newsys_conf(): incorrect arguments given");
			else
				die('newsys_conf(): incorrect arguments given');
		} else {
			# Requesting a configuration directive value
			if ($dbh === NULL)
			{
				# Prevents recursion
				$dbh = FALSE;
				$dbh = newsys_get_dbh();
			}

			# Check if user has a preference
			if (newsys_session_exists() &&
				newsys_isr_has_field("newsys_users","pref_".$args[0]) &&
				is_resource($dbh))
			{
				$key = "pref_" . $dbh->prepare_str($args[0]);
				$dbh->query("	SELECT
							$field
						FROM
							newsys_users
						WHERE
							user_id = $user_id");
				if ($dbh->num_rows())
				{
					list ($val) = $dbh->fetch_row();
					return $val;
				}
			}

			# Lookup default preference value
			if (!array_key_exists($args[0],$vars))
				if (function_exists('newsys_handle_error'))
					newsys_handle_error("newsys_conf(): configuration value requested before supplied; key: $args[0]");
				else
					die('newsys_conf(): configuration value requested before supplied; key: $args[0]');
	
			return $vars[$args[0]];
		}

		return;
	}

	/*
	 * These files must reside in a directory in
	 * PHP's include_path
	 */
	require_once "CPL/1.0.inc";
	require_once "newsys-config.inc";
	require_once "TimeStamp/1.5.inc";

	$newsys_sys_root = newsys_conf('sys_root');

	require_once "$newsys_sys_root/cache.inc";
	require_once "$newsys_sys_root/comment.inc";
	require_once "$newsys_sys_root/crypt.inc";
	require_once "$newsys_sys_root/extra.inc";
	require_once "$newsys_sys_root/misc.inc";
	require_once "$newsys_sys_root/of.inc";
	require_once "$newsys_sys_root/session.inc";
	require_once "$newsys_sys_root/story.inc";
	require_once "$newsys_sys_root/user.inc";
	require_once "$newsys_sys_root/userfields.inc";
	require_once "$newsys_sys_root/xml.inc";

	# Fix PHP from tampering with our data
	if (ini_get('magic_quotes_gpc'))
	{
		array_strip_slashes($_POST);
		array_strip_slashes($_GET);
		array_strip_slashes($_REQUEST);
		array_strip_slashes($_COOKIE);
		array_strip_slashes($_SESSION);
	}

	# We don't want PHP obscuring our data,
	# we just have to be careful
	ini_set('magic_quotes_runtime',FALSE);

/*
 * Should we add code to check crypt_key for changes
 * so that we can update the database and cryptographic
 * communications?
 *
 * In this case, saving the old, cached key would not
 * be very wise, as it could easily be examined should
 * its contents be stored in a file. We should also
 * probably save a version in the database as well, in
 * the case that the admin has use_cache off.
 */
	if (!defined("NEWSYS_SKIP_INIT"))
		newsys_fields_update();

/*
	if (file_exists(newsys_build_path(NEWSYS_PATH_SYS) . "/install"))
		newsys_handle_error("The `install' directory has not been deleted");
*/
?>
