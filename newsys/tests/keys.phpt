<?php
	define("NEWSYS_SKIP_INIT",TRUE,TRUE);

	require "newsys/main.inc";

//echo newsys_t_gen_key("ext_des");exit();
	newsys_conf("crypt_key",'$2a$05$vljAeBA3IZwmIXHhqsAsNI');
echo newsys_crypt('Iek49jf8');exit();


	echo
"
update newsys_users set password = '",newsys_crypt("sdf"),"'	where username = 'jaredy';\n",
"";


?>
