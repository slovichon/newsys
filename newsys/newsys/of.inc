<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 03:50:08 PM
	 *
	 * of.inc - output formatting routines
	 */

	function newsys_of_login($of)
	{
		$path	= newsys_build_path();

		# On the event of a failed log-in attempt
		$redir	= newsys_http_post("newsys_redir");

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
								. " [" . newsys_of_popup($of,"Help","$path/help.php?id=".newsys_help_id("clear rss")) . "]"
							);

				if (newsys_conf("use_cache"))
					$output .=	$of->list_item
							(
								  $of->link("Clear Cache",		"$path/admin/cache.php")
								. " [" . newsys_of_popup($of,"Help","$path/help.php?id=".newsys_help_id("clear cache")) . "]"
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

	function newsys_of_nav_menu($of,$prefs)
	{
		$offset		= (int)$prefs["offset"];
		$total		= (int)$prefs["total"];
		$url		= $prefs["url"];
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
?>
