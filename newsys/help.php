<?php
	define("NEWSYS_SKIP_INIT",TRUE,TRUE);

	require_once "newsys/main.inc";

	$newsys_help_id		= (int)@$_GET["id"];
	$newsys_help_title	= "";
	$newsys_help		= "";

	switch ($newsys_help_id)
	{
		case newsys_help_id("overview break"):
			$newsys_help_title	= "Overview Break";
			$newsys_help		=	array
						(
							"This value is used while filling out a story to denote where the automatic story overview breaks off.",
							"Instead of filling out the both the overview and story, you can just specify where at in your story you would like the overview to break off, and the overview will be copied from the beginning of the story to this breakpoint."
						);
			break;

		case newsys_help_id("auto-urls"):
			$newsys_help_title	= "Auto-URLs";
			$newsys_help		=	array
						(
							"Automatic URL generation is a feature where URLs that are entered in user input, such as stories are comments, are automatically turned into hyperlinks on medium that are capable or rendering such.",
							"This value is influenced by the Newsys configuration directive `story_auto_url_tlds' which is an array containing matches for top-level domains (such as `com' or `net') for which automatic URL generation will occur."
						);
			break;

		case newsys_help_id("allowed html"):
			$newsys_help_title	= "Allowed HTML";
			$newsys_help		=	array
						(
							"This value corresponds to the Newsys configuration directive `story_allowed_html' which should be an array of HTML tagnames which are allowed to be used in user input, such as stories or comments.",
							"HTML tags found in user input that do not match entries from this directive are escaped."
						);
			break;

		case newsys_help_id("html attributes"):
			$newsys_help_title	= "HTML Attributes";
			$newsys_help		=	array
						(
							"This value reflects which attributes are allowed in HTML tags from user input. This value corresponds to the `story_allowed_attr' directive from the Newsys configuration.",
							"Attributes found from user input not also found in this directive are removed."
						);
			break;

		case newsys_help_id("clear rss"):
			$newsys_help_title	= "Clear RSS";
			$newsys_help		=	array
						(
							"RSS is an acronym for Rich Site Summary. It is a type of RDF (Resource Description Framework) for describing resources available on Web sites. ",
							"Newsys makes use of RSS, should you turn this functionality on, by placing the latest-posted stories into an RSS file, enabling others to quickly and efficiently gather information about your latest stories without having to parse your pages' output."
						);
			break;

		case newsys_help_id("clear cache"):
			$newsys_help_title	= "Clear Cache";
			$newsys_help		=	array
						(
							"Newsys provides a caching system speed up page response times. In an ideal situation, a database connection is not even made, which can, under certain circumstances, maximize efficiency on your site to alow more users to access your site resources more quickly.",
							"You may also turn off this functionality if you so desire."
						);
			break;

		case newsys_help_id("password confirm"):
			$newsys_help_title	= "Password Confirm";
			$newsys_help		=	array
						(
							"While updating your profile, if you specify a new password, it will be changed. If you do not, your password will be left unchanged instead of requiring you to re-type your password during each profile update."
						);
			break;

		default:
			$newsys_help_id	= 0;
			$newsys_help	= array("We could not find help for this item.");
	}

	$newsys_of = newsys_get_of();

	echo newsys_get_template("quick_header");

	if ($newsys_help_id)
		echo $newsys_of->header("Item #$newsys_help_id: $newsys_help_title");

	foreach ($newsys_help as $newsys__i_help)
		echo $newsys_of->p($newsys__i_help);

	echo	newsys_of_close_window($newsys_of,"Close Window"),
		newsys_get_template("quick_footer");
?>
