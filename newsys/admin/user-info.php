<?php
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)			= newsys_log_in($newsys_dbh,NEWSYS_COMMENTER);
	$newsys_errors		= E_NS_NONE;
	$newsys_user_errors	= array();
	$newsys_matches		= array();
	$newsys_skip		= FALSE;
	$newsys_bad_pass	= FALSE;
	$newsys_no_pw_match	= FALSE;
	$newsys_user_fields	= newsys_conf("user_fields");

	# By default, we're assuming the user who is loading our page is changing their own
	# information. This may not be the case if an admin is changing someone else's info,
	# the case in which we'll need to grab this variable.
	$newsys_target_user_id	= @$_POST["target_user_id"] ? $_POST["target_user_id"] : $newsys_user_id;

	# Load old information for the user so that he/she can make relative changes.
	$newsys_user	= $newsys_dbh->query("	SELECT
							*
						FROM
							newsys_users
						WHERE
							user_id = $newsys_target_user_id",DB_ROW);

	# Only proceed to change the user info if user is changing their own info or has
	# the authority to change someone else's.
	if (@$_POST["newsys_t_submitted"] && ($newsys_user_id == $newsys_target_user_id || $newsys_user_type >= NEWSYS_ADMIN))
	{
		/* Only non-admins need to re-authenticate */
		if ($newsys_user_type != NEWSYS_ADMIN)
			if (newsys_crypt(@$_POST["old_password"]) != $newsys_user["password"])
				$newsys_bad_pass = TRUE;

		$newsys_type = $newsys_user["type"];

		/* Only admins are allowed to change user types */
		if ($newsys_user_type == NEWSYS_ADMIN)
			$newsys_type = (int)@$_POST["type"];

		if (!$newsys_bad_pass)
		{
			if (@$_POST["password"] == @$_POST["password2"])
			{
				$newsys_new_user =	array
							(
								"user_id"	=> $newsys_target_user_id,
								"email"		=> @$_POST["email"],
								"type"		=> $newsys_type
							);

				/* If the password has changed, specify so */
				if (@$_POST["password"])
					$newsys_new_user["password"] = $_POST["password"];

				foreach (array_keys($newsys_user_fields) as $newsys_field_id)
					$newsys_new_user[$newsys_field_id] = @$_POST[$newsys_field_id];

				list
				(
					$newsys_errors,
					$newsys_user_errors

				) = newsys_user_update($newsys_dbh,$newsys_new_user);

				if ($newsys_errors == E_NS_NONE && !count($newsys_user_errors))
				{
					echo	newsys_get_template("header"),
						$newsys_of->header("User Information Updated"),
						$newsys_of->p("Your user information has been successfully updated."),
						newsys_of_actions($newsys_of,$newsys_user_type),
						newsys_get_template("footer");

					$newsys_skip = TRUE;
				}
			} else {
				$newsys_no_pw_match = TRUE;
			}
		}
	}

	if (!$newsys_skip)
	{
		echo	newsys_get_template("header"),
			$newsys_of->header("Updating User Information");

		if ($newsys_errors != E_NS_NONE || count($newsys_user_errors) || $newsys_bad_pass || $newsys_no_pw_match)
		{
			$newsys_t = "";

			if ($newsys_errors & E_NS_USER_PASS)	$newsys_t .= " Please enter a password of at least five characters..";
			if ($newsys_errors & E_NS_USER_EMAIL)	$newsys_t .= " Please enter a valid e-mail address.";
			if ($newsys_bad_pass)			$newsys_t .= " Please enter your current password.";
			if ($newsys_no_pw_match)		$newsys_t .= " Your passwords do not match.";

			foreach ($newsys_user_errors as $newsys_user_error)
				$newsys_t .= " " . $newsys_user_fields[$newsys_user_error]["error_message"];

			echo $newsys_of->p($newsys_t);
		}

		$newsys_path = newsys_build_path();

		echo	$newsys_of->form_start(array()),
				$newsys_of->table_start(array('class' => "newsysTable")),
					$newsys_of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => "User Profile")),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Username:"),			array('class' => newsys_gen_class(),'value' => $newsys_user["username"]));

		if ($newsys_user_type != NEWSYS_ADMIN)
			echo		 $newsys_of->table_row(array('class' => "newsysDesc",'value' => "Old Password:"),		array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "password",		'name' => "old_password"))));

		echo			$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Password:"),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "password",		'name' => "password")) . " [" . newsys_of_popup($newsys_of,"Help","$newsys_path/help.php?id=".newsys_help_id("confirm password")) . "]")),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Verify Password:"),		array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "password",		'name' => "password2")))),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "E-mail Address:"),		array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "text",		'name' => "email",'value' => $newsys_user["email"]))));

		foreach ($newsys_user_fields as $newsys_field_id => $newsys_field)
			echo		$newsys_of->table_row(array('class' => "newsysDesc",'value' => $newsys_field['name'] . ':'),	array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => $newsys_field['type'],	'name' => $newsys_field_id,'value' => $newsys_user[$newsys_field_id]))));

		if ($newsys_user_type == NEWSYS_ADMIN)
			echo		$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Access Level:"),		array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "select",		'name' => "type",'options' => newsys_conf("levels"),'value' => $newsys_user["type"]))));

		echo			$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$newsys_of->input(array('type' => "hidden",'name' => "target_user_id",'value' => $newsys_target_user_id)) .
															$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Update",'name' => "newsys_t_submitted",'onclick' => newsys_js_dbl_submit())))),
				$newsys_of->table_end(),
			$newsys_of->form_end(),
			newsys_of_actions($newsys_of,$newsys_user_type),
			newsys_get_template("footer");
	}
?>
