<?php
	require_once "newsys/main.inc";

	$newsys_dbh			= newsys_get_dbh();
	$newsys_of			= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type
	)				= newsys_log_in($newsys_dbh,NEWSYS_COMMENTER);
	$newsys_errors			= E_NS_NONE;
	$newsys_user_errors		= array();
	$newsys_story_id		= (int)@$_REQUEST["story_id"];
	$newsys_parent_comment_id	= (int)@$_REQUEST["parent_comment_id"];
	$newsys_skip			= FALSE;
	$newsys_story			= newsys_story_get($newsys_dbh,$newsys_story_id);

	/* Validate parent comment */
	if
	(
		(
			$newsys_parent_comment_id
			&&
			!$newsys_dbh->query("	SELECT
							comment_id
						FROM
							newsys_comments
						WHERE
							comment_id = $newsys_parent_comment_id",DB_COL)
		)
		||
		!newsys_conf("hier_comments")
	)
		$newsys_parent_comment_id = 0;

	if (is_array($newsys_story))
	{
		$newsys_comment_fields = newsys_conf("comment_fields");

		if (@$_POST["newsys_t_submitted"])
		{
			$newsys_comment =	array
						(
							'story_id'		=> $newsys_story_id,
							'parent_comment_id'	=> $newsys_parent_comment_id,
							'author_id'		=> $newsys_user_id,
							'comment'		=> @$_POST["comment"],
							'subject'		=> @$_POST["subject"]
						);

			foreach (array_keys($newsys_comment_fields) as $newsys_field_id)
				$newsys_comment[$newsys_field_id] = @$_POST[$newsys_field_id];

			list ($newsys_errors,$newsys_user_errors) = newsys_comment_add($newsys_dbh,$newsys_comment);

			if ($newsys_errors == E_NS_NONE && !count($newsys_user_errors))
			{
				echo	newsys_get_template("header"),
					$newsys_of->header("Comment Added"),
					$newsys_of->p("You have successfully commented on this story."),
					newsys_of_actions($newsys_of,$newsys_user_type),
					newsys_get_template("footer");

				$newsys_skip = TRUE;
			}
		}

		if (!$newsys_skip)
		{
			echo	newsys_get_template("header"),
				$newsys_of->header("Adding A Comment");

			if ($newsys_errors != E_NS_NONE || count($newsys_user_errors))
			{
				$newsys_t = "You have entered invalid input.";

				if ($newsys_errors & E_NS_COM_COM)		$newsys_t .= " Please enter a comment.";
				if ($newsys_errors & E_NS_COM_SUBJECT)		$newsys_t .= " Please enter a subject.";
				if ($newsys_errors & E_NS_COM_NO_STORY)		$newsys_t .= " You cannot comment on a story which does not exist.";
				if ($newsys_errors & E_NS_COM_NOT_ALLOWED)	$newsys_t .= " Commenting to stories is not allowed at this time.";
				if ($newsys_errors & E_NS_COM_MAX_EXCEED)	$newsys_t .= " The maximum number of comments has exceeded for this story.";

				foreach ($newsys_user_errors as $newsys_user_error)
					$newsys_t .= " " . $newsys_comment_fields[$newsys_user_error]["error_message"];

				echo $newsys_of->p($newsys_t);
			}

			$newsys_path = newsys_build_path();

			$newsys_t = "Comment:"
				.	$newsys_of->p
					(
						array('class' => "newsysInfo"),
						$newsys_of->br(),
						newsys_of_popup
						(
							$newsys_of,
							$newsys_of->strong("Auto-URLs"),
							"$newsys_path/help.php?id=".newsys_help_id("auto-urls")
						),": ",
							(newsys_conf("story_auto_urls") ? "on" : "off"),
						$newsys_of->br(),
						$newsys_of->br(),
						newsys_of_popup
						(
							$newsys_of,
							$newsys_of->strong("Allowed HTML"),
							"$newsys_path/help.php?id=".newsys_help_id("allowed html")
						),": ",
						$newsys_of->br(),
						(
							count(newsys_conf("story_allowed_html")) ?
							join(", ",newsys_conf("story_allowed_html")) :
							"none"
						),
						$newsys_of->br(),
						$newsys_of->br(),
						newsys_of_popup
						(
							$newsys_of,
							$newsys_of->strong("HTML attributes"),
							"$newsys_path/help.php?id=".newsys_help_id("html attributes")
						),": ",
						$newsys_of->br(),
						(
							count(newsys_conf("story_allowed_attr")) ?
							join(", ",newsys_conf("story_allowed_attr")) :
							"none"
						)
					);

			$newsys_story = newsys_story_get($newsys_dbh,$newsys_story_id);

			echo	$newsys_of->p("You are adding a comment to the following story:"),
				newsys_get_template("story",$newsys_story_id);

			if ($newsys_parent_comment_id && newsys_conf("hier_comments"))
				echo newsys_get_template("comment",$newsys_story_id,$newsys_parent_comment_id);

			echo	$newsys_of->form_start(array()),
					$newsys_of->table_start(array('class' => "newsysTable")),
						$newsys_of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => "Adding A Comment")),
						$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Subject:"),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => 'text','name' => "subject",'value' => $newsys_story["headline"])))),
						$newsys_of->table_row(array('class' => "newsysDesc",'value' => $newsys_t),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => 'textarea','class' => "newsysSmallTextArea",'name' => "comment"))));

			foreach ($newsys_comment_fields as $newsys_field_id => $newsys_field)
				echo		$newsys_of->table_row(array('class' => "newsysDesc",'value'	=> $newsys_field['name'] . ':'),	array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => $newsys_field['type'],'name' => $newsys_field_id,'value' => $newsys_field['default_value']))));

			echo			$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$newsys_of->input(array('type' => "hidden",'name' => "story_id",		'value' => $newsys_story_id)) .
																$newsys_of->input(array('type' => "hidden",'name' => "parent_comment_id",	'value' => $newsys_parent_comment_id)) .
																$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Add",'name' => "newsys_t_submitted",'onclick' => newsys_js_dbl_submit())) .
																$newsys_of->input(array('type' => "reset", 'class' => "newsysButton",'value' => "Clear")))),
					$newsys_of->table_end(),
				$newsys_of->form_end(),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		}
	} else {
		echo	newsys_get_template("header"),
			$newsys_of->header("Access Denied"),
			$newsys_of->p("You cannot comment on this story because it does not exist."),
			newsys_of_actions($newsys_of,$newsys_user_type),
			newsys_get_template("footer");
	}
?>
