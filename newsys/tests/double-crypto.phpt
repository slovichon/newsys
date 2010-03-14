<?php
	define("NEWSYS_SKIP_INIT",TRUE,TRUE);

	require "newsys/main.inc";

/*
 * for testing a(b(c)) = b(a(c))
 * where a could be routine for db storage
 * and b could be routine for http session storage
 */
	########################################

	$key1 = newsys_t_gen_key();
	$key2 = newsys_t_gen_key();

	echo "Key 1: $key1\n";
	echo "Key 2: $key2\n";

	########################################

	$data = "sukacak";

	echo "Data: $data\n";

	#######################################

	$newsys_crypt_key = $key1;
	$t = newsys_crypt($data);

	$newsys_crypt_key = $key2;
	$t = newsys_crypt($t);

	echo "1(2(data)): $t\n";

	######################################

	$newsys_crypt_key = $key2;
	$t = newsys_crypt($data);

	$newsys_crypt_key = $key1;
	$t = newsys_crypt($t);

	echo "2(1(data)): $t\n";
?>
