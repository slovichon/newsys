<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 04:40:19 PM
	 *
	 * misc.inc - miscellaneous routines
	 */

	# newsys_str_parse() types
	define("NEWSYS_STR_NONE",	1,TRUE);
	define("NEWSYS_STR_HTML",	2,TRUE);
	define("NEWSYS_STR_URL",	4,TRUE);
	define("NEWSYS_STR_ALL",	NEWSYS_STR_HTML | NEWSYS_STR_URL,TRUE);

	# newsys_build_path() types
	define("NEWSYS_PATH_ABS",	1,TRUE);
	define("NEWSYS_PATH_REL",	2,TRUE);
	define("NEWSYS_PATH_SYS",	3,TRUE);

	function newsys_build_path($type = NEWSYS_PATH_REL)
	{
		switch ($type)
		{
			case NEWSYS_PATH_REL: return newsys_conf("web_root").newsys_conf("path");
			case NEWSYS_PATH_SYS: return newsys_conf("sys_root").newsys_conf("path");
			case NEWSYS_PATH_ABS: return newsys_conf("site_uri").newsys_conf("path");
			default:
				newsys_handle_error("newsys_build_path(): invalid type; type: $type");
		}
	}

	function newsys_gen_class()
	{
		static $swt = TRUE;

		$swt = !$swt;

		return "newsysData" . ($count ? "1" : "2");
	}

	function newsys_move_file($src,$dst)
	{
		global $php_errormsg;

		if (!@copy($src,$dst))
			newsys_handle_error("newsys_move_file(): cannot copy file; src: $src; dst: $dst");

		newsys_unlink($src);

		return;
	}

	function newsys_get_dbh()
	{
		$dbh_type = newsys_conf("dbh_type");

		$class	= "DBH_$dbh_type";
		$file	= "DBH-$dbh_type/1.7.inc";

		require_once $file;

		return	new $class
			(
				newsys_conf("dbh_host"),
				newsys_conf("dbh_username"),
				newsys_conf("dbh_password"),
				newsys_conf("dbh_database")
			);
	}

	function newsys_get_of()
	{
		$of_type = newsys_conf("of_type");

		$class	= "OF_$of_type";
		$file	= "OF-$of_type/1.7.inc";

		require_once $file;

		return new $class();
	}

	function newsys_handle_error($msg)
	{ 
		# To monitor infinite recursion
		static $count = 0;
		global $php_errormsg;

		$msg .=	"; Date: " . date("F d, Y h:i:s A") .
			"; File: " . $_SERVER["PHP_SELF"];

		if (ini_get('track_errors'))
			$msg .= "; PHP Error: $php_errormsg";

		$msg .= "\n";

		if (newsys_conf("trigger_errors"))
			trigger_error($msg,E_USER_ERROR);

		if ($log = newsys_conf("error_log"))
			if ($fp = @fopen($log,"a"))
			{
				fputs($fp,$msg);
				fclose($fp);
			}

		/*
		 * Mail error report
		 *
		 * Note that if we used newsys_mail(), it could recursively
		 * throw errors
		 */
		if (newsys_conf("mail_errors"))
			@mail
			(
				newsys_conf("site_email"),
				newsys_conf("site_name") . " Newsys Errors",
				$msg,
				"From: " . newsys_conf("site_email")
			);

		if (newsys_conf("show_errors"))
		{
			if ($count++)
			{
				# Ok, no fancy stuff, just exit
				echo	"The following error occurred while processing that directive: $msg";
			} else {
				$of = newsys_get_of();

				echo	newsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("The following error occurred while processing that directive: $msg"),
					newsys_get_template("footer");
			}
		} else {
			if ($count++)
			{
				# Same here; nothing fancy
				echo	"An error has occurred. Please contact the site administrator and report this error.";
			} else {
				$of = newsys_get_of();

				echo	newsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("An error has occurred. Please contact the site administrator and report this error."),
					newsys_get_template("footer");
			}
		}

		exit(1);
	}

	function newsys_redirect($url)
	{
		# Clear buffer if there is stuff in it
		if (headers_sent())
			ob_end_clean();

		if (preg_match("!^[a-z]+:!",$url))
		{
			# Checks for a complete URI; we're done

		} elseif (preg_match("!^/!",$url)) {

			# Checks for relative path
			$url	= newsys_conf("site_uri") . $url;
		} else {
			# Must be relative to '.'
			$path	= $_SERVER["PHP_SELF"];
			$path	= preg_replace("![^/]+$!","",$path);
			$url	= newsys_conf("site_uri") . $path . $url;
		}

		header("Location: $url");

		exit(0);
	}

	function newsys_mail($to,$from,$subject,$body)
	{
		global $php_errormsg;

		# Strip possible CRLFs in the header
		$from = preg_replace("/(?:\r\n|[\r\n])+/","",$from);

		if (!@mail($to,$subject,$body,"From: $from"))
			newsys_handle_error("newsys_mail: error in mail()");

		return;
	}

	/*
	 * This temporary function is just used to clean up attributes
	 * found by newsys_str_parse()
	 */
	function newsys_clean_attr($name,$val)
	{
		# Attributes will be matched with their delimiters (" or ')
		$val = preg_replace("/^&quot;|^'|&quot;$|'$/","",$val);

		# Attributes will have been subject to htmlEntities()
# But should we pass them to html_entity_decode()?
		$val = html_entity_decode($val);

		if ($name == "style" && newsys_conf("story_strip_expr") &&
			preg_match_all("/([a-zA-Z-]+)\s*:\s*expression(/",$val,$matches))
		{
			# $matches[0] will contain the whole string
			array_shift($matches);
			$val = newsys_css_remove($matches,$val);
		}

		return $attr;
	}

	function newsys_css_remove($ids,$data)
	{
		$props	= explode(";",$data);
		$len	= count($props);

		for ($i = 0; $i < $len; $i++)
		{
			list ($name) = explode(":",$props[$i]);

			# Found it, remove property
			if (in_array($name,$ids))
				$props[$i--] = $props[--$len];
		}

		return join(";",$props);
	}

	/*
	 * This temporary function is just used to check potentially
	 * arbitrarily-protocoled URIs for disallowed protocols
	 */
	function newsys_check_proto($url)
	{
		if (preg_match("/^\s*(\w+):/",$url,$matches))
			return in_array($matches[1],newsys_conf("story_allowed_proto"));

		return TRUE;
	}

	function newsys_str_parse($str,$flags = NEWSYS_STR_ALL)
	{
		# Convert HTML
		$str = htmlEntities($str);

		if ($flags & NEWSYS_STR_HTML)
		{
			/*
			 * Note: should probably be an option including leaving
			 * alone, blocking, and truncating.
			 */

			# Allowed HTML tags
			$html	= strToLower(join("|",newsys_conf("story_allowed_html")));
			$str	=	preg_replace
					(
						"!
							&lt;			# HTML is escaped
							(
								/?		# Start or end
								(?i:$html)	# Case-insensitive tag match
								\b		# Word boundary
								.*?		# Rest of tag
							)
							&gt;
						!x",
						"<$1>",# "<$tag parse_attr($attr)>" ?
						$str
					);

			# Allowed HTML attributes
			$story_allowed_attr	= newsys_conf("story_allowed_attr");
			$story_attr_proto	= newsys_conf("story_attr_proto");

			while (1)
			{
				$new = preg_replace
				(
					"!
					( # HTML tag ($1)
						<				# HTML tag (allowed & matched)
							\w+
							(?:\s*\w+=\".*?\")*	# Previously-allowed attributes
							\s?			# Just for formatting purposes
					)
							\s*
							(\w+)			# Attribute name ($2)
							\s*
							=
							\s*
							( # Attribute ($3) (One of the following...)
								&quot;.*?&quot;	# Double quotes
								|
								'.*?'		# Single quotes (not changed
								|		#	in htmlEntities()
								[^\"\s>]+	# No quotes, bounded by whitespace
							)
					( # End of tag ($4)
							.*?			# Other tag content
						>				# End of tag
					)
					!xe",
					'
						"$1" .	# Tag and previous attributes
						(
							/*
							 * Validate attribute, must be allowed in
							 * story_allowed_attr and if its of type to be
							 * checked for malicious protocols, subject
							 * to checking
							 */
							in_array("$2",$story_allowed_attr) &&
							(
								in_array("$2",$story_attr_proto) ?

								# Subject to checking if special attribute
								newsys_check_proto(newsys_clean_attr("$3"))

								# Else it is good
								: TRUE
							) ?

							# Formatted attribute
							" $2=\"" . newsys_clean_attr("$3") . "\""

							# Or nothing
							: ""
						) .
						"$4"
					',
					$str,
					1 # We can only perform this match once per loop
				);

				if ($new == $str)
					# Nothing changed, we're done
					break;
				else
					# "Save changes" and continue to next attribute
					$str = $new;
			}
		}

		# Turn URLs into hyperlinks
		if ($flags & NEWSYS_STR_URL)
		{
			if (newsys_conf("story_auto_urls"))
			{
				$tlds = "(?:" .
					join("|",newsys_conf("story_auto_url_tlds")) . ")";

				/*
				 * Since negative lookbehind assertions are not
				 * supported, we'll save want we don't want to
				 * match and check its absence in our
				 * replacement section.
				 *
				 * Below is the following:
				 *	\\1 -> what was matched
				 *	\\2 -> what we don't want to match
				 *	\\3 -> the URL
				 */
/*
 * NOTE: we have a potential bug as this rewrites URLs assuming they're in
 * HTML format. It should actually grab the output of $of->link() and make
 * changes relative to that accordingly.
 */
				$str	=	preg_replace
						(
							"{(	# Everything - in case of failed match ($1)
#								(?<!<a\b[^>]+\bhref\s*=\s*[\"']?|<a\b[^>]+\bhref\b[^>]+>)
								# What we don't want to match ($2)
								(
									# <a href=URL
									<a\n[^>]+\bhref\s*=\s*[\"']?
									|
									# <a>URL</a>
									<a[^>]*>(?!.*</a>.*)
									|
									# URL
								)
								# The URL ($3)
								(
									(?:http:/{1,3})?
									(?:www\\.)?
								#	(?!\d+\\.\d+\b) # To prevent, e.g., `3.3'
									[a-zA-Z0-9-]
									[a-zA-Z0-9.-]+
									\\.
									(?:
										" .
										# check if they want us to match preset tlds
										($tlds == "(?:)" ? "
											[a-zA-Z]+ # tld
										" : "
											$tlds\b
								#			(?=\s|/|$)
										") .
										"
										(?:
											/
											\S*?
											(?=
												[,.:;!]+$|
												[,.:;!]+\s|
												$|
												\s
											) # No trailing punctuation
										)?
								#		\S*
									)
								)
							)}iex",
							'
								"$2" ? "$1" :
								"<a href=\\"" .
								(
									preg_match("!^http://!","$3") ?
									"" :
									"http://"
								) .
								(
									preg_match
									(
										"!/!",
										preg_replace("!^http://!","","$3")
									)
									?
									"$3"
									:
									"$3/"
								) .
								"\\">$3</a>"
							',
							$str
						);
			}
		}

		# Fix newline madness
		$str = preg_replace("/\r\n|(?!<\r)\n|(?!<\n)\r/","<br />",$str);

		# Break up long words
		$str = preg_replace("!([^\s<>/\"']{" . newsys_conf("story_word_length") . "})!","\\1 ",$str);

		return $str;
	}

	function newsys_unlink($file)
	{
		global $php_errormsg;

		if (!@unlink($file))
			newsys_handle_error("newsys_unlink(): cannot unlink() file; file: $file");

		return;
	}

	function newsys_http_post($var)
	{
		if (newsys_conf("check_http_referer") && !newsys_valid_referer())
			return NULL;

		return @$_POST[$var];
	}

	function newsys_valid_referer()
	{
		return preg_match("/^" . preg_quote(newsys_conf("site_uri")) . "/",$_SERVER["HTTP_REFERER"]);
	}

	function newsys_help_id($item)
	{
		switch (strToLower($item))
		{
			case "overview break":		return 101;
			case "auto-urls":		return 102;
			case "allowed html":		return 103;
			case "html attributes":		return 104;
			case "clear rss":		return 105;
			case "clear cache":		return 106;
			case "password confirm":	return 107;
			default:			return 0;
		}
	}
?>
