<?php
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)			= newsys_log_in($newsys_dbh,NEWSYS_REGULAR);
	$newsys_errors		= E_NS_NONE;
	$newsys_user_errors	= array();
	$newsys_skip		= FALSE;
	$newsys_story_fields	= newsys_conf("story_fields");

	if (@$_POST["newsys_t_submitted"])
	{
		$newsys_story =	array
				(
					'author_id'	=> $newsys_user_id,
					'headline'	=> @$_POST["headline"],
					'story'		=> @$_POST["story"],
					'overview'	=> @$_POST["overview"]
				);

		foreach (array_keys($newsys_story_fields) as $newsys_field_id)
			$newsys_story[$newsys_field_id] = @$_POST[$newsys_field_id];

		list
		(
			$newsys_errors,
			$newsys_user_errors

		) =	newsys_story_add($newsys_dbh,$newsys_story);

		if ($newsys_errors == E_NS_NONE && !count($newsys_user_errors))
		{
			echo	newsys_get_template("header"),
				$newsys_of->header("Story Added"),
				$newsys_of->p("Your story has been successfully added."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");

			$newsys_skip = TRUE;
		}
	}

	if (!$newsys_skip)
	{
		echo	newsys_get_template("header"),
			$newsys_of->header("Adding A Story");

		if ($newsys_errors != E_NS_NONE || count($newsys_user_errors))
		{
			$newsys_t = "";

			if ($newsys_errors & E_NS_STORY_STORY)		$newsys_t .= " Please enter a story.";
			if ($newsys_errors & E_NS_STORY_HEADLINE)	$newsys_t .= " Please enter a headline.";

			foreach ($newsys_user_errors as $newsys_user_error)
				$newsys_t .= " " . $newsys_story_fields[$newsys_user_error]["error_message"];

			echo $newsys_of->p($newsys_t);
		}

		$newsys_path = newsys_build_path();

		# Build posting information
		$newsys_t = "Story:" .
				$newsys_of->p
				(
					array('class' => "newsysInfo"),
					$newsys_of->br(),
					newsys_of_popup
					(
						$newsys_of,
						$newsys_of->strong("Overview break"),
						"$newsys_path/help.php?id=".newsys_help_id("overview break")
					),": ",
					$newsys_of->br(),
						newsys_conf("overview_break"),
					$newsys_of->br(),
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

		echo	$newsys_of->form_start(array()),
				$newsys_of->table_start(array('class' => "newsysTable")),
					$newsys_of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => 'Adding A Story')),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Headline:"),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "text",					'name' => "headline")))),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Overview:"),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "textarea",'class' => "newsysSmallTextArea",	'name' => "overview")))),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => $newsys_t),			array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "textarea",'class' => "newsysTextArea",	'name' => "story"))));

		foreach ($newsys_story_fields as $newsys_field_id => $field_id)
			echo		$newsys_of->table_row(array('class' => "newsysDesc",'value' => $newsys_field['name'] . ':'),	array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => $newsys_field['type'],	'name' => $newsys_field_id,'value' => $newsys_field['default_value']))));

		echo			$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' =>	$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Add",'name' => "newsys_t_submitted",'onclick' => newsys_js_dbl_submit())) .
															$newsys_of->input(array('type' => "reset", 'class' => "newsysButton",'value' => "Clear")))),
				$newsys_of->table_end(),
			$newsys_of->form_end(),
			newsys_of_actions($newsys_of,$newsys_user_type),
			newsys_get_template("footer");
	}
?>
