<?php
	require_once "newsys/main.inc";

	/*
	 * Note that this page allows people with no authorization
	 * whatsoever to sign up for an account, so must be extra
	 * careful
	 */

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();

	$newsys_user_id		= newsys_is_logged_in($newsys_dbh);
	$newsys_skip		= FALSE;

	if ($newsys_user_id)
	{
		/* User is logged in, check priviledges */
		list
		(
			$newsys_user_id,
			$newsys_user_type

		)			= newsys_log_in($newsys_dbh,NEWSYS_ADMIN);
	} else {
		/* User has no privileges, proceed with caution */
		if (!newsys_conf("allow_join"))
			$newsys_skip = TRUE;
	}

	if ($newsys_skip)
	{
		echo	newsys_get_template("header"),
			$newsys_of->header("Error"),
			$newsys_of->p("Self-user registration is currently disabled."),
			newsys_get_template("footer");
	} else {
		$newsys_errors		= E_NS_NONE;
		$newsys_user_errors	= array();
		$newsys_skip		= FALSE;
		$newsys_no_pw_match	= FALSE;
		$newsys_user_fields	= newsys_conf("user_fields");

		if (@$_POST["newsys_t_submitted"])
		{
			if (@$_POST["password"] == @$_POST["password2"])
			{
				$newsys_new_user_type = NEWSYS_COMMENTER;

				if ($newsys_user_id)
					$newsys_new_user_type = @$_POST["type"];

				$newsys_user =	array
						(
							'username'	=> @$_POST["username"],
							'password'	=> @$_POST["password"],
							'email'		=> @$_POST["email"],
							'type'		=> $newsys_new_user_type
						);

				foreach (array_keys($newsys_user_fields) as $newsys_field_id)
					$newsys_user[$newsys_field_id] = @$_POST[$newsys_field_id];

				list
				(
					$newsys_errors,
					$newsys_user_errors

				) =	newsys_user_add($newsys_dbh,$newsys_user);

				if ($newsys_errors == E_NS_NONE && !count($newsys_user_errors))
				{
					echo	newsys_get_template("header"),
						$newsys_of->header("User Added"),
						$newsys_of->p("The new user was successfully created."),
						newsys_of_actions($newsys_of,$newsys_user_type),
						newsys_get_template("footer");

					$newsys_skip = TRUE;
				}
			} else {
				$newsys_no_pw_match = TRUE;
			}
		}

		if (!$newsys_skip)
		{
			echo	newsys_get_template("header"),
				$newsys_of->header($newsys_user_id ? "Adding User" : "Signing Up");

			if ($newsys_errors != E_NS_NONE || count($newsys_user_errors) || $newsys_no_pw_match)
			{
				$newsys_t = "";

				if ($newsys_errors & E_NS_USER_NAME)		$newsys_t .= " Please enter an alphanumeric-only username.";
				if ($newsys_errors & E_NS_USER_NAME_USE)	$newsys_t .= " This username is already in use.";
				if ($newsys_errors & E_NS_USER_PASS)		$newsys_t .= " Please enter an alphanumeric-only password.";
				if ($newsys_errors & E_NS_USER_EMAIL)		$newsys_t .= " Please enter a valid e-mail address.";
				if ($newsys_no_pw_match)			$newsys_t .= " Your passwords must match.";

				foreach ($newsys_user_errors as $newsys_user_error)
					$newsys_t .= " " . $newsys_user_fields[$newsys_user_error]["error_message"];

				echo $newsys_of->p($newsys_t);
			}

			echo	$newsys_of->form_start(array()),
					$newsys_of->table_start(array('class' => "newsysTable")),
						$newsys_of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => "Adding A User")),
						$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Username:"),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "text",		'name' => "username")))),
						$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Password:"),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "password",		'name' => "password")))),
						$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Verify Password:"),		array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "password",		'name' => "password2")))),
						$newsys_of->table_row(array('class' => "newsysDesc",'value' => "E-mail Address:"),		array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "text",		'name' => "email"))));

			foreach ($newsys_user_fields as $newsys_field_id => $newsys_field)
				echo		$newsys_of->table_row(array('class' => "newsysDesc",'value' => $newsys_field['name'] . ':'),	array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => $newsys_field['type'],	'name' => $newsys_field_id,'value' => $newsys_field['default_value']))));

			if ($newsys_user_id)
				echo		$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Access Level:"),		array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "select",		'name' => "type",'options' => newsys_conf("levels")))));

			echo			$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Add",'name' => "newsys_t_submitted",'onclick' => newsys_js_dbl_submit())) .
																$newsys_of->input(array('type' => "reset", 'class' => "newsysButton",'value' => "Clear")))),
					$newsys_of->table_end(),
				$newsys_of->form_end();

			if ($newsys_user_id)
				echo newsys_of_actions($newsys_of,$newsys_user_type);

			echo	newsys_get_template("footer");
		}
	}
?>
