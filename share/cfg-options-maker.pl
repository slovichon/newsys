#!/usr/bin/perl -w

use strict;

sub dir
{
	local $_;
	
	my ($name,$type,@desc) = @_;

	print 	"\t" x 5,"newsys_",$name,"\n",
		"\t" x 6,$type,"\n\n";

	for (@desc)
	{
		$_ = "\t$_";
		
		s/\t/" " x 8/e;
=cut
		while (1)
		{
			s/\w+\W*/length($&) + ws_len($`) > 72 ? "\n$&" : $&/e;
			last unless /[^\n]{72}/;
		}
=cut	
		print;
		print "\n";
	}

	print "\n";

	return
}

sub ws_len
{
	my ($tok) = shift;
	local $&;

	$tok =~ /.*\z/;

	return length($&);
}

dir("encode_salt",		"string",	"Salt to use in which to encode user passwords before entering the database");
dir("show_errors",		"boolean",	"Determines whether or not to display error output on generated pages");
dir("time_format",		"string",	"PHP's date() format used to display date/times in generated pages");
dir("result_limit",		"integer",	"Number of content items to display per page");
dir("page_limit",		"integer",	"Number of page links to show before/after current page");
dir("allow_comments",		"boolean",	"Determines whether to allow users to comment to posted stories");
dir("auth_type",		"enum",		"Type of authentication to use when validating user logins",
						"Can be one of NEWSYS_AUTH_HTTP (HTTP authentication) or NEWSYS_AUTH_COOKIE (stored cookie)");
dir("hier_comment",		"boolean",	"Determines if hierarchical commenting or one-dimesional style should be used");
dir("story_word_length",	"integer",	"Number of non-whitespace characters to be allotted before whitespace should be forced in between");
dir("story_auto_urls",		"boolean",	"Determines whether to turn potential URLs into hyperlinks");
dir("story_allowed_html",	"array",	"Contains an array of feasible HTML tags to not be converted to HTML entities in posted stories/comments");
dir("story_html_attr",		"boolean",	"Determines whether to allow HTML tag attributes in HTML tags from posted stories/comments");
dir("levels",			"array",	"Contains an array of labels for various types of users");
dir("error_log",		"string",	"File name/path to newsys error log for logging errors");
dir("mail_errors",		"boolean",	"Determines whether to send e-mail notices to site administrator warning of newsys errors");
dir("xml_files",		"array",	"Contains an array of filenames to RSS and other XML application datafiles");
dir("path",			"string",	"Relative path from site root to root newsys directory");
dir("site_name",		"string",	"Web site name");
dir("site_uri",			"string",	"URI to Web site root");
dir("site_email",		"string",	"E-mail address of site administrator");
dir("cache_dir",		"string",	"Directory to newsys cache",
						"Must be writable as the user newsys runs as");
dir("templates_dir",		"string",	"Directory to newsys templates");
