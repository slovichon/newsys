<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 10:40:08 AM
	 *
	 * crypt.inc - cryptographic routines
	 */

	# newsys_rand_str() types
	define("NEWSYS_RAND_ALNUM",	1,TRUE);	# Alpha-numeric
	define("NEWSYS_RAND_VIS_NQ",	2,TRUE);	# All visual characters, no quotes

	function newsys_crypt($data)
	{
		return crypt($data,newsys_conf('crypt_key'));
	}

	function newsys_crypt_key_len($type)
	{
		$len = array(2,13);

		switch ($type)
		{
			case 'blowfish':	$len = array(29,60);	break;
			case 'md5':		$len = array(15,34);	break;
			case 'ext_des':		$len = array(9,13);	break;
			default:		$len = array(2,13);	break;
		}

		return $len;
	}

	function newsys_crypt_gen_key($type = "blowfish")
	{
		$key = "";

		if ($type == "blowfish"	&& !CRYPT_BLOWFISH)	$type = "md5";
		if ($type == "md5"	&& !CRYPT_MD5)		$type = "ext_des";
		if ($type == "ext_des"	&& !CRYPT_EXT_DES)	$type = "default";

		switch ($type)
		{
			case "blowfish":	$key = '$2a$05$'	. newsys_rand_str(22,NEWSYS_RAND_ALNUM);	break;
			case "md5":		$key = '$1$'		. newsys_rand_str(12);				break;
			case "ext_des":		$key =			  newsys_rand_str(9);				break;
			default:		$key =			  newsys_rand_str(2);
		}

		return $key;
	}

	function newsys_rand_str($size,$type = NEWSYS_RAND_VIS_NQ)
	{
		$random		= "";
		$rand		= "";

		$min		= 0;
		$max		= 0;

		$excluded	= array();

		switch ($type)
		{
			case NEWSYS_RAND_VIS_NQ:
			{
				$min		= 33;
				$max		= 126;
				$excluded	= array(39,34,92); # ' " and \
				break;
			}

			case NEWSYS_RAND_ALNUM:
			{
				$min		= 47;
				$max		= 122;
				$excluded	= array(58,59,60,61,62,63,64,91,92,93,94,95,96);
				break;
			}

			default:
				newsys_handle_error("newsys_rand_str(): invalid type; type: $type");
		}

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
?>
