<?php
	function get_file_content($file)
	{
		global $php_errormsg;

		$fp = @fopen($file,"r");

		if (!$fp)
			die("Cannot open file $file");

		$size = @filesize($file);

		$data = fread($fp,$size);

		fclose($fp);

		return $data;
	}

	function dump($data,$file,$mode)
	{
		touch($file)		or die("Cannot touch file $file");
		chmod($file,$mode)	or die("Cannot chmod file $file");

		$fp = @fopen($file,"w");

		if (!$fp)
			die("Cannot open file $file");

		fputs($fp,$data);

		fclose($fp);

		return;
	}

	function handle_fatal_error($msg)
	{
		echo "A fatal error has occured: $msg\n";

		exit(1);
	}

	define("NEWSYS_SKIP_INIT",TRUE,TRUE);
	require_once "newsys/main.inc";
	require_once "CPL/1.0.inc";

	CPL::error_handler('handle_fatal_error');

	$stdin = fopen("php://stdin","r");

	$password_types	=	array
				(
					'default' => 'Default'
				);

	if (CRYPT_BLOWFISH)
		$password_types['blowfish'] = 'Blowfish***';

	if (CRYPT_MD5)
		$password_types['md5'] = 'MD5**';

	if (CRYPT_EXT_DES)
		$password_types['ext_des'] = 'Extended DES*';

	while (TRUE)
	{
		echo	"You will need to select a password hashing type\n",
			"so that Newsys can determine the length of passwords\n",
			"that will be stored in the database.\n\n",
			"Available password types are:\n",
			"+---------------+---------------\n",
			"| Password type\t| Description\n",
			"+---------------+---------------\n";

		foreach ($password_types as $type => $desc)
			echo "| $type\t",(strlen($type)<5?"\t":""),"| $desc\n";

		echo	"+---------------+---------------\n",
			"Note that stars next to a password type denote its\n",
			"preferability.\n\n";

		echo	"Password type: ";
		$password_type = chop(fgets($stdin,30));

		if (array_key_exists($password_type,$password_types))
			break;
		else
			echo "\nInvalid password type; try again\n";
	}

	echo "Administrator username: ";
	$username	= chop(fgets($stdin,30));

	$password	= "";
	$password2	= "";

	while (TRUE)
	{
		echo "Administrator password (will not echo): ";
		system("stty -echo");
		$password = chop(fgets($stdin,50));
		system("stty echo");

		echo "\nConfirm password: ";
		system('stty -echo');
		$password2 = chop(fgets($stdin,50));
		system('stty echo');

		if ($password == $password2)
			break;
		else
			echo "\nPasswords do not match; try again\n";
	}

	$password = newsys_crypt($password);

	/* Should we validate this in a loop? */
	echo "\nAdministrator e-mail address: ";
	$email		= chop(fgets($stdin,30));

	#######################################################################

	echo 	"\nWe will now move into database configuration. It is assumed that\n",
		"you have an operational RDBMS setup with a viable database for your\n",
		"newsys installation. If you do not, you can do so now and afterwards\n",
		"continue with this installation process.\n\n";

	$dbh_type	= "";
	$supported_dbs	= array('MySQL'=>"MySQL");
	
	while (TRUE)
	{
		echo	"You will now be asked to select a database type. Note that\n",
			"the type you select must be supported by CPL's DBH class. For\n",
			"more information on which databases are supported, consult the\n",
			"CPL Web page (http://www.easyphp.net/projects/cpl/).\n\n",
			"Available database types are:\n",
			"+-------+--------------\n",
			"| RDBMS\t| CPL Driver\n",
			"+-------+--------------\n";

		foreach ($supported_dbs as $i_name => $i_val)
			echo "| $i_name\t| $i_val\n";

		echo	"+-------+--------------\n\n";

		echo "Database type: ";
		$dbh_type = chop(fgets($stdin,10));

		if (in_array($dbh_type,$supported_dbs))
			break;
		else
			echo "Unsupported database type; try again\n";
	}

	echo "Database host: ";
	$dbh_host = chop(fgets($stdin,100));

	echo "Database username: ";
	$dbh_username = chop(fgets($stdin,50));

	echo "Database password (will not echo): ";
	system('stty -echo');
	$dbh_password = chop(fgets($stdin,50));
	system('stty echo');

	echo "\nDatabase name: ";
	$dbh_database = chop(fgets($stdin,50));

	######################################################################

	echo "\nGenerating a crypto key...";

	$key	= newsys_t_gen_key($password_type);

	echo " done.\n";
	
	list
	(
		$key_len,
		$pass_len

	)	= newsys_t_key_len($password_type);

	newsys_conf('crypt_key',$key);

	######################################################################
	# Generate create.sql

	echo "Generating SQL...";
	
	$data		= get_file_content("create.sql.in");
	$subs		=	array
				(
					'CRYPT_KEY_LEN'	=> $key_len,
					'USERNAME'	=> $username,
					'PASSWORD'	=> newsys_crypt($password),
					'EMAIL'		=> $email,
					'PASSWORD_LEN'	=> $pass_len
				);

	/* Comments aren't important here and may even screw things up */
	$data		= preg_replace("!/\*.*?\*/!s","",$data);
	$data		= preg_replace("/--.*/","",$data); // note for the record that -- is a stupid comment

	foreach ($subs as $pat => $sub)
		$data = str_replace("@@$pat@@",$sub,$data);

	echo " done.\n";

	require_once "DBH-$dbh_type/1.7.inc";

	$class = "DBH_$dbh_type";

	echo "Connecting to RDBMS...";

	$dbh = new $class($dbh_host,$dbh_username,$dbh_password,$dbh_database);

	echo "...";

	foreach (explode(';',$data) as $sql)
		if (chop($sql))
			$dbh->query($sql,DB_NULL);
	
	echo " done.\n";
	######################################################################

	######################################################################
	# Generate newsys-config.inc

	echo "Generating newsys configuration file...";
	
	$data		= get_file_content("newsys-config.inc.in");
	$subs		=	array
				(
					'CRYPT_KEY'	=> $key,
					'EMAIL'		=> $email,
					'DBH_TYPE'	=> $dbh_type,
					'DBH_HOST'	=> $dbh_host,
					'DBH_USERNAME'	=> $dbh_username,
					'DBH_PASSWORD'	=> $dbh_password,
					'DBH_DATABASE'	=> $dbh_database
				);

	foreach ($subs as $pat => $sub)
		$data = str_replace("@@$pat@@",$sub,$data);

	dump($data,"newsys-config.inc",0600);

	echo " done.\n";
	
	######################################################################

	echo "
Move the newly-generated ``./newsys-config.inc'' to a safe place where
other users cannot access it, preferably to a place outside of the web-
accessible directory structure

Make sure the directory in which ``newsys-config.inc'' will be placed
in is in PHP's `include_path' directive.

Further configure the directives in this configuration file (see
`doc/directives')

Copy the `templates-dist' directory from this `install' directory or
make your own template directory (see `doc/templates')

Remove this ``install'' sub-directory.
";
?>
