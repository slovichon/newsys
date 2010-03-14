<?php
	define("NEWSYS_SKIP_INIT",TRUE,TRUE);

	require "newsys/main.inc";

	CPL::display_errors(TRUE);
	newsys_conf("show_errors",TRUE);

	$ts = new TimeStamp();

	$ts->load_current();

	$story =	array
			(
				'story_id'	=> 59,
				'headline'	=> "Da Fuckn Test",
				'overview'	=> "Welcome to da fucking test",
				'date'		=> $ts->get_unix()
			);

#	$doc = newsys_xml_add(NEWSYS_XML_STORIES,$story);
	$doc = newsys_xml_setup(NEWSYS_XML_STORIES);
	$doc = newsys_xml_remove(NEWSYS_XML_STORIES,array("story_id" => 59),$doc);

	echo $doc->dump_mem(TRUE);
?>
