<?php
	define("NEWSYS_SKIP_INIT",TRUE,TRUE);

	require "newsys/main.inc";

	newsys_conf("story_allowed_html",array('a'));
	newsys_conf("story_allowed_attr",array('href'));

	$data = "Read foo.8\nMy favorite site is easyphp.net www.easyphp.net http://www.easyphp.net keep in mind that i also enjoy http://www.php.net/articles/foo.html, you know
	
	
 i also like http://www.cats.com/incoming/foo.php?foo=bar&cat=dog. i recommend you visit it

 i also like http://www.cats.com/incoming/foo.php
 
 i also like http://www.cats.com/incoming/

 along with <a href='foobar' dog=cat foo=\"bar\">FOOBAR</a>

 i also like http://www.cats.com/incoming

	Directly quoted from www.php.net:

\"The Software & Support Verlag, based in Frankfurt, Germany, has now published a magazine on PHP as a tribute to the steadily growing German PHP community. Editor in chief of the new magazine is Bjoern Schotte. More information can be found at www.phpmag.de.\"";

	$data = "
	
	visit <a href=\"/foo/bar.php\">my website</a>
	
	";

	$data = newsys_str_parse($data,NEWSYS_STR_URL | NEWSYS_STR_HTML);

	echo preg_replace("!<br />!","\n",$data),"\n";
?>
