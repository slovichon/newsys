<?php
	$value = "";

	switch (1)
	{
		case CRYPT_BLOWFISH:	$value = "blowfish";	break;
		case CRYPT_MD5:		$value = "md5";		break;
		case CRYPT_EXT_DES:	$value = "ext_des";	break;
		default:		$value = "default";
	}

	echo $value;
?>
