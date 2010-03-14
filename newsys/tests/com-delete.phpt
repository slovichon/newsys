<?php
	define("NEWSYS_SKIP_INIT",TRUE,TRUE);

	require "newsys/main.inc";

	$comments =	array
			(
				1 => array(2,3,4), # 2 in is 1
				2 => array(5,6,7,8,9), # 6 is in 2
				3 => array(),
				4 => array(10,11,12,13),
				5 => array(),
				6 => array(14,15), # 15 is in 6
				7 => array(),
				8 => array(),
				9 => array(),
				10 => array(),
				11 => array(),
				12 => array(),
				13 => array(),
				14 => array(),
				15 => array(16,17), # 17 is in 15
				16 => array(),
				17 => array()
			);

	# should return array of (15,6,2,1)
	print_r(newsys_comment_get_ancestors(17,$comments));

?>
