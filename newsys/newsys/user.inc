<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 05:40:33 PM
	 *
	 * user.inc - user routines
	 */

	# User-level types
	define("NEWSYS_COMMENTER",	1,TRUE);
	define("NEWSYS_REGULAR",	2,TRUE);
	define("NEWSYS_ADMIN",		3,TRUE);

	# Users
	# Note: the user error codes are more strict because any user can sign up
	# without permission (potentially)
	define("E_NEWSYS_USER_NAME",		1,TRUE);	# Invalid username
	define("E_NEWSYS_USER_NAME_USE",	2,TRUE);	# Username already in use
	define("E_NEWSYS_USER_PASS",		4,TRUE);	# Invalid password
	define("E_NEWSYS_USER_EMAIL",		8,TRUE);	# Invalid e-mail

	function newsys_user_add($dbh,$user)
	{
		$err = E_NEWSYS_NONE;

		/* Validate incoming user information */
		if (!preg_match("/^[a-zA-Z0-9_-]{5,30}$/",$user["username"]))	$err |= E_NEWSYS_USER_NAME;
		if (!preg_match("/^[^'\"]{5,30}$/",$user["password"]))		$err |= E_NEWSYS_USER_PASS;
		if (!mail_is_valid($user["email"]))				$err |= E_NEWSYS_USER_EMAIL;

		$t = $dbh->prepare_str($user["username"],SQL_REG);

		if
		(
			$dbh->query("	SELECT
						username
					FROM
						newsys_users
					WHERE
						username = '$t'",DB_COL)
		)
										$err |= E_NEWSYS_USER_NAME_USE;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_USER,$user);

		if ($err != E_NEWSYS_NONE || count($user_errors))
			return array($err,$user_errors);

		$user["type"]		= (int)$user["type"];
		$user["email"]		= $dbh->prepare_str(newsys_str_parse($user["email"],NEWSYS_STR_NONE));
		$user["password"]	= $dbh->prepare_str(newsys_crypt($user["password"]));

		$sql_fields		= "";
		$sql_values		= "";

		foreach (newsys_conf("user_fields") as $user_field_id => $user_field)
		{
			$user[$user_field_id] = $dbh->prepare_str(
				newsys_str_parse($user[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql_fields .= ",ud_$user_field_id";
			$sql_values .= ",'{$user[$user_field_id]}'";
		}

		$dbh->query("	INSERT INTO
					newsys_users
				(
					username,
					password,
					email,
					type
					$sql_fields
				) VALUES (
					'{$user["username"]}',
					'{$user["password"]}',
					'{$user["email"]}',
					'{$user["type"]}'
					$sql_values
				)",DB_NULL);

		if (newsys_conf("use_cache"))
		{
			$user_id = $dbh->last_insert_id();

			/* Update cache */
			newsys_cache_expire("user",$user_id);
		}

		return array(E_NEWSYS_NONE,array());
	}

	function newsys_user_update($dbh,$user)
	{
		$err = E_NEWSYS_NONE;

/*
 * If no new password is specified, don't change it.
 * The password field is special because we don't want
 * to print the value out as the 'default' value while
 * a user updates his info
 */
#		if (!preg_match("/^[a-zA-Z0-9_-]{5,30}$/",$user["username"]))	$err |= E_NEWSYS_USER_NAME;
		if ($user["password"])
			if (!preg_match("/^[^'\"]{5,30}$/",$user["password"]))	$err |= E_NEWSYS_USER_PASS;
		if (!mail_is_valid($user["email"]))				$err |= E_NEWSYS_USER_EMAIL;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_USER,$user);

		if ($err != E_NEWSYS_NONE || count($user_errors))
			return array($err,$user_errors);

		$user["type"]	= (int)$user["type"];
		$user["email"]  = $dbh->prepare_str(newsys_str_parse($user["email"],NEWSYS_STR_NONE),SQL_REG);

		if ($user["password"])
			$user["password"] = $dbh->prepare_str(newsys_crypt($user["password"]),SQL_REG);

		$sql = "	UPDATE
					newsys_users
				SET
";

#					username		= '{$user["username"]}',

		/* Only update the password if it has been specified to change */
		if ($user["password"])
			$sql .= "
					password		= '{$user["password"]}',";

		foreach (newsys_conf("user_fields") as $user_field_id => $user_field)
		{
			$user[$user_field_id] = $dbh->prepare_str(newsys_str_parse($user[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql .= "	ud_$user_field_id	= '{$user[$user_field_id]}',";
		}

		$sql .= "
					email			= '{$user["email"]}',
					type			= '{$user["type"]}'
				WHERE
					user_id			= '{$user["user_id"]}'";

		$dbh->query($sql,DB_NULL);

		/* Update cache */
		if (newsys_conf("use_cache"))
			newsys_cache_expire("user",$user["user_id"]);

		return array(E_NEWSYS_NONE,array());
	}

	function newsys_user_get_id_by_name($dbh,$name)
	{
		$name = $dbh->prepare_str($dbh);
		return $dbh->query("	SELECT
						user_id
					FROM
						newsys_users
					WHERE
						username = '$name'",DB_COL);
	}

	function newsys_user_get($dbh,$user_id)
	{
		$user_id	= (int)$user_id;
		$user		= $dbh->query("	SELECT
							*
						FROM
							newsys_users
						WHERE
							user_id = $user_id",DB_ROW);

		if (is_array($user))
		{
			$data = array();

			foreach ($user as $id => $val)
				if (preg_match("/^ud_/",$id))
					$data[preg_replace("/^ud_/","",$id)] = $val;
				else
					$data[$id] = $val;

			$user = $data;
		}

		return $user;
	}

	function newsys_user_remove($dbh,$user_id)
	{
		$user_id = (int)$user_id;

		/* Update cache */
		if (newsys_conf("use_cache"))
		{
			# Delete all comments by user
			$dbh->query("	SELECT
						story_id,
						comment_id
					FROM
						newsys_comments
					WHERE
						author_id = $user_id",DB_ROWS);

			while (list ($story_id,$comment_id) = $dbh->fetch_row())
				newsys_cache_delete("comment",$story_id,$comment_id);

			# Delete all stories by user
			$dbh->query("	SELECT
						story_id
					FROM
						newsys_stories
					WHERE
						author_id = $user_id",DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				newsys_cache_delete("story",$id);

			$dbh->query("	SELECT
						story_id
					FROM
						newsys_last_stories
					WHERE
						author_id = $user_id",DB_ROWS);

			$ids = array();

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);

			if (count($ids))
				newsys_rebuild_latest_stories();

			newsys_cache_delete("user",$user_id);
		}

		/* Update database */
		$dbh->query("	DELETE FROM
					newsys_stories
				WHERE
					author_id	= $user_id",DB_NULL);

		$dbh->query("	DELETE FROM
					newsys_comments
				WHERE
					author_id	= $user_id",DB_NULL);

		return $dbh->query("	DELETE FROM
						newsys_users
					WHERE
						user_id		= $user_id",DB_NULL);
	}
?>
