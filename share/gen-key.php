<?php
	function gen_key()
	{
		$key = "";

		if (CRYPT_BLOWFISH)
		{
			$key = '$2a$05$' . rand_str(22);

		} elseif (CRYPT_MD5) {

			$key = '$1$' . rand_str(8);

		} elseif (CRYPT_EXT_DES) {

			$key = rand_str(12);
		} else {
			$key = rand_str(8);
		}

		return $key;
	}

	function rand_str($size)
	{
		global $php_errormsg;

		$rand	= "";
		$min	= 33;	# lowest visable ascii char value
		$max	= 126;	# highest
/*
		if (file_exists("/dev/urandom"))
		{
			$fp = @fopen("/dev/urandom","r");

			if (!$fp)
				die("Cannot open file; file: /dev/urandom");

			$i = 0;

			while ($i++ < $size)
			{
				warn(".");
				$rand .= chr((ord(fread($fp,1)) % ($max - $min)) + $min);
			}

			fclose($fp);
		} else {
*/			$i = 0;

			while ($i++ < $size)
			{
				warn(".");
				$rand .= chr(mt_rand($min,$max));
			}
/*		}
*/
		return $rand;
	}

	function warn($msg)
	{
		static $fp;

		if (!$fp)
			$fp = fopen("php://stderr","w");

		fputs($fp,$msg);
	}

	echo gen_key();
?>
