<!--

You shouldn't be seeing this. If you are, that means
PHP has not been correctly set up. Please double-
check your PHP installation and be sure that PHP
parsing is being executed on `.php' via your Web
server configuration.

-->
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html lang="en-US" xml:lang="en-US" xmlns="http://www.w3.org/1999/xhtml">
	<head>
		<title>Newsys Installer</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
		<link rel="stylesheet" type="text/css" media="screen" href="../default.css" />
	</head>
	<body>
<!--
<?php
	echo "-","-",">";

	define("NEWSYS_SKIP_INIT",TRUE,TRUE);

	require_once "newsys/main.inc";
	require_once "CPL/1.0.inc";

#	CPL::display_errors(TRUE);
	CPL::error_handler("handle_fatal_error");

	function handle_fatal_error($msg)
	{
		global $php_erromsg;

		echo $msg,"; PHP Error Message: ",@$php_errormsg;

		exit(1);
	}

	function get_file_content($file)
	{
		$fp	= @fopen($file,"r");

		if (!$fp)
			handle_fatal_error("Unable to open() file; file: $file");

		$size	= @filesize($file);

		if (!$size)
			handle_fatal_error("Cannot read size of file; file: $file");

		$data	= fread($fp,$size);

		fclose($fp);

		return $data;
	}

	$supported_db =	array
			(
				#"mSQL"		=> "mSQL",
				#"PostgreSQL"	=> "PostgreSQL",
				"MySQL"		=> "MySQL"
			);
	$of = newsys_get_of();

	if (FALSE)
	{
		echo "--><p>You do not have PHP installed. Please install it and try this installation again.</p><!--";
	} else {
		$errmsg = "";

		# validate
		switch (@$_POST['page'])
		{
			case 3:
			{
				if (!preg_match("/^[a-zA-Z0-9_-]{5,30}$/",@$_POST["username"]))	$errmsg .= " Please enter an alpha-numeric username.";
				if (!in_array(@$_POST['dbh_type'],$supported_db))		$errmsg .= " Please select a database type from the list.";
				if (!@$_POST['dbh_host'])					$errmsg .= " Please enter a database host.";
				if (!@$_POST['dbh_database'])					$errmsg .= " Please enter a database name.";
				if (!@$_POST['dbh_username'])					$errmsg .= " Please enter a database username.";
				if (!@$_POST['dbh_password'])					$errmsg .= " Please enter a database password.";
				if (@$_POST['password'] != @$_POST['password2'])		$errmsg .= " Your passwords do not match.";
				if (!mail_is_valid(@$_POST['email']))				$errmsg .= " Please enter an e-mail address.";
				if (!preg_match("/^[^'\"\\\\]{5,30}$/",@$_POST["password"]))	$errmsg .= " Please enter a password without the following characters: '\"\\";

				if ($errmsg)
					$_POST["page"] = 2;
				break;
			}
		}

		switch (@$_POST["page"])
		{
			case 3: # generate key
			{
				# Admin info
				$username	= @$_POST['username'];
				$password	= @$_POST['password'];
				$password2	= @$_POST['password2'];
				$email		= @$_POST['email'];

				$dbh_type	= @$_POST['dbh_type'];
				$dbh_host	= @$_POST['dbh_host'];
				$dbh_username	= @$_POST['dbh_username'];
				$dbh_password	= @$_POST['dbh_password'];
				$dbh_database	= @$_POST['dbh_database'];

				$password_type	= @$_POST['password_type'];

				echo	$of->header("Step 3: Key"),
					$of->p("The first thing I need to do is communicate with the database.");
				flush();

				$class = "DBH_$dbh_type";

				require_once "DBH-$dbh_type/1.7.inc";

				$dbh = new $class($dbh_host,$dbh_username,$dbh_password,$dbh_database);

				$username	= $dbh->prepare_str($username,	SQL_REG);
				$email		= $dbh->prepare_str($email,	SQL_REG);

				echo	$of->p("The next thing I will do is generate an encryption key for storing passwords.");

				list
				(
					$key_len,
					$pass_len

				)	= newsys_t_key_len($password_type);
				$key	= newsys_t_gen_key($password_type);

				newsys_conf('crypt_key',$key);

				echo	$of->p("I am now going to try to setup the databases.");
				flush();

				$sql	= get_file_content("./create.sql.in");

				$subs	=	array
						(
							'USERNAME'	=> $username,
							'PASSWORD'	=> newsys_crypt($password),
							'PASSWORD_LEN'	=> $pass_len,
							'EMAIL'		=> $email,
							'CRYPT_KEY_LEN'	=> $key_len
						);

				/* Comments aren't important here and may even screw things up */
				$sql	= preg_replace("!/\*.*?\*/!s","",$sql);
				$sql	= preg_replace("/--.*/","",$sql);

				foreach ($subs as $pat => $sub)
					$sql = str_replace("@@$pat@@",$sub,$sql);

				$stmts	= explode(';',$sql);

				foreach ($stmts as $stmt)
					if (chop($stmt))
						$dbh->query($stmt,DB_NULL);

				echo	$of->p("The database has been set up correctly. I am now going to try writing the configuration file.");
				flush();

				$config	= get_file_content("./newsys-config.inc.in");

				$subs	=	array
						(
							'CRYPT_KEY'	=> $key,
							'DBH_TYPE'	=> $dbh_type,
							'DBH_HOST'	=> $dbh_host,
							'DBH_USERNAME'	=> $dbh_username,
							'DBH_PASSWORD'	=> $dbh_password,
							'DBH_DATABASE'	=> $dbh_database,
							'EMAIL'		=> $email
						);

				foreach ($subs as $pat => $sub)
					$config = str_replace("@@$pat@@",$sub,$config);

				@touch("./newsys-config.inc")		|| handle_fatal_error("Cannot touch file; file: ./newsys-config.inc. Please make sure the `install' directory has the writable bit set on for whichever user your Web server is running as.");

				@chmod("./newsys-config.inc",0600)	|| handle_fatal_error("Cannot chmod file; file: ./newsys-config.inc");

				$fp = @fopen("./newsys-config.inc","w");

				if (!$fp)
					handle_fatal_error("Cannot write to file; file: ./newsys-config.inc. Please make sure permissions are set correctly to write to new files in the `install' directory.");

				fputs($fp,$config);

				fclose($fp);

				echo	$of->p("The configuration file has been successfully written."),
					$of->p("Congradulations! Newsys has almost been properly installed! Here are some finalizing issues you should take care of:"),
					$of->of_list
					(
						OF_LIST_UN,
						"Move the newly-generated ``./newsys-config.inc'' to a safe place where other users cannot access it, preferably to a place outside of the web-accessible directory structure",
						"Make sure the directory in which ``newsys-config.inc'' will be placed in is in PHP's `include_path' directive.",
						"Further configure the directives in this configuration file (see `doc/directives')",
						"Copy the `templates-dist' directory from this `install' directory or make your own template directory (see `doc/templates')",
						"Remove this ``install'' sub-directory."
					),
					$of->link('Log Into Newsys','../login.php');

				break;
			}

			case 2: # gather input
			{
				$username	= htmlEntities(@$_POST["username"])	|| "";
				$email		= htmlEntities(@$_POST["email"])	|| "";
				$dbh_host	= htmlEntities(@$_POST["dbh_host"])	|| "";
				$dbh_username	= htmlEntities(@$_POST["dbh_username"])	|| "";
				$dbh_database	= htmlEntities(@$_POST["dbh_database"])	|| "";
				$password_types	=	array
							(
								"default"	=> "Default"
							);

				if (defined("CRYPT_BLOWFISH") && constant("CRYPT_BLOWFISH"))
					$password_types['blowfish'] = "Blowfish***";

				if (defined("CRYPT_MD5") && constant("CRYPT_MD5"))
					$password_types['md5'] = "MD5**";

				if (defined("CRYPT_EXT_DES") && constant("CRYPT_EXT_DES"))
					$password_types['ext_des'] = "Extended DES*";

				$def_type = "default";

				switch (TRUE)
				{
					case CRYPT_BLOWFISH:	$def_type = "blowfish";	break;
					case CRYPT_MD5:		$def_type = "md5";	break;
					case CRYPT_EXT_DES:	$def_type = "ext_des";	break;
					#default:		$def_type = "default";	break;
				}

				echo	$of->header("Step 2: Input");

				if ($errmsg)
					echo $of->p("The following error(s) has/have occured: $errmsg");

				echo	$of->p("I need to collect some information for the first user this Newsys installation will implore. Please fill out the fields below."),
					$of->form
					(
						array(),
						$of->table
						(
							array('class' => "newsysTable"),
							$of->table_head("Field","Value"),
							$of->table_row(array('class' => "newsysDesc",'value' => "Username:"),		array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "text",	'name' => "username",		'value' => $username)))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Password:"),		array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "password",	'name' => "password")))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Verify Password:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "password",	'name' => "password2")))),
							$of->table_row(array('class' => "newsysDesc",'value' => "E-mail Address:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "text",	'name' => "email",		'value' => $email)))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Password Type:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "select",	'name' => "password_type",'options' => $password_types,'value' => $def_type)) .
																	$of->p("This field is dependent on the features available on the system on which you are running Newsys. I will need to setup the password length according to which hashing algorithm that will be chosen.") .
																	$of->p("Note that the more stars next to an item on the list denotes its preferability."))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Database Type:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "select",	'name' => "dbh_type",'options' => $supported_db)))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Database Host:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "text",	'name' => "dbh_host",		'value' => $dbh_host)))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Database Username:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "text",	'name' => "dbh_username",	'value' => $dbh_username)))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Database Password:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "password",	'name' => "dbh_password")))),
							$of->table_row(array('class' => "newsysDesc",'value' => "Database Name:"),	array('class' => newsys_gen_class(),'value' => $of->input(array('type' => "text",	'name' => "dbh_database",	'value' => $dbh_database)))),
							$of->table_row(array('colspan' => 2,'class' => "newsysFooter",'value' =>	$of->input(array('type' => "hidden",'name' => "page",'value' => 3)) .
																$of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Next Page")) .
																$of->input(array('type' => "reset", 'class' => "newsysButton",'value' => "Reset"))))
						)
					);

				break;
			}

			default:
			{
				echo
					$of->header("Welcome"),
					$of->p("Welcome to Newsys " . NEWSYS_VERSION . "! I have a few things to do before Newsys will run properly on your system, so let's get to it!"),
					$of->p("However, before we start, make sure you do the following:"),
					$of->of_list
					(
						OF_LIST_UN,
						"Install " . $of->link("CPL","http://www.easyphp.net/projects/cpl") . " and make sure the CPL root directory is in the PHP `include_path' directive value.",
						"Allow whichever user your Web server is running as to create and write new files in this `install' directory."
					),
					$of->form
					(
						array(),
						$of->input(array('type' => "hidden",'name' => "page",'value' => 2)),
						$of->input(array('type' => "submit",'value' => "Next Page",'class' => "newsysButton"))
					);
			}
		}
	}

	echo "<","!","-","-","\n";
?>
-->
	<hr />
	<span class="newsysInfo"><a href="http://www.easyphp.net/projects/newsys">Newsys Project Page</a></span>
	</body>
</html>
