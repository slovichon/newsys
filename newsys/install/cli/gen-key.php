<?php
	function newsys_gen_key()
	{
		$key = "";

		switch (1)
		{
			case CRYPT_BLOWFISH:	$key = '$2a$05$'	. newsys_rand_str(22,47,122,array(58,59,60,61,62,63,64,91,92,93,94,95,96));	break;
			case CRYPT_MD5:		$key = '$1$'		. newsys_rand_str(8);	break;
			case CRYPT_EXT_DES:	$key =			  newsys_rand_str(12);	break;
			default:		$key =			  newsys_rand_str(8);
		}

		return $key;
	}

	function newsys_rand_str($size,$min = 33,$max = 126,$excluded = NULL)
	{
		global $php_errormsg;

		$random	= "";
		$rand	= "";

		if (!is_array($excluded))
			$excluded = array();

		for ($i = 0; $i < $size; $i++)
		{
			do
			{
				$random = mt_rand($min,$max);

			} while (in_array($random,$excluded));

			$rand .= chr($random);
		}

		return $rand;
	}

	echo newsys_gen_key();
?>
