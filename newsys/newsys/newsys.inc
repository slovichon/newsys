<?php
	/*******************************************************
	 * Newsys
	 * Core Newsys library routines
	 *
	 * Consult NEWSYS_ROOT/doc/LICENSE for licensing
	 * information.
	 *
	 * Thursday, January 09, 2003 06:25:57 PM
	 *
	 * Written by Jared Yanovich <jaredy@closeedge.net>
	 *******************************************************/

	define("NEWSYS_VERSION",	"-current",TRUE);

	# Miscellaneous constants
	define("NEWSYS_BUF_SIZE",	1024,TRUE);

	# User-level types
	define("NEWSYS_COMMENTER",	1,TRUE);
	define("NEWSYS_REGULAR",	2,TRUE);
	define("NEWSYS_ADMIN",		3,TRUE);

	# Login types
	define("NEWSYS_AUTH_HTTP",	1,TRUE);
	define("NEWSYS_AUTH_SESSION",	2,TRUE);
	define("NEWSYS_AUTH_COOKIE",	3,TRUE);

	# User-defined field types
	define("NEWSYS_FLD_USER",	1,TRUE);
	define("NEWSYS_FLD_STORY",	2,TRUE);
	define("NEWSYS_FLD_COMMENT",	3,TRUE);

	# newsys_str_parse() types
	define("NEWSYS_STR_NONE",	1,TRUE);
	define("NEWSYS_STR_HTML",	2,TRUE);
	define("NEWSYS_STR_URL",	4,TRUE);
	define("NEWSYS_STR_ALL",	NEWSYS_STR_HTML | NEWSYS_STR_URL,TRUE);

	# newsys_build_path() types
	define("NEWSYS_PATH_ABS",	1,TRUE);
	define("NEWSYS_PATH_REL",	2,TRUE);
	define("NEWSYS_PATH_SYS",	3,TRUE);

	# newsys_rand_str() types
	define("NEWSYS_RAND_ALNUM",	1,TRUE);	# Alpha-numeric
	define("NEWSYS_RAND_VIS_NQ",	2,TRUE);	# All visual characters, no quotes

	# XML control types
	define("NEWSYS_XML_STORIES",1,TRUE);

	# Define error codes
	# Generic
	define("E_NS_NONE",		0,	TRUE);	# No error

	# Users
	# Note: the user error codes are more strict because any user can sign up
	# without permission (potentially)
	define("E_NS_USER_NAME",	1,TRUE);	# Invalid username
	define("E_NS_USER_NAME_USE",	2,TRUE);	# Username already in use
	define("E_NS_USER_PASS",	4,TRUE);	# Invalid password
	define("E_NS_USER_EMAIL",	8,TRUE);	# Invalid e-mail

	# Comments
	define("E_NS_COM_COM",		1,TRUE);	# No comment
	define("E_NS_COM_SUBJECT",	2,TRUE);	# No subject
	define("E_NS_COM_NO_STORY",	4,TRUE);	# Story ID is invalid
	define("E_NS_COM_NOT_ALLOWED",	8,TRUE);	# Commenting not allowed
	define("E_NS_COM_MAX_EXCEED",	16,TRUE);	# Maximum number of comments exceeded

	# Stories
	define("E_NS_STORY_HEADLINE",	1,TRUE);	# No headline
	define("E_NS_STORY_STORY",	2,TRUE);	# No story

	/*
	 * newsys-config.inc will call this function, so it must be
	 * defined before the config file is included
	 */
	function newsys_conf()
	{
		static $vars	= array();

		$args		= func_get_args();
		$argc		= func_num_args();

		if ($argc == 2)
		{
			$vars[$args[0]] = $args[1];

		} elseif ($argc != 1) {

			newsys_handle_error("Wrong number of arguments to newsys_conf()");
		}

		if (!array_key_exists($args[0],$vars))
			newsys_handle_error("Configuration value requested before supplied; key: $args[0]");

		return $vars[$args[0]];
	}

	# Required libraries/resources
	require_once "CPL/1.0.inc";
	require_once "newsys-config.inc";
	require_once "TimeStamp/1.5.inc";

	if (!function_exists("strip_recursive_slashes"))
	{
		function strip_recursive_slashes(&$data)
		{
			if (!is_array($data))
				return;

			foreach ($data as $key => $val)
			{
				if (is_array($val))
					strip_recursive_slashes($data[$key]);
				else
					$data[$key] = stripSlashes($val);
			}

			return;
		}
	}

	/* Only doing this to encapsulate variables */
	if (ini_get('magic_quotes_gpc'))
	{
		strip_recursive_slashes($_POST);
		strip_recursive_slashes($_GET);
		strip_recursive_slashes($_REQUEST);
		strip_recursive_slashes($_COOKIE);
		strip_recursive_slashes($_SESSION);
	}

	/* We don't want PHP obscuring our data -- we just have to be careful */
	ini_set('magic_quotes_runtime',FALSE);

	/*******************************************************
	 * Begin other miscellaneous functions
	 *******************************************************/

	if (!function_exists("mail_is_valid"))
	{
		function mail_is_valid($email)
		{
			$matches = array();

			# Make sure the '@' exists...
			if (preg_match("/^.+@(.+)$/",$email,$matches))
			{
				# As well as a valid hostname
/*
 * NOTE: there is a potential bug here. "Misconfigured" (i.e.,
 * poorly configured) hosts can allow mail servers for domains
 * on which you cannot perform NS lookups, since domain and mx
 * records differ.
 */
 				if (newsys_conf("check_email_addr"))
				{
					if (PHP_OS == "Windows")
						return getHostByName($matches[1]) != $matches[1];
					else
						return getmxrr($matches[1],$addr);
				} else {
					return strpos($matches[1],".") !== FALSE;
				}
			} else {
				return 0;
			}
		}
	}

	if (!function_exists("unHTMLEntities"))
	{
		/* Concept borrowed from PHP.net */
		function unHTMLEntities($str)
		{
			$table = array_flip(get_html_translation_table(HTML_ENTITIES));

			return strtr($str,$table);
		}
	}

	/*******************************************************
	 * Begin newsys miscellaneous functions
	 *******************************************************/

	function newsys_crypt($data)
	{
#		static $crypt_key = newsys_conf('crypt_key');
		return crypt($data,newsys_conf("crypt_key"));
	}

	function newsys_t_key_len($type)
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

	function newsys_t_gen_key($type = "blowfish")
	{
		$key = "";

		if ($type == "blowfish" && !CRYPT_BLOWFISH)
			$type = "md5";

		if ($type == "md5" && !CRYPT_MD5)
			$type = "ext_des";

		if ($type == "ext_des" && !CRYPT_EXT_DES)
			$type = "default";

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
				newsys_handle_error("Invalid type to newsys_rand_str(); type: $type");
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

	function newsys_build_path($type = NEWSYS_PATH_REL)
	{
		$path = "";

		switch ($type)
		{
			case NEWSYS_PATH_REL: $path = newsys_conf("web_root") . newsys_conf("path"); break;
			case NEWSYS_PATH_SYS: $path = newsys_conf("sys_root") . newsys_conf("path"); break;
			case NEWSYS_PATH_ABS: $path = newsys_conf("site_uri") . newsys_conf("path"); break;
			default:
				newsys_handle_error("Invalid newsys_build_path() type; type: $type");
		}

		return $path;
	}

	function newsys_gen_class()
	{
		static $count = 1;

		return "newsysData" . (($count++ % 2) + 1);
	}

	function newsys_move_file($src,$dst)
	{
		global $php_errormsg;

		@copy($src,$dst)	|| newsys_handle_error("Cannot copy file; src: $src; dst: $dst");

		@unlink($src)		|| newsys_handle_error("Cannot unlink() file; file: $src");

		return;
	}

	function newsys_next_prev_menu($indices)
	{
		$of		= newsys_get_of();
		$offset		= (int)$indices["offset"];
		$total		= (int)$indices["total"];
		$url		= $indices["url"];
		$lim		= newsys_conf("result_limit");
		$page_lim	= newsys_conf("page_limit");
		$output		= "";

		if ($offset >= $lim)
		{
			$output .=	$of->link("First",     $url . "0")		. " | " .
					$of->link("Previous",  $url . ($offset - $lim))	. " | ";
		} else {
			$output .=	"First | Previous | ";
		}

		$current_index	= floor($offset	/ $lim);
		$last_index	= ceil($total	/ $lim);

		if ($current_index < $page_lim / 2)
		{
			/*
			 * The "left boundary" exists when we can't evenly distribute
			 * the "previous pages'" hyperlinks
			 */
			$start	= 0;

		} elseif ($current_index > $last_index - ceil($page_lim / 2)) {

			/*
			 * The "right boundary" exists when we can't evenly distribute
			 * the "next pages'" hyperlinks
			 */

			$start	= $last_index - $page_lim;
		} else {
			$start	= $current_index - floor($page_lim / 2);
		}

		for ($i = $start; $i < $start + $page_lim && $i < $last_index; $i++)
		{
			if ($i == $current_index)
			{
				$output .= $of->strong($i + 1) . " | ";
			} else {
				$output .= $of->link($i + 1,$url . ($i * $lim)) . " | ";
			}
		}

		if ($offset + $lim < $total)
		{
			$output .=	$of->link("Next",$url . ($offset + $lim)) . " | " .
					$of->link("Last",$url . ($total - $lim));
		} else {
			$output .=	"Next | Last";
		}

		return $output;
	}

	function newsys_get_dbh()
	{
		static $dbh = array();

		$dbh_type = newsys_conf("dbh_type");

		if (!array_key_exists($dbh_type,$dbh))
		{
			$class	= "DBH_$dbh_type";
			$file	= "DBH-$dbh_type/1.7.inc";

			require_once $file;

			$dbh[$dbh_type] =	new $class
						(
							newsys_conf("dbh_host"),
							newsys_conf("dbh_username"),
							newsys_conf("dbh_password"),
							newsys_conf("dbh_database")
						);
		}

		return $dbh[$dbh_type];
	}

	function newsys_get_of()
	{
		static $of = array();

		$of_type = newsys_conf("of_type");

		if (!array_key_exists($of_type,$of))
		{
			$class		= "OF_$of_type";
			$file		= "OF-$of_type/1.7.inc";

			require_once $file;

			$of[$of_type]	= new $class();
		}

		return $of[$of_type];
	}

	function newsys_handle_error($msg)
	{
		static $count = 0; # To monitor infinite recursion
		global $php_errormsg;

		$msg .=	"; Date: " . date("F d, Y h:i:s A") .
			"; File: " . $_SERVER["PHP_SELF"];

		if ($php_errormsg)
			$msg .= "; PHP Error: $php_errormsg";

		$msg .= "\n";

		if (newsys_conf("trigger_errors"))
			trigger_error($msg,E_USER_ERROR);

		# Try to log regardless
		if ($fp = @fopen(newsys_conf("error_log"),"a"))
		{
			fputs($fp,$msg);
			fclose($fp);
		}

		# Mail error report
		if (newsys_conf("mail_errors"))
			@mail
			(
				newsys_conf("site_email"),
				newsys_conf("site_name") . " Newsys Errors",
				$msg,
				"From: " . newsys_conf("site_email")
			);

		$of = newsys_get_of();

		if (newsys_conf("show_errors"))
		{
			if ($count++)
			{
				# Ok, no fancy stuff, just exit
				echo	"The following error occured while processing that directive: $msg";
			} else {
				echo	newsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("The following error occurred while processing that directive: $msg"),
					newsys_get_template("footer");
			}
		} else {
			if ($count++)
			{
				# Same here; nothing fancy
				echo	"An error ocurred while processing this directive. Please try again later.";
			} else {
				echo	newsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("An error occurred while processing this directive. Please try again later."),
					newsys_get_template("footer");
			}
		}

		exit(1);
	}

	function newsys_redirect($url)
	{
		# Clear buffer if there's stuff in it
		if (headers_sent())
			ob_end_clean();
/*
 * NOTE: $url here *should* be an absolute URI. Perhaps if it doesn't
 * contain an 'proto://...' we should give it one?
 */

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

	function newsys_mail($to,$from,$subject,$message)
	{
		global $php_errormsg;

		if (!@mail($to,$subject,$message,"From: $from"))
			newsys_handle_error("Cannot send mail via mail()");

		return;
	}

	/*
	 * This temporary function is just used to clean up attributes
	 * found by newsys_str_parse()
	 */
	function newsys_t_clean_attr($attr)
	{
		# Attributes will be matched with their delimiters (" or ')
		$attr = preg_replace("/^&quot;|^'|&quot;$|'$/","",$attr);

		# Attributes will have been subject to htmlEntities()
		$attr = unHTMLEntities($attr);

		return $attr;
	}

	/*
	 * This temporary function is just used to check potentially
	 * arbitrarily-protocoled URIs for disallowed protocols
	 */
	function newsys_t_check_proto($url)
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
								newsys_t_check_proto(newsys_t_clean_attr("$3"))

								# Else it is good
								: TRUE
							) ?

							# Formatted attribute
							" $2=\"" . newsys_t_clean_attr("$3") . "\""

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
				$tlds = "(?:" . join("|",newsys_conf("story_auto_url_tlds")) . ")";

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
 * HTML format. It should actually grab the output of $ns_of->link() and make
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

	/*******************************************************
	 * Begin newsys caching functions
	 *******************************************************/

	function newsys_cache_expire()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");

		if (!@touch("$cache_dir/info-$id"))
			newsys_handle_error("Cannot touch() file; file: $cache_dir/info-$id");

		return;
	}

	function newsys_cache_delete()
	{
		$args		= func_get_args();
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");

		/* This data might not be cached and thus might not exist */
		if (file_exists("$cache_dir/cache-$id"))
			newsys_unlink("$cache_dir/cache-$id");

		if (file_exists("$cache_dir/info-$id"))
			newsys_unlink("$cache_dir/info-$id");

		return;
	}

	function newsys_unlink($file)
	{
		global $php_errormsg;

		if (!@unlink($file))
			newsys_handle_error("Cannot unlink() file; file: $file");

		return;
	}

	function newsys_cache_add_info()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$data		= array_shift($args);
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");

		$fp = @fopen("$cache_dir/info-$id","a");

		if (!$fp)
			newsys_handle_error("Cannot open file; file: $cache_dir/info-$id");

		fputs($fp,$data . "\n");

		fclose($fp);

		return;
	}

	function newsys_cache_remove_info()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$data		= array_shift($args);
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");
		$temp_dir	= newsys_conf("temp_dir");

		/* For allowing removal of multiple pieces of data */
		if (!is_array($data))
			$data = array($data);

		/* Incoming data */
		$fp_in	= @fopen("$cache_dir/info-$id","r");

		if (!$fp_in)
			newsys_handle_error("Cannot open file; file=$cache_dir/info-$id");

		/* Temporary storage */
		$f_temp	= tempnam($temp_dir,"ns-info");

		if (!$f_temp)
			newsys_handle_error("Cannot create temporary filename with tempnam()");

		/* Outgoing data */
		$fp_out	= @fopen($f_temp,"w");

		if (!$fp_out)
			newsys_handle_error("Cannot open file; file: $f_temp");

		# Keep all data except our exclusion
		while ($line = fgets($fp_in,NEWSYS_BUF_SIZE))
			if (!in_array(chop($line),$data))
				fputs($fp_out,$line);

		fclose($fp_out);
		fclose($fp_in);

		newsys_move_file($f_temp,"$cache_dir/info-$id");

		return;
	}

	function newsys_cache_is_fresh()
	{
		static $cache		= array();

		static $cache_dir;
		static $templates_dir;
		$args			= func_get_args();
		$type			= array_shift($args);
		$id			= join('-',$args);

		if (!$cache_dir)
			$cache_dir	= newsys_conf("cache_dir");

		if (!$templates_dir)
			$templates_dir	= newsys_conf("templates_dir");

		# Before we do anything, make sure the file is cached
		if (!file_exists("$cache_dir/cache-$id"))
			return FALSE;

		# Grab the cache's last modification timestamp
		$mod_time	= @filemtime("$cache_dir/cache-$id");

		# Cache last mod time for templates
		if (!array_key_exists("mtime_struct_$type",$cache))
		{
			$cache["mtime_struct_$type"]	= filemtime("$templates_dir/$type.inc");
			$cache["mtime_content_$type"]	= filemtime("$templates_dir/$type.t");
			$cache["dep_$type"]		= @file("$templates_dir/$type.dep");

			$deps				= array();

			# Compile dependencies
			if (is_array($cache["dep_$type"]))
			{
				foreach ($cache["dep_$type"] as $dep)
				{
					# Skip comments/blank lines
					if (preg_match("/^\s*#|^\s*$/",$dep))
						continue;

					# Syntax: rule:arg
					preg_match("/^(\w+):(.*)$/",$dep,$matches);

					# Expand variables
					$matches[2] =	preg_replace
							(
								"/
									{?			# Optional { ... } delimiter
									(
										\\\$		# Dollar
										[a-zA-Z_]	# First char
										[a-zA-Z_0-9]+	# Subsequent chars
										(?:
											\[	# Subscript
											([\"'])	# Quotes
											.*?	# Index
											\\2	# Matching quote
											\]
										)*
									)
									}?
								/xe",
								"$1",
								$matches[2]
							);

					array_push($deps,array('type' => $matches[1],'arg' => $matches[2]));
				}
			}

			$cache["dep_$type"]		= $deps;
		}

		# Check if the templates have changed
		if
		(
			$cache["mtime_struct_$type"] > $mod_time
			||
			$cache["mtime_content_$type"] > $mod_time
		)
			return FALSE;

		# Make sure the dependencies haven't expired
		foreach ($cache["dep_$type"] as $dep)
		{
			switch ($dep["type"])
			{
				# Check last mod time
				case "file":
					if (@filemtime($dep['arg']) > $mod_time)
						return FALSE;

					break;
			}
		}

		# It must be fresh then...
		return TRUE;
	}

	if (!function_exists('file_dump'))
	{
		function file_dump($file,$data)
		{
			global $php_errormsg;

			$fp = @fopen($file,"w");

			if (!$fp)
				newsys_handle_error("Unable to write file; file: $file");

			fputs($fp,$data);

			fclose($fp);

			return $data;
		}
	}

	if (!function_exists('file_slurp'))
	{
		function file_slurp($file)
		{
			global $php_errormsg;

			$fp = @fopen($file,"r");

			if (!$fp)
				newsys_handle_error("Unable to read file; file: $file");

			$size = @filesize($file);

			$data = fread($fp,$size);

			fclose($fp);

			return $data;
		}
	}

	function newsys_get_template()
	{
		static $_use_cache;
		global $php_errormsg;

		if ($_use_cache == NULL)
			$_use_cache = newsys_conf("use_cache");

		$output		= "";
		$args		= func_get_args();
		$_type		= $args[0];
		$_id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");
		$templates_dir	= newsys_conf("templates_dir");

		$NO_CACHE	= FALSE;

		# If something has changed, rebuild it
		if ($_use_cache && newsys_cache_is_fresh($_id))
		{
			# Nothing has changed, used cached version
			$output = file_slurp("$cache_dir/cache-$_id");
		} else {
			# Load template-specific dependencies
			$ret	= include($templates_dir . "/$_type.inc");

			# Load structure and check for errors
			if (!$ret)
				return NULL;

			# Load template, substitute variables, save output
			ob_start();
			include("$templates_dir/$_type.t");
			$output	= ob_get_contents();
			ob_end_clean();

			# Substitute variables
			$output =	preg_replace
					(
						"/
							{?				# Optional { ... } delimiter
							(
								\\\$			# Dollar
								[a-zA-Z_]		# First char
								[a-zA-Z_0-9]+		# Subsequent chars
								(?:
									\[		# Subscript
									([\"'])		# Quotes
									.*?		# Index
									\\2		# Matching quote
									\]
								)*
							)
							}?
						/xe",
						"$1", # will cause a problem with $arr[\"index\"]
						$output
					);

			# Write over old cached version
			if (!$NO_CACHE)
				file_dump("$cache_dir/cache-$_id",$output);
		}

		return $output;
	}

	/*******************************************************
	 * Begin newsys XML control functions
	 *******************************************************/

	function newsys_xml_update($type,$data,$doc = NULL)
	{
		global $php_errormsg;

		$xml_files	= newsys_conf("xml_files");
		$xml_file	= $xml_files[$type];

		if (!$doc)
		{
			$doc = @domxml_open_file($xml_file);

			# Altering something that doesn't exist isn't supposed to happen
			if (!$doc)
				newsys_handle_error("XML file can't be loaded; file: $xml_file");
		}

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$obj_rdf = $doc->document_element();

				foreach ($obj_rdf->child_nodes() as $obj_item)
				{
					# We *should* only need to update the item,
					# since the URL *shouldn't* have changed
					if ($obj_item->node_name() == "item")
					{
						$attr = $obj_item->get_attribute('rdf:about');
// START FLAKY DOMXML HACK
if (!$attr)
	$attr = $obj_item->get_attribute("about");
// END FLAKY DOMXML HACK

						if (!preg_match("/\?story_id=(\d+)$/",$attr,$matches))
							newsys_handle_error("Can't find story ID from XML file during story update; file: $xml_file");

						if ($matches[1] == $data["story_id"])
						{
							foreach ($obj_item->child_nodes() as $child)
							{
								$children = $child->child_nodes();

								switch ($child->node_name())
								{
									#case "newsys:timestamp":
									case 'timestamp':
										$children = $child->child_nodes();
										$child->remove_child($children[0]); /* Should be text node */
										$child->append_child($doc->create_text_node($data["date"]));
										break;

									case "description":
										$children = $child->child_nodes();
										$child->remove_child($children[0]); /* Should be text node */
										$child->append_child($doc->create_text_node($data["overview"]));
										break;

									case "title":
										$children = $child->child_nodes();
										$child->remove_child($children[0]); /* Should be text node */
										$child->append_child($doc->create_text_node($data["headline"]));
										break;
								}
							}

							break;
						}
					}
				}

				break;
			}

			default:
			{
				newsys_handle_error("Unrecognized newsys XML type; type: $type");
			}
		}

		return $doc;
	}

	function newsys_xml_write($type,$doc)
	{
		global $php_errormsg;

		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$xml_file	= $xml_files[$type];

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$data		= $doc->dump_mem(TRUE);

		file_dump($xml_file,$data);

		return;
	}

	function newsys_xml_add($type,$data,$doc = NULL)
	{
		global $php_errormsg;

		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$xml_file	= $xml_files[$type];

		if (!$doc)
		{
			$doc = @domxml_open_file($xml_file);

			# Create a new document if no such one exists
			if (!$doc)
				$doc = newsys_xml_setup($type);
		}

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$link		= newsys_build_path(NEWSYS_PATH_ABS) . "/view.php?story_id=" . $data["story_id"];

				$obj_rdf	= $doc->document_element();

				$obj_item	= $doc->create_element("item");
				$obj_item->set_attribute("rdf:about",	$link);

				$obj_title	= $doc->create_element("title");
				$obj_link	= $doc->create_element("link");
				$obj_ns_ts	= $doc->create_element("newsys:timestamp");
				$obj_desc	= $doc->create_element("description");

				$obj_title->append_child($doc->create_text_node($data['headline']));
				$obj_link->append_child($doc->create_text_node($link));
				$obj_ns_ts->append_child($doc->create_text_node($data['date']));
				$obj_desc->append_child($doc->create_text_node($data['overview']));

				$obj_item->append_child($obj_title);
				$obj_item->append_child($obj_link);
				$obj_item->append_child($obj_ns_ts);
				$obj_item->append_child($obj_desc);

				$obj_rdf->append_child($obj_item);

				# Remove the old stories
				$children		= $obj_rdf->child_nodes();
				$oldest_date		= date("YmdHis");	# Every story should be before now
				$num_stories		= 0;
				$removed_stories	= array();

				# Count them up
				foreach ($children as $obj_item)
					if ($obj_item->node_name() == "item")
						$num_stories++;

				while ($num_stories > newsys_conf("result_limit"))
				{
					$oldest_story = NULL;

					# Then find the oldest and remove it
					foreach ($children as $obj_item)
					{
						if ($obj_item->node_name() == "item")
						{
							# Check if this is the oldest story
							foreach ($obj_item->child_nodes() as $child)
							{
/*
								$grand_children = $child->child_nodes();

								...
									# Should be text node
									$grand_children[0]->node_value() < $oldest_date
*/
								if
								(
									/* DOMXML is a bit flaky*/
									(/*$child->node_name() == "timestamp" ||*/
									$child->node_name() == "newsys:timestamp") &&
									$child->get_content() < $oldest_date
								)
								{
									$oldest_story	= $obj_item;

//									$oldest_date	= $grand_children[0]->node_value();

									$oldest_date	= $child->get_content();

									$attr		= $obj_item->get_attribute("rdf:about");

// FLAKY DOMXML FIX
if (!$attr)
	$attr = $obj_item->get_attribute("about");
// END FLAKY DOMXML FIX
									if (!preg_match("/\?story_id=(\d+)$/",$attr,$matches))
										newsys_handle_error("Can't find oldest story ID from XML file during story add; file: $xml_file");

									array_push($removed_stories,$matches[1]);

									break;
								}
							}
						}
					}

					if (!$oldest_story)
						newsys_handle_error("Couldn't find oldest story from XML file during story add; file: $xml_file");

					$obj_rdf->remove_child($oldest_story);

					$num_stories--;
				}

				$obj_rdf_seq = NULL;

				# Now add our entry to the <channel> element, and remove the others
				foreach ($children as $channel)
				{
					if ($channel->node_name() == "channel")
					{
						# Find the <items> elements
						foreach ($channel->child_nodes() as $obj_items)
						{
							if ($obj_items->node_name() == "items")
							{
								foreach ($obj_items->child_nodes() as $obj_rdf_seq)
								{
									if ($obj_rdf_seq->node_name() == "rdf:Seq" /*||
									    $obj_rdf_seq->node_name() == "Seq"*/) /* Flake DOMXML fix */
									{
										if ($obj_rdf_seq->has_child_nodes())
										{
											$seq_children = $obj_rdf_seq->child_nodes();

											foreach ($seq_children as $obj_rdf_li)
											{
												if (!preg_match("/\?story_id=(\d+)$/",$obj_rdf_li->get_attribute("resource"),$matches))
													newsys_handle_error("Could not find oldest story ID from XML file during story add; file: $xml_file");

												$id = $matches[1];

												if (in_array($id,$removed_stories))
													$obj_rdf_seq->remove_child($obj_rdf_li);
											}
										}

										break;
									}
								}

								break;
							}
						}

						break;
					}
				}

				if (!$obj_rdf_seq)
					newsys_handle_error("Could not find oldest story from XML file during story add; file: $xml_file");

				$obj_rdf_li = $doc->create_element("rdf:li");
				$obj_rdf_li->set_attribute("resource",$link);
				$obj_rdf_seq->append_child($obj_rdf_li);

				break;
			}

			default:
			{
				newsys_handle_error("Unrecognized newsys XML type; type=$type");
			}
		}

		return $doc;
	}

	function newsys_xml_remove($type,$data,$doc = NULL)
	{
		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$xml_file	= $xml_files[$type];

		if (!$doc)
		{
			$doc = @domxml_open_file($xml_file);

			if (!$doc)
				newsys_handle_error("XML file can't be loaded; file: $xml_file");
		}

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$obj_rdf = $doc->document_element();

				foreach ($obj_rdf->child_nodes() as $child)
				{
					if ($child->node_name() == "channel")
					{
						foreach ($child->child_nodes() as $obj_items)
						{
							if ($obj_items->node_name() == "items")
							{
								foreach ($obj_items->child_nodes() as $obj_rdf_seq)
								{
									if ($obj_rdf_seq->node_name() == "rdf:Seq" && is_array($obj_rdf_seq->child_nodes()))
									{
										foreach ($obj_rdf_seq->child_nodes() as $obj_rdf_li)
										{
											if (!preg_match("/\?story_id=(\d+)$/",$obj_rdf_li->get_attribute("resource"),$matches))
												newsys_handle_error("Can't find story ID from XML file during story removal; file: $xml_file");

											if ($matches[1] == $data["story_id"])
											{
												$obj_rdf_seq->remove_child($obj_rdf_li);

												break;
											}
										}

										break;
									}
								}

								break;
							}
						}
					} elseif ($child->node_name() == "item") {

						$attr = $child->get_attribute("rdf:about");

// START FLAKY DOMXML HACK
if (!$attr)
	$attr = $child->get_attribute("about"); // no namespace
// END FLAKY DOMXML HACK

						if (!preg_match("/\?story_id=(\d+)$/",$attr,$matches))
							newsys_handle_error("Can't find story ID from XML file during story removal; file: $xml_file");

						if ($matches[1] == $data["story_id"])
							$obj_rdf->remove_child($child);
					}
				}

				break;
			}

			default:
			{
				newsys_handle_error("Unknown XML type; type: $type");
			}
		}

		return $doc;
	}

	function newsys_xml_setup($type)
	{
		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$doc = domxml_new_doc("1.0");

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$obj_rdf	= $doc->create_element("rdf:RDF");
				$obj_rdf->set_attribute("xmlns:rdf",		"http://www.w3.org/1999/02/22-rdf-syntax-ns#");
				$obj_rdf->set_attribute("xmlns",		"http://purl.org/rss/1.0/");
				$obj_rdf->set_attribute("xmlns:newsys",		"http://www.easyphp.net/projects/newsys/pub/newsys-ns/newsys-1.0.rdf");

				$obj_channel	= $doc->create_element("channel");
				$obj_channel->set_attribute("rdf:about",	"http://www.xml.com/xml/news.rss");

				$obj_title	= $doc->create_element("title");
				$obj_title->append_child($doc->create_text_node(newsys_conf("site_name")));

				$obj_link	= $doc->create_element("link");
				$obj_link->append_child($doc->create_text_node(newsys_conf("site_uri") . "/"));

				$obj_desc	= $doc->create_element("description");
				$obj_desc->append_child($doc->create_text_node(newsys_conf("site_desc")));

				$obj_image	= $doc->create_element("image");
				$obj_image->set_attribute("rdf:resource",newsys_conf("rss_icon"));

				$obj_items	= $doc->create_element("items");

				$obj_rdf_seq	= $doc->create_element("rdf:Seq");

				$obj_items->append_child($obj_rdf_seq);

				$obj_channel->append_child($obj_title);
				$obj_channel->append_child($obj_link);
				$obj_channel->append_child($obj_desc);
				$obj_channel->append_child($obj_image);
				$obj_channel->append_child($obj_items);

				$obj_rdf->append_child($obj_channel);

				$doc->append_child($obj_rdf);

				break;
			}

			default:
			{
				newsys_handle_error("Unrecognized XML type; type: $type");
			}
		}

		return $doc;
	}

	/*******************************************************
	 * Begin newsys story functions
	 *******************************************************/

	function newsys_rebuild_latest_stories()
	{
		global $php_errormsg;

		$dbh	= newsys_get_dbh();

		/* Clear out old stories */
		$dbh->query("	DELETE FROM
					newsys_last_stories",DB_NULL);

		$dbh->query("	SELECT
					author_id,
					story_id,
					create_date
				FROM
					newsys_stories
				ORDER BY
					create_date DESC
				LIMIT
					" . newsys_conf("result_limit"),DB_ROWS);

		$stories = array();

		/* Refresh listing */
		while ($story = $dbh->fetch_row())
			array_push($stories,$story);

		foreach ($stories as $story)
			$dbh->query("	INSERT INTO
						newsys_last_stories
					(
						author_id,
						story_id,
						create_date
					) VALUES (
						{$story["author_id"]},
						{$story["story_id"]},
						{$story["create_date"]}
					)",DB_NULL);

		/* Update cache */
		if (newsys_conf("use_cache"))
		{
/*
 * Should we re-compute the latest stories in the database
 * as well here to eliminate any possible discrepancies?
 */

			$fp	= @fopen(newsys_conf("cache_dir") . "/info-last_stories","w");

			if (!$fp)
				newsys_handle_error("Could not open latest stories cache file");

			/* This format expects backwards data */
			foreach (array_reverse($stories) as $story)
				fputs($fp,$story["story_id"] . "\n");

			fclose($fp);
		}

		return;
	}

	function newsys_get_latest_stories()
	{
		$ids = array();

		if (newsys_conf("use_cache"))
		{
			/* See if we can use the cached version */
			$ids = @file(newsys_conf("cache_dir") . "/info-last_stories");

			if (!is_array($ids))
				$ids = array();

			/* Remove newlines from IDs */
			foreach ($ids as $key => $value)
				$ids[$key] = chop($value);

			/* Stories should be reversed */
			$ids = array_reverse($ids);
		} else {
			/* Fallback to the database */
			$dbh = newsys_get_dbh();

			$dbh->query("	SELECT
						story_id
					FROM
						newsys_stories
					ORDER BY
						create_date DESC
					LIMIT
						/* This next part is just in case */
						" . newsys_conf("result_limit"),DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);
		}

		return $ids;
	}

	function newsys_story_get($dbh,$story_id)
	{
		$story_id	= (int)$story_id;
		$story		= $dbh->query("	SELECT
							*
						FROM
							newsys_stories
						WHERE
							story_id = $story_id",DB_ROW);

		return $story;
	}

	function newsys_story_get_comments($story_id)
	{
		$ids		= array();
		$story_id	= (int)$story_id;

		if (newsys_conf("use_cache"))
		{
			/* See if we can use the cached version */
			$ids = @file(newsys_conf("cache_dir") . "/info-story-$story_id");

			if (!is_array($ids))
				$ids = array();

			/* Remove newlines */
			foreach ($ids as $key => $val)
				$ids[$key] = chop($val);

			/* Comments should be in reverse-order */
			$ids = array_reverse($ids);
		} else {
			/* Fallback to the database */
			$dbh = newsys_get_dbh();

			$dbh->query("	SELECT
						comment_id
					FROM
						newsys_comments
					WHERE
						story_id		= $story_id AND
						parent_comment_id	= 0",DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);
		}

		return $ids;
	}

	function newsys_story_update($dbh,$story)
	{
		$err = E_NS_NONE;

		/* Validate story fields */
		if (!$story["overview"])
		{
			/*
			 * Default actions:
			 *      1) Look for newsys_overview_break
			 *      2) Take the first 255 chars/break
			 */

			$t = preg_quote(newsys_conf("overview_break"),"/");

			if (preg_match("/^(.*?)\s*$t/",$story["story"],$matches))
			{
				/* User-defined overview break */
				$story["overview"] = $matches[1];

				/* Remove overview break from regular story */
				$story["story"] = preg_replace("/\s*$t/","",$story["story"]);
			} else {
				/*
				 * Else break it ourself after the word
				 * closest to the 255 char mark
				 */
				$story["overview"] = preg_replace("/<.*?>/","",$story["story"]);

				$broken = FALSE;

				while (strlen($story["overview"]) > 252)
				{
					$broken = TRUE;
					$story["overview"] = preg_replace("/\s*\S+\s*$/","",$story["overview"]);
				}

				if ($broken)
					$story["overview"] .= "...";
			}
		}

		if (!$story["headline"])	$err |= E_NS_STORY_HEADLINE;
		if (!$story["story"])		$err |= E_NS_STORY_STORY;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_STORY,$story);

		if ($err != E_NS_NONE || count($user_errors))
			return array($err,$user_errors);

		$story["story_id"]	= (int)$story["story_id"];
		$story["overview"]	= $dbh->prepare_str(newsys_str_parse($story["overview"],NEWSYS_STR_NONE),SQL_REG);
		$story["story"]		= $dbh->prepare_str(newsys_str_parse($story["story"]),SQL_REG);
		$story["headline"]	= $dbh->prepare_str(newsys_str_parse($story["headline"],NEWSYS_STR_NONE),SQL_REG);

		$sql = "	UPDATE
					newsys_stories
				SET
					overview	= '{$story["overview"]}',
					story		= '{$story["story"]}',
					mod_date	= NOW(),";

		foreach (newsys_conf("story_fields") as $user_field_id => $user_field)
		{
			$story[$user_field_id] = $dbh->prepare_str(newsys_str_parse($story[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql .= "	$user_field_id	= '{$story[$user_field_id]}',";
		}

		$sql .= "
					headline	= '{$story["headline"]}'
				WHERE
					story_id	= {$story["story_id"]}";

		/* Update database */
		$dbh->query($sql,DB_NULL);

		$is_latest = $dbh->query("	SELECT
							author_id
						FROM
							newsys_last_stories
						WHERE
							story_id = {$story["story_id"]}",DB_COL);


		/* Update cache */
		if (newsys_conf("use_cache"))
			newsys_cache_expire("story",$story["story_id"]);

		# Update RSS info
		if ($is_latest && newsys_conf("use_rss"))
		{
/*
			$ts = new TimeStamp();
			$ts->load_current();
			$story['date'] = $ts->get_unix();

*/
			$story['date'] = time();

			newsys_xml_write
			(
				NEWSYS_XML_STORIES,
				newsys_xml_update
				(
					NEWSYS_XML_STORIES,
					$story
				)
			);
		}

		return array(E_NS_NONE,array());
	}

	function newsys_story_delete($dbh,$story_id)
	{
		$story_id	= (int)$story_id;

		$use_cache	= newsys_conf("use_cache");

		$dbh->query("	DELETE FROM
					newsys_stories
				WHERE
					story_id	= $story_id",DB_NULL);

		if ($use_cache)
		{
			/*
			 * Should we loop through all the templates and
			 * check dependencies for this story and delete
			 * all cached templates?
			 *
			 * Also -- should we update other resources this
			 * affects, such as info-user so the profile gets
			 * updated for things such as 'last story' and
			 * 'num posts'
			 */

			newsys_cache_delete("story",$story_id);

			# Gather up the comments
			$dbh->query("	SELECT
						comment_id
					FROM
						newsys_comments
					WHERE
						story_id = $story_id",DB_ROWS);

			# And remove them
			while (list ($comment_id) = $dbh->fetch_row())
				newsys_cache_delete("comment",$story_id,$comment_id);
		}

		$dbh->query("	DELETE FROM
					newsys_comments
				WHERE
					story_id	= $story_id",DB_NULL);

		$is_latest = $dbh->query("	DELETE FROM
							newsys_last_stories
						WHERE
							story_id	= $story_id",DB_NULL);

		if ($use_cache && $is_latest)
			newsys_rebuild_latest_stories();

		# Update RSS info
		if ($is_latest && newsys_conf("use_rss"))
		{
			newsys_xml_write
			(
				NEWSYS_XML_STORIES,
				newsys_xml_remove
				(
					NEWSYS_XML_STORIES,
					array
					(
						'story_id' => $story_id
					)
				)
			);
		}

		return;
	}

	function newsys_story_add($dbh,$story)
	{
		$err = E_NS_NONE;

		/* Validate story fields */
		if (!$story["overview"])
		{
			/*
			 * Default actions:
			 *	1) Look for newsys_overview_break
			 *	2) Take the first 255 chars/break
			 */

			$t = preg_quote(newsys_conf("overview_break"),"/");

			if (preg_match("/^(.*?)\s*$t/",$story["story"],$matches))
			{
				/* User-defined overview break */
				$story["overview"] = $matches[1];

				/* Remove overview break from regular story */
				$story["story"] = preg_replace("/\s*$t/","",$story["story"]);
			} else {
				/*
				 * Else break it ourself after the word
				 * closest to the 255 char mark
				 */
				$story["overview"] = preg_replace("/<.*?>/","",$story["story"]);

				$broken = FALSE;

				while (strlen($story["overview"]) > 252)
				{
					$broken = TRUE;
					$story["overview"] = preg_replace("/\s*\S+\s*$/","",$story["overview"]);
				}

				if ($broken)
					$story["overview"] .= "...";
			}
		}

		if (!$story["headline"])	$err |= E_NS_STORY_HEADLINE;
		if (!$story["story"])		$err |= E_NS_STORY_STORY;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_STORY,$story);

		if ($err != E_NS_NONE || count($user_errors))
			return array($err,$user_errors);

		$story["author_id"]	= (int)$story["author_id"];
		$story["overview"]	= $dbh->prepare_str(newsys_str_parse($story["overview"],NEWSYS_STR_NONE),SQL_REG);
		$story["story"]		= $dbh->prepare_str(newsys_str_parse($story["story"]),SQL_REG);
		$story["headline"]	= $dbh->prepare_str(newsys_str_parse($story["headline"],NEWSYS_STR_NONE),SQL_REG);

		$sql_fields		= "";
		$sql_values		= "";

		foreach (newsys_conf("story_fields") as $user_field_id => $user_field)
		{
			$story[$user_field_id] = $dbh->prepare_str(newsys_str_parse($story[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql_fields .= ",$user_field_id";

			$sql_values .= ",'{$story[$user_field_id]}'";
		}

		/* Update database */
		$dbh->query("	INSERT INTO
					newsys_stories
				(
					author_id,
					overview,
					headline,
					story,
					create_date,
					allow_comments
					$sql_fields
				) VALUES (
					{$story["author_id"]},
					'{$story["overview"]}',
					'{$story["headline"]}',
					'{$story["story"]}',
					NOW(),
					" . newsys_conf("allow_comments") . " /* Default value to allow/disallow */
					$sql_values
				)",DB_NULL);

		$story_id	= $dbh->last_insert_id();

		/* Gather oldest story to remove it */
		$old_story_id	= $dbh->query("	SELECT
							story_id
						FROM
							newsys_last_stories
						ORDER BY
							create_date ASC
						LIMIT
							1",DB_COL);

		/* Add new story to latest stories */
		$dbh->query("	INSERT INTO
					newsys_last_stories
				(
					story_id,
					author_id,
					create_date
				) VALUES (
					$story_id,
					{$story["author_id"]},
					NOW()
				)",DB_NULL);

		/* Check if we're going to force a displacement */
		$total = $dbh->query("	SELECT
						COUNT(*)
					FROM
						newsys_stories",DB_NULL);

		/* Remove oldest story */
		$result_limit = newsys_conf("result_limit");

		if ($old_story_id && $total > $result_limit)
			$dbh->query("	DELETE FROM
						newsys_last_stories
					WHERE
						story_id = $old_story_id",DB_NULL);

		/* Update cache */
		if (newsys_conf("use_cache"))
		{
			newsys_cache_add_info($story_id,"last_stories");


#echo $dbh->query("SELECT COUNT(*) FROM newsys_stories",DB_COL);

			if ($old_story_id && $total > $result_limit)
				newsys_cache_remove_info($old_story_id,"last_stories");
#echo $dbh->query("SELECT COUNT(*) FROM newsys_stories",DB_COL);

		}

#echo $dbh->query("SELECT COUNT(*) FROM newsys_stories",DB_COL);

		/* Update XML */
		if (newsys_conf("use_rss"))
		{
			$new_story = $story;

			/*
			 * Don't modify the original story -- not that it would really matter
			 *
			 * Note: we have to modify it to add the not-present 'story id' and
			 * 'timestamp' fields
			 */
			$new_story['story_id']	= $story_id;
/*
			$ts			= new TimeStamp();
			$ts->load_current();
			$new_story['date']	= $ts->get_unix();

*/
			$new_story['date']	= time();

			newsys_xml_write
			(
				NEWSYS_XML_STORIES,
				newsys_xml_add
				(
					NEWSYS_XML_STORIES,
					$new_story
				)
			);
		}

		return array(E_NS_NONE,array());
	}

	/*******************************************************
	 * Begin newsys comment functions
	 *******************************************************/

	function newsys_comment_add($dbh,$comment)
	{
		$comment["story_id"]	= (int)$comment["story_id"];

		# Validate story identifier
		if
		(
			!$dbh->query("	SELECT
						story_id
					FROM
						newsys_stories
					WHERE
						story_id = {$comment["story_id"]}",DB_COL)
		)
			return array(E_NS_COM_NO_STORY,array());

		$story		= newsys_story_get($dbh,$comment['story_id']);
		$max_comments	= newsys_conf('max_comments');

		/* Validate comment */
		$err = E_NS_NONE;

		if (!$comment["comment"])					$err |= E_NS_COM_COM;
		if (!$comment["subject"])					$err |= E_NS_COM_SUBJECT;
		if (!newsys_conf("allow_comments"))				$err |= E_NS_COM_NOT_ALLOWED;
		if ($max_comments && $story['num_comments'] >= $max_comments)	$err |= E_NS_COM_MAX_EXCEED;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_COMMENT,$comment);

		if ($err != E_NS_NONE || count($user_errors))
			return array($err,$user_errors);

	#	$comment["story_id"]	= (int)$comment["story_id"];
		$comment["author_id"]	= (int)$comment["author_id"];
		$comment["comment"]	= $dbh->prepare_str(newsys_str_parse($comment["comment"]),SQL_REG);

		# Grab unique comment identifier
		$comment["comment_id"]	= 1 + $dbh->query("	SELECT
									comment_id
								FROM
									newsys_comments
								WHERE
									story_id = ${comment["story_id"]}
								ORDER BY
									comment_id	DESC
								LIMIT
									1",DB_COL);

		$sql_fields		= "";
		$sql_values		= "";

		foreach (newsys_conf("comment_fields") as $user_field_id => $user_field)
		{
			$comment[$user_field_id] = $dbh->prepare_str(newsys_str_parse($comment[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql_fields .= ",$user_field_id";

			$sql_values .= ",'{$comment[$user_field_id]}'";
		}

		/* Update database */
		$dbh->query("	INSERT INTO newsys_comments
				(
					story_id,
					comment_id,
					author_id,
					subject,
					comment,
					create_date,
					parent_comment_id
					$sql_fields
				) VALUES (
					{$comment["story_id"]},
					{$comment["comment_id"]},
					{$comment["author_id"]},
					'{$comment["subject"]}',
					'{$comment["comment"]}',
					NOW(),
					{$comment["parent_comment_id"]}
					$sql_values
				)",DB_NULL);

		$dbh->query("	UPDATE
					newsys_stories
				SET
					num_comments	= num_comments + 1
				WHERE
					story_id	= {$comment["story_id"]}",DB_NULL);

		/* Update cache */
		if (newsys_conf("use_cache"))
		{
			/*
			 * If the parent is a comment, link to it, otherwise
			 * it must be a direct comment to a story, the case
			 * in which it must be linked to the story
			 */
			if (newsys_conf("hier_comments") && $comment['parent_comment_id'])
				newsys_cache_add_info($comment["comment_id"],"comment",$comment["story_id"],$comment["parent_comment_id"]);
			else
				newsys_cache_add_info($comment["comment_id"],"story",$comment["story_id"]);

		}

		return array(E_NS_NONE,array());
	}

	function newsys_comment_get_ancestors($needle,$haystack)
	{
		$ancestors	= array($needle);
		$continue	= 1;

		while ($continue)
		{
			$continue = 0;

			foreach ($haystack as $parent => $children)
				foreach ($children as $child)
					if ($child == $needle)
					{
						array_push($ancestors,$parent);

						// Try again for the parent
						$needle	= $parent;

						$continue	= 1;

						break 2;
					}
		}

		return $ancestors;
	}

	function newsys_comment_delete($dbh,$story_id,$comment_id)
	{
		$story_id	= (int)$story_id;
		$comment_id	= (int)$comment_id;

		# Validate comment
		$comment	= newsys_comment_get($dbh,$story_id,$comment_id);

		if (!is_array($comment))
			return;

		/* Grab all comments */
		$dbh->query("	SELECT
					comment_id,
					parent_comment_id
				FROM
					newsys_comments
				WHERE
					story_id = $story_id",DB_ROWS);

		$comments	= array();
		$all_comments	= array();

		/* Create a list of parent and child comments */
		while (list ($i_comment_id,$i_parent_comment_id) = $dbh->fetch_row())
			if (array_key_exists($i_parent_comment_id,$all_comments))
				array_push($all_comments[$i_parent_comment_id],$i_comment_id);
			else
				$all_comments[$i_parent_comment_id] = array($i_comment_id);

		/* Single out child comments to the current comment */
		foreach ($all_comments as $parent => $children)
			foreach ($children as $child)
				if (in_array($comment_id,newsys_comment_get_ancestors($child,$all_comments)))
					// Is a child of the target comment
					array_push($comments,$child);

#echo "comments to remove: ";print_r($comments);

		/* Remove comments */
		$dbh->query("	DELETE FROM
					newsys_comments
				WHERE
					story_id	= $story_id AND
					comment_id	IN(" . join(",",$comments) . ")",DB_NULL);

		/* Update num_comments in story */
		$dbh->query("	UPDATE
					newsys_stories
				SET
					num_comments = num_comments - " . count($comments) . "
				WHERE
					story_id = $story_id",DB_NULL);

		/* Clear out cache*/
		if (newsys_conf("use_cache"))
		{
			foreach ($comments as $i_id)
				newsys_cache_delete("comment",$story_id,$i_id);

			newsys_cache_remove_info($comments,"story",$story_id);

			if (newsys_conf("hier_comments") && $comment['parent_comment_id'])
				newsys_cache_remove_info($comment_id,"comment",$story_id,$comment['parent_comment_id']);
		}

		return;
	}

	function newsys_comment_update($dbh,$comment)
	{
/*
 * Should we check $story_id here for a valid story in the database?
 * Worst case: it doesn't exist, the 'UPDATE' query doesn't affect
 * anything. It all comes down to 'SELECT' vs. 'UPDATE,' except in the case
 * where both queries will be made.
 */
		/* Validate comment fields */
		$err = E_NS_NONE;

		if (!$comment["comment"]) $err |= E_NS_COM_COM;
		if (!$comment["subject"]) $err |= E_NS_COM_SUBJECT;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_COMMENT,$comment);

		if ($err != E_NS_NONE || count($user_errors))
			return array($err,$user_errors);

		$comment["story_id"]	= (int)$comment["story_id"];
		$comment["comment_id"]	= (int)$comment["comment_id"];
		$comment["comment"]	= $dbh->prepare_str(newsys_str_parse($comment["comment"]),SQL_REG);

		/* Update database */
		$sql = "	UPDATE
					newsys_comments
				SET
					mod_date	= NOW(),";

		foreach (newsys_conf("comment_fields") as $user_field_id => $user_field)
		{
			$comment[$user_field_id] = $dbh->prepare_str(newsys_str_parse($comment[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql .= "	$user_field_id	= '{$comment[$user_field_id]}',";
		}

		$sql .= "
					comment		= '{$comment["comment"]}'
				WHERE
					story_id	= {$comment["story_id"]}	AND
					comment_id	= {$comment["comment_id"]}";

		/* Update database */
		$dbh->query($sql,DB_NULL);

		/* Update cache */
		if (newsys_conf("use_cache"))
			newsys_cache_expire("comment",$comment["story_id"],$comment["comment_id"]);

		return array(E_NS_NONE,array());
	}

	function newsys_comment_get($dbh,$story_id,$comment_id)
	{
		$story_id	= (int)$story_id;
		$comment_id	= (int)$comment_id;

		$comment	= $dbh->query("	SELECT
							*
						FROM
							newsys_comments
						WHERE
							story_id	= $story_id AND
							comment_id	= $comment_id",DB_ROW);

		return $comment;
	}

	function newsys_get_comment_comments($story_id,$comment_id)
	{
		$story_id	= (int)$story_id;
		$comment_id	= (int)$comment_id;
		$ids		= array();

		if (newsys_conf("use_cache"))
		{
			/* See if we can use the cached version */
			$ids = @file(newsys_conf("cache_dir") . "/info-comment-$story_id-$comment_id");

			if (!is_array($ids))
				$ids = array();

			/* Strip newlines */
			foreach ($ids as $key => $value)
				$ids[$key] = $value;

			/* IDs should be in reverse order */
			$ids = array_reverse($ids);
		} else {
			/* Fall back to the database */
			$dbh = newsys_get_dbh();

			$dbh->query("	SELECT
						comment_id
					FROM
						newsys_comments
					WHERE
						story_id		= $story_id AND
						parent_comment_id	= $comment_id
					ORDER BY
						create_date DESC",DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);
		}

		return $ids;
	}

	/*******************************************************
	 * Begin newsys user functions
	 *******************************************************/

	function newsys_user_add($dbh,$user)
	{
		$err		= E_NS_NONE;

		/* Validate incoming user information */
		if (!preg_match("/^[a-zA-Z0-9_-]{5,30}$/",$user["username"]))	$err |= E_NS_USER_NAME;
		if (!preg_match("/^[^'\"]{5,30}$/",$user["password"]))		$err |= E_NS_USER_PASS;
		if (!mail_is_valid($user["email"]))				$err |= E_NS_USER_EMAIL;

		$t = $dbh->prepare_str($user["username"],SQL_REG);

		if
		(
			$dbh->query("	SELECT
						username
					FROM
						newsys_users
					WHERE
						username = '$t'",DB_COL)
		)
										$err |= E_NS_USER_NAME_USE;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_USER,$user);

		if ($err != E_NS_NONE || count($user_errors))
			return array($err,$user_errors);

		$user["type"]		= (int)$user["type"];
		$user["email"]		= $dbh->prepare_str(newsys_str_parse($user["email"],NEWSYS_STR_NONE),SQL_REG);
		$user["password"]	= $dbh->prepare_str(newsys_crypt($user["password"]),SQL_REG);

		$sql_fields		= "";
		$sql_values		= "";

		foreach (newsys_conf("user_fields") as $user_field_id => $user_field)
		{
			$user[$user_field_id] = $dbh->prepare_str(newsys_str_parse($user[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql_fields .= ",$user_field_id";

			$sql_values .= ",'{$user[$user_field_id]}'";
		}

		$dbh->query
		(
			"INSERT INTO newsys_users
			(
				username,
				password,
				email,
				type
				$sql_fields
			) VALUES (
				'{$user["username"]}',
				'{$user["password"]}',
				'{$user["email"]}',
				'{$user["type"]}'
				$sql_values
			)",
			DB_NULL
		);

		if (newsys_conf("use_cache"))
		{
			$user_id = $dbh->last_insert_id();

			/* Update cache */
			newsys_cache_expire("user",$user_id);
		}

		return array(E_NS_NONE,array());
	}

	function newsys_user_update($dbh,$user)
	{
		$err	= E_NS_NONE;

/*
 * If no new password is specified, don't change it.
 * The password field is special because we don't want
 * to print the value out as the 'default' value while
 * a user updates his info
 */
#		if (!preg_match("/^[a-zA-Z0-9_-]{5,30}$/",$user["username"]))	$err |= E_NS_USER_NAME;
		if ($user["password"])
			if (!preg_match("/^[^'\"]{5,30}$/",$user["password"]))	$err |= E_NS_USER_PASS;
		if (!mail_is_valid($user["email"]))				$err |= E_NS_USER_EMAIL;

		/* Array containing ids for each field with invalid input */
		$user_errors = newsys_fields_validate(NEWSYS_FLD_USER,$user);

		if ($err != E_NS_NONE || count($user_errors))
			return array($err,$user_errors);

		$user["type"]	= (int)$user["type"];
		$user["email"]  = $dbh->prepare_str(newsys_str_parse($user["email"],NEWSYS_STR_NONE),SQL_REG);

		if ($user["password"])
			$user["password"] = $dbh->prepare_str(newsys_crypt($user["password"]),SQL_REG);

		$sql = "	UPDATE
					newsys_users
				SET
";

#					username	= '{$user["username"]}',

		/* Only update the password if it has been specified to change */
		if ($user["password"])
			$sql .= "
					password	= '{$user["password"]}',";

		foreach (newsys_conf("user_fields") as $user_field_id => $user_field)
		{
			$user[$user_field_id] = $dbh->prepare_str(newsys_str_parse($user[$user_field_id],$user_field["parse"]),SQL_REG);

			$sql .= "	$user_field_id	= '{$user[$user_field_id]}',";
		}

		$sql .= "
					email		= '{$user["email"]}',
					type		= '{$user["type"]}'
				WHERE
					user_id		= '{$user["user_id"]}'";

		$dbh->query($sql,DB_NULL);

		/* Update cache */
		if (newsys_conf("use_cache"))
			newsys_cache_expire("user",$user["user_id"]);

		return array(E_NS_NONE,array());
	}

	function newsys_user_get($dbh,$user_id)
	{
		$user_id	= (int)$user_id;

		return $dbh->query("	SELECT
						*
					FROM
						newsys_users
					WHERE
						user_id = $user_id",DB_ROW);
	}

	function newsys_user_remove($dbh,$user_id)
	{
		$user_id	= (int)$user_id;

		/* Update cache */
		if (newsys_conf("use_cache"))
		{
			# Delete all comments by user
			$dbh->query("	SELECT
						story_id,
						comment_id
					FROM
						newsys_comments
					WHERE
						author_id = $user_id",DB_ROWS);

			while (list ($story_id,$comment_id) = $dbh->fetch_row())
				newsys_cache_delete("comment",$story_id,$comment_id);

			# Delete all stories by user
			$dbh->query("	SELECT
						story_id
					FROM
						newsys_stories
					WHERE
						author_id = $user_id",DB_ROWS);

			while (list ($id) = $dbh->fetch_row())
				newsys_cache_delete("story",$id);

			$dbh->query("	SELECT
						story_id
					FROM
						newsys_last_stories
					WHERE
						author_id = $user_id",DB_ROWS);

			$ids = array();

			while (list ($id) = $dbh->fetch_row())
				array_push($ids,$id);

			if (count($ids))
				newsys_rebuild_latest_stories();

			newsys_cache_delete("user",$user_id);
		}

		/* Update database */
		$dbh->query("	DELETE FROM
					newsys_stories
				WHERE
					author_id	= $user_id",DB_NULL);

		$dbh->query("	DELETE FROM
					newsys_comments
				WHERE
					author_id	= $user_id",DB_NULL);

		return $dbh->query("	DELETE FROM
						newsys_users
					WHERE
						user_id		= $user_id",DB_NULL);
	}

	/*******************************************************
	 * Begin newsys log-on session handling functions
	 *******************************************************/

	function newsys_is_logged_in($dbh)
	{
		$user_id = 0;

		switch (newsys_conf("auth_type"))
		{
			case NEWSYS_AUTH_HTTP:
			{
				$username	= @$_SERVER["PHP_AUTH_USER"];
				$password	= @$_SERVER["PHP_AUTH_PW"];

				$password	= newsys_crypt($password);

				$username	= $dbh->prepare_str($username,SQL_REG);
				$password	= $dbh->prepare_str($password,SQL_REG);

				$user_id	= $dbh->query("	SELECT
									user_id
								FROM
									newsys_users
								WHERE
									username = '$username' AND
									password = '$password'",DB_COL);
				break;
			}

			case NEWSYS_AUTH_COOKIE:
			{
				# Checking posted form data
				if (@$_POST["newsys_submitted"])
				{
					$username		= @$_POST["newsys_user"];
					$password		= @$_POST["newsys_pw"];

					$password		= newsys_crypt($password);
/*
 * NOTE: saving the user's encrypted password in their cookie
 * is NOT a good idea. This entire type of auth is highly
 * frowned upon. Use another type instead.
 */
					setCookie("newsys_user",$username);
					setCookie("newsys_pw",$password);

					$_COOKIE["newsys_user"]	= $username;
					$_COOKIE["newsys_pw"]	= $password;
				}

				$username	= $dbh->prepare_str(@$_COOKIE["newsys_user"],	SQL_REG);
				$password	= $dbh->prepare_str(@$_COOKIE["newsys_pw"],	SQL_REG);
				$user_id	= $dbh->query("	SELECT
									user_id
								FROM
									newsys_users
								WHERE
									username = '$username' AND
									password = '$password'",DB_COL);

				break;
			}

			case NEWSYS_AUTH_SESSION:
			{
				newsys_session_start();

				if (@$_POST["newsys_submitted"])
				{
					$username			= @$_POST["newsys_user"];
					$password			= @$_POST["newsys_pw"];

					$password			= newsys_crypt($password);

					$_SESSION["newsys_user"]	= $username;
					$_SESSION["newsys_pw"]		= $password;
				}

				$username	= $dbh->prepare_str(@$_SESSION["newsys_user"],SQL_REG);
				$password	= $dbh->prepare_str(@$_SESSION["newsys_pw"],SQL_REG);
				$user_id	= $dbh->query("	SELECT
									user_id
								FROM
									newsys_users
								WHERE
									username = '$username' AND
									password = '$password'",DB_COL);

				break;
			}

			default:
			{
				newsys_handle_error("Unknown auth type; type: " . newsys_conf("auth_type"));
			}
		}

		return (int)$user_id;
	}

	function newsys_session_start()
	{
		static $called = FALSE;

		if (!$called)
		{
			session_name("newsys");
			session_start();

			$called = TRUE;
		}

		return;
	}

	function newsys_log_in($dbh,$min_level)
	{
		$user_id	= newsys_is_logged_in($dbh);
		$user_type	= $dbh->query("	SELECT
							type
						FROM
							newsys_users
						WHERE
							user_id = $user_id",DB_COL);

		if ($user_id && $user_type >= $min_level)
		{
			return array($user_id,$user_type);
		} else {
			$of = newsys_get_of();

			switch (newsys_conf("auth_type"))
			{
				case NEWSYS_AUTH_HTTP:
				{
					// Should probably be updated to HTTP/1.1
					header("WWW-Authenticate: Basic realm=\"Newsys Administration\"");
					header("HTTP/1.0 401 Unauthorized");

					echo	newsys_get_template("header"),
						$of->header("Error"),
						$of->p("The username and password you supplied is invalid, or your level of authority is not high enough to access the requested resource.");
						newsys_get_template("footer");

					break;
				}

				case NEWSYS_AUTH_COOKIE:
				{

					if (@$_COOKIE["newsys_user"])
					{
						/* User tried to log in */
						echo	newsys_get_template("header"),
							$of->header("Error"),
							$of->p("The username and password you supplied is invalid, or your level of authority is not high enough to access the requested resource."),
							newsys_of_login($of),
							newsys_get_template("footer");

					} else {
						/* User did not try to log in */
						echo	newsys_get_template("header"),
							$of->header("Log In"),
							newsys_of_login($of),
							newsys_get_template("footer");
					}

					break;
				}

				case NEWSYS_AUTH_SESSION:
				{
					if (@$_SESSION["newsys_user"])
					{
						/* User tried to log in */
						echo	newsys_get_template("header"),
							$of->header("Error"),
							$of->p("The username and password you supplied is invalid, or your level of authority is not high enough to access the requested resource."),
							newsys_of_login($of),
							newsys_get_template("footer");
					} else {
						/* User did not try to log in */
						echo	newsys_get_template("header"),
							$of->header("Log In"),
							newsys_of_login($of),
							newsys_get_template("footer");
					}

					break;
				}

				default:
				{
					newsys_handle_error("Unknown auth type; type: " . newsys_conf("auth_type"));
				}
			}

			exit();
		}
	}

	function newsys_log_out()
	{
		switch (newsys_conf("auth_type"))
		{
			case NEWSYS_AUTH_HTTP:
			{
				header("WWW-Authenticate: Basic realm=\"Newsys Administration\",stale=\"TRUE\"");
				header("HTTP/1.0 401 Unauthorized");

				$_SERVER["PHP_AUTH_USER"]	= "";
				$_SERVER["PHP_AUTH_PW"]		= "";

				break;
			}

			case NEWSYS_AUTH_COOKIE:
			{
				$_COOKIE["newsys_user"]		= "";
				$_COOKIE["newsys_pw"]		= "";

				setCookie("newsys_user");
				setCookie("newsys_pw");

				break;
			}

			case NEWSYS_AUTH_SESSION:
			{
				newsys_session_start();

				$_SESSION["newsys_user"]	= "";
				$_SESSION["newsys_pw"]		= "";

				break;
			}

			default:
			{
				newsys_handle_error("Unknown auth type; type: " . newsys_conf("auth_type"));
			}
		}

		return;
	}

	function newsys_of_login($of)
	{
		$path	= newsys_build_path();

		# On the event of a failed log-in attempt
		$redir	= @$_POST["newsys_redir"];

		# Else it'd be a page which referred them here
		if (!$redir)
			$redir = @$_SERVER["REQUEST_URI"];

		# Unless it *is* the log-in page
		if ($redir == "$path/login.php")
			$redir = "";
		else
			$redir = htmlEntities($redir);

		$t = "";

		if (newsys_conf("allow_join"))
			$t = $of->link("Not a member?","$path/admin/user-add.php") . $of->br();

		return	$of->form
			(
				array('action' => "$path/login.php"),
				$of->table
				(
					array('class' => "newsysTable"),
					$of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => "Log In")),
					$of->table_row(array('class' => "newsysDesc",'value' => "Username:"),array('class' => "newsysData1",'value' => $of->input(array('type' => "text",	'name' => "newsys_user")))),
					$of->table_row(array('class' => "newsysDesc",'value' => "Password:"),array('class' => "newsysData2",'value' => $of->input(array('type' => "password",	'name' => "newsys_pw")))),
					$of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$of->input(array('type' => "hidden",'name' => "newsys_redir",'value' => $redir)) .
															$of->input(array('type' => "submit",'name' => "newsys_submitted",'class' => "newsysButton",'value' => "Log In")) .
															$of->input(array('type' => "reset",'class' => "newsysButton",'value' => "Clear")))),
					$of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$t .
															$of->link("Forgot your password?",	"$path/lost-pass.php") .
															$of->br() .
															$of->link("Forgot your username?",	"$path/lost-user.php")))
				)
			);
	}

	function newsys_of_actions($of,$user_type)
	{
		$path		= newsys_build_path();
		$output		= $of->hr()
				. $of->p("Newsys Actions:")
				. $of->list_start(OF_LIST_UN);

		switch ($user_type)
		{
			case NEWSYS_ADMIN:
				$output .=	/*$of->list_item($of->link("Edit Configuration",	"$path/admin/config.php"))
					.*/	$of->list_item($of->link("Add User",			"$path/admin/user-add.php"))
					.	$of->list_item($of->link("List Users (Remove/Edit)",	"$path/admin/user-list.php"));

				if (newsys_conf("use_rss"))
					$output .=	$of->list_item
							(
								  $of->link("Clear RSS",		"$path/admin/rss.php")
								. " [" . newsys_of_popup($of,"Help","$path/help.php?id=2046") . "]"
							);

				if (newsys_conf("use_cache"))
					$output .=	$of->list_item
							(
								  $of->link("Clear Cache",		"$path/admin/cache.php")
								. " [" . newsys_of_popup($of,"Help","$path/help.php?id=2047") . "]"
							);

			case NEWSYS_REGULAR:
				$output	.=	$of->list_item($of->link("Add Story",			"$path/admin/story-add.php"));

			case NEWSYS_COMMENTER:
				$output	.=	$of->list_item($of->link("Edit Profile",		"$path/admin/user-info.php"))
					.	$of->list_item($of->link("Log Out",			"$path/logout.php"));

				/* The cases propagate and an 'admin' condition will see this case */
				if ($user_type != NEWSYS_ADMIN)
					$output .= $of->list_item($of->link("Cancel Account",		"$path/admin/user-cancel.php"));
		}

		$output .= $of->list_end(OF_LIST_UN);

		return $output;
	}

	function newsys_of_close_window($of,$value)
	{
		$data =	addSlashes
			(
				$of->link
				(
					array
					(
						'value'		=> $value,
						'href'		=> "#",
						'onclick'	=> 'window.close()'
					)
				)
			);

		return "<script type=\"text/javascript\">document.write('$data')</script>";
	}

	function newsys_of_popup($of,$value,$href)
	{
		$name = 'ns' . newsys_rand_str(4,NEWSYS_RAND_ALNUM);

		return	$of->link
			(
				array
				(
					'value'		=> $value,
					'href'		=> $href,
					'onclick'	=> "window.open('$href','$name','width=500,height=300'); return false;"
				)
			);
	}

	function newsys_js_dbl_submit()
	{
		return	"if (this.value == 'Processing...')"
		.	"{"
		.		"window.alert('Please allow sufficient time for processing.');"
		.		"return false;"
		.	"} else {"
		.		"this.value = 'Processing...';"
		.		"return true;"
		.	"}";
	}

	/*******************************************************
	 * Begin newsys user-defined fields functions
	 *******************************************************/

	function newsys_fields_validate($type,$data)
	{
		$var	= NULL;
		$errors	= array();

		switch ($type)
		{
			case NEWSYS_FLD_USER:		$var = newsys_conf("user_fields");	break;
			case NEWSYS_FLD_STORY:		$var = newsys_conf("story_fields");	break;
			case NEWSYS_FLD_COMMENT:	$var = newsys_conf("comment_fields");	break;
			default:
				newsys_handle_error("Invalid type to newsys_validate_fields(); type: $type");
		}

/*
 * A little extra processing here will actually on
 * average save more overall, since the script will
 * be executed multiple times between fixing errors
 * in input, might as well just let the user know
 * everything that's wrong at the same
 */
		/* Validate user-defined fields */
		foreach ($var as $user_field_id => $user_field)
		{
			if ($user_field["validation"])
			{
				if (!preg_match($user_field["validation"],$data[$user_field_id]))
					if ($user_field["required"])
					{
						array_push($errors,$user_field_id);
					} else {
						$data[$user_field_id] = $user_field["default_value"];
					}
			}
		}

		return $errors;
	}

	function newsys_fields_update()
	{
		global $php_errormsg;

		$vars		=	array
					(
						'user'		=> newsys_conf("user_fields"),
						'story'		=> newsys_conf("story_fields"),
						'comment'	=> newsys_conf("comment_fields")
					);

		$skip		= FALSE;
		$cache_dir	= newsys_conf("cache_dir");

		if (newsys_conf("use_cache"))
		{
			if (file_exists("$cache_dir/info-user_fields"))
			{
				/* Make sure the content has changed */

				/*
				 * We're already going through the data,
				 * save it to run against the database
				 */
				$changed_vars	=	array
							(
								'user'		=> array('var' => array()),
								'story'		=> array('var' => array()),
								'comment'	=> array('var' => array())
							);
				$anomoly	= FALSE;
				$dbh		= NULL;

				$fp = @fopen("$cache_dir/info-user_fields","r+");

				if (!$fp)
					newsys_handle_error("Cannot open file; file: $cache_dir/info-user_fields");

				while ($line = fgets($fp,NEWSYS_BUF_SIZ))
				{
					$line		= chop($line);
					$old_fields	= @unserialize($line);
					$swt		= FALSE;
					$type		= "";
					$id		= "";
					$var		= "";

					if (!is_array($old_fields))
						continue;

					foreach ($vars as $id => $var)
					{

						$old_val = $anomoly;

						if ($var == $old_fields)
						{
							/* Oops, could have made a mistake, reset the values */
							$anomoly		= FALSE;
							$changed_vars[$id]	= array();

							/* We're done with this variable */
							unset($vars[$id]);
							break;
						} else {
							$anomoly		= TRUE;
							$changed_vars[$id]	= $old_fields;
						}

						$anomoly = $old_val;
					}
				}

				if ($anomoly)
				{
					fseek($fp,0,SEEK_SET);
					newsys_fields_write_cache($fp);
					newsys_fields_write_db(NULL,$changed_vars);
				} else {
					/*
					 * There haven't been any changed, no need to check
					 * the database for any changes
					 */
					$skip = TRUE;
				}

				fclose($fp);
			} else {
				/*
				 * Cache is empty we must fallback to the database's cache
				 * and rebuild the file cache regardless
				 */
				newsys_fields_write_cache();
			}
		}

		/* Else it will check the database for a change */
		if (!$skip)
		{
			$dbh		= newsys_get_dbh();
			$anomoly	= FALSE;
			$changed_vars	=	array
						(
							'user'		=> array(),
							'story'		=> array(),
							'comment'	=> array()
						);
			$old_vars	= array();

			list
			(
				$old_vars['user'],
				$old_vars['story'],
				$old_vars['comment']

			) = $dbh->query("	SELECT
							user_fields_sig,
							story_fields_sig,
							comment_fields_sig
						FROM
							newsys_config",DB_ROW);

			foreach ($old_vars as $id => $old_var)
			{
				$old_fields = @unserialize($old_var);

				if (!is_array($old_fields))
					$old_fields = array('var' => array());

				if ($old_fields != $vars[$id])
				{
					$anomoly = TRUE;
					$changed_vars[$id] = $old_fields;
				}
			}

			if ($anomoly)
				newsys_fields_write_db($dbh,$changed_vars);
		}

		return;
	}

	/* We can optionally be given an open file handle to write */
	function newsys_fields_write_cache($fp = NULL)
	{
		$already_open	= TRUE;
		$cache_dir	= newsys_conf("cache_dir");

		if (!$fp)
		{
			$fp = @fopen("$cache_dir/info-user-fields","w");

			if (!$fp)
				newsys_handle_error("Serious error! The database has been updated and the cache not be updated; file: $cache_dir/info-user-fields");

//			$already_opened = TRUE;
			$already_opened = FALSE;
		}

		$vars =	array
			(
				newsys_conf("user_fields"),
				newsys_conf("story_fields"),
				newsys_conf("comment_fields")
			);

		foreach ($vars as $var)
			fputs($fp,serialize($var) . "\n");

		/* Any old data must be truncated */
		if ($already_opened)
			ftruncate($fp,ftell($fp));
		else
			fclose($fp);

		return;
	}

	function newsys_fields_write_db($dbh,$old_vars)
	{
		$sigs = array();
		$vars =	array
			(
				'user'		=> array('var' => newsys_conf("user_fields"),		'db_name' => "newsys_users"),
				'story'		=> array('var' => newsys_conf("story_fields"),		'db_name' => "newsys_stories"),
				'comment'	=> array('var' => newsys_conf("comment_fields"),	'db_name' => "newsys_comments")
			);

		if (!$dbh)
			$dbh = newsys_get_dbh();

		foreach (array_keys($old_vars) as $id)
		{
			/* Take note of the changed fields */
			$old_fields	= $old_vars[$id]["var"];
			$removed_fields	= array();
			$var_diff	= $vars[$id]["var"];

			foreach ($old_fields as $old_id => $old_field)
			{
				if (@in_array($old_field,$var_diff))
				{
					/*
					 * This field exists in both, remove it
					 * from the diff
					 */
					foreach ($var_diff as $index => $i_field)
						if ($i_field == $old_field)
						{
							/*
							 * By removing it from the difference,
							 * only new, unique fields will exist
							 * in the difference array
							 */
							unset($var_diff[$index]);
							break;
						}
					/*
					 * Else, add it to a changed array, which
					 * will save database data yet perform
					 * minor conversions
					 */
				} else {
					array_push($removed_fields,$old_id);
				}
			}

			foreach ($removed_fields as $removed_field)
				$dbh->query("	ALTER TABLE
							{$vars[$id]["db_name"]}
						DROP
							$removed_field",DB_NULL);

			/* This contains just new fields */
			foreach ($var_diff as $field_id => $added_field)
				$dbh->query("	ALTER TABLE
							{$vars[$id]["db_name"]}
						ADD
							$field_id {$added_field["database_type"]}",DB_NULL);

			$sigs[$id] = $dbh->prepare_str(serialize($vars[$id]),SQL_REG);
		}

		/* Update signatures */

		/* Check to see if one already exists */
		if ($dbh->query("	SELECT
						COUNT(*)
					FROM
						newsys_config",DB_COL))
		{
			$dbh->query("	UPDATE
						newsys_config
					SET
						user_fields_sig		= '{$sigs["user"]}',
						story_fields_sig	= '{$sigs["story"]}',
						comment_fields_sig	= '{$sigs["comment"]}'",DB_NULL);
		} else {
			$dbh->query("	INSERT INTO
						newsys_config
					(
						user_fields_sig,
						story_fields_sig,
						comment_fields_sig
					) VALUES (
						'{$sigs["user"]}',
						'{$sigs["story"]}',
						'{$sigs["comment"]}'
					)",DB_NULL);
		}

		return;
	}

/*
 * Should we add code to check crypt_key for changes
 * so that we can update the database and cryptographic
 * communications?
 *
 * In this case, saving the old, cached key would not
 * be very wise, as it could easily be examined should
 * its contents be stored in a file. We should also
 * probably save a version in the database as well, in
 * the case that the admin has use_cache off.
 */

	if (!defined("NEWSYS_SKIP_INIT"))
		newsys_fields_update();

/*
	if (file_exists(newsys_build_path(NEWSYS_PATH_SYS) . "/install"))
		newsys_handle_error("The `install' directory has not been deleted");
*/
?>
