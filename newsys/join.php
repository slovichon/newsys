<?php
	require "newsys.inc";

	$ns_message		= "";
	$ns_of			= newsys_get_of();
	$ns_errors		= E_NS_NONE;
	$ns_user_errors		= array();
	$ns_bad_pass		= FALSE;
	$ns_skip		= FALSE;
	$newsys_user_fields	= newsys_conf("user_fields");

	if (@$_POST["action"] == "Join")
	{
		if (@$_POST["password"] == @$_POST["password2"])
		{
			$ns_dbh = newsys_get_dbh();

			list
			(
				$ns_errors,
				$ns_user_errors

			) =	newsys_user_add
				(
					$ns_dbh,
					array
					(
						'username'	=> @$_POST["username"],
						'password'	=> @$_POST["password"],
						'email'		=> @$_POST["email"],
						'type'		=> NEWSYS_COMMENTER
					)
				);

			if ($ns_errors == E_NS_NONE && !count($ns_user_errors))
			{
				echo	newsys_get_template("header"),
					$ns_of->header("Joined"),
					$ns_of->p("You have successfully signed up! You may now post comments to news stories.");
					newsys_get_template("footer");

				$ns_skip = TRUE;
			}
		} else {
			$ns_bad_pass = TRUE;
		}
	}

	if (!$ns_skip)
	{
		echo	newsys_get_template("header"),
			$ns_of->header("Join");

		if ($ns_errors != E_NS_NONE)
		{
			$ns_errmsg = "";

			if ($ns_bad_pass)			$ns_errmsg .= " Your passwords are not the same.";
			if ($ns_errors & E_NS_USER_NAME)	$ns_errmsg .= " Please enter an alpha-numeric username.";
			if ($ns_errors & E_NS_USER_PASS)	$ns_errmsg .= " Please enter a password between 5 and 30 characters.";
			if ($ns_errors & E_NS_USER_EMAIL)	$ns_errmsg .= " Please enter a valid e-mail address.";
			if ($ns_errors & E_NS_USER_NAME_USE)	$ns_errmsg .= " This username has already been chosen. Please try another.";

			/* Add user-defined error messages */
			foreach ($ns_user_errors as $ns_user_error)
				$ns_errmsg .= " " . $newsys_user_fields[$ns_user_error]["error_msg"];

			echo $ns_of->p($ns_errmsg);
		}

		echo	$ns_of->form
			(
				array(),
				$ns_of->table
				(
					array(),
					$ns_of->table_row(array('class' => "newsysDesc",'value' => "Username:"),	array('class' => "newsysData1",'value' => $ns_of->input(array('type' => "text",		'name' => "username")))),
					$ns_of->table_row(array('class' => "newsysDesc",'value' => "Password:"),	array('class' => "newsysData1",'value' => $ns_of->input(array('type' => "password",	'name' => 'password')))),
					$ns_of->table_row(array('class' => "newsysDesc",'value' => "Verify:"),		array('class' => "newsysData1",'value' => $ns_of->input(array('type' => "password",	'name' => 'password2')))),
					$ns_of->table_row(array('class' => "newsysDesc",'value' => "E-mail Address:"),	array('class' => "newsysData1",'value' => $ns_of->input(array('type' => "email",	'name' => "email")))),
					$ns_of->table_row(array('class' => "newsysFooter",'colspan' => 2,"value" =>	$ns_of->input(array('type' => "submit",	'class' => "newsysButton",'value' => "Join",'name' => "action")) .
															$ns_of->input(array('type' => "reset",	'class' => "newsysButton",'value' => "Clear"))))
				)
			),
			newsys_get_template("footer");
	}
?>
