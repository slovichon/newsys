<?php
	define("NS_T_BOOL",	1,	1);
	define("NS_T_STR",	2,	1);
	define("NS_T_INT",	3,	1);
	define("NS_T_FILE",	4,	1);
	define("NS_T_ARRAY",	5,	1);
	define("NS_T_FIELDS",	6,	1);

	function newsys_t_row($desc,$name,$type)
	{
		static $count = 1;

		$args		= func_get_args();
		$desc		= array_shift($args);
		$name		= array_shift($args);
		$type		= array_shift($args);

		# Column one data
		$t_col1		= array('class' => "newsysDesc",'value' => $name . ":");
		$class		= $count++ % 2 ? "newsysData1" : "newsysData2";

		# Field name
		$t_field_name	= "";

		switch ($type)
		{
			case NS_T_BOOL:
			{
				$t_field_name = "checked";
			}

			case NS_T_INT:
			case NS_T_STR:
			case NS_T_FILE:
			{
				if (!$t_field_name)
					$t_field_name = "value";

				$t_input =	array
						(
							'type'		=> "text",
							'name'		=> $name,
							$t_field_name	=> $GLOBALS[$name]
						);

				if ($type == NS_T_INT)
					$t_input['validate'] = '/^\d+$/';

				return	OF::table_row
					(
						$t_col1,
						array
						(
							'class' => $class,
							'value' => OF::input($t_input)
						)
					);
			}

			case NS_T_ARRAY:
			{
				return	OF::table_row
					(
						$t_col1,
						array
						(
							'class' => $class,
							'value' =>	OF::input
									(
										array
										(
											'type'	=> "text",
											'name'	=> $name,
											'value'	=> join(', ',$GLOBALS["name"])
										)
									)
						)
					);
			}

			case NS_T_ENUM:
			{
				$values = array();

				switch ($name)
				{
					case "newsys_auth_type":
					{
						$values =	array
								(
									NEWSYS_AUTH_HTTP	=> "HTTP",
									NEWSYS_AUTH_COOKIE	=> "Cookie",
									NEWSYS_AUTH_SESSION	=> "Session"
								);
						break;
					}
				}

				return	OF::table_row
					(
						$t_col1,
						array
						(
							'class' => $class,
							'value' =>	OF::input
									(
										array
										(
											'type'		=> "select",
											'name'		=> $name,
											'options'	=> $values,
											'value'		=> $GLOBALS[$name]
										)
									)
						)
					);
			}

			case NS_T_FIELDS:
			{
			}

			case NS_T_SET:
			{
				$values	= $GLOBALS[$name];
				$t	= "";

				foreach ($values as $i_key => $i_val)
					$t .=	OF::input
						(
							array
							(
								'type'		=> "text",
								'name'		=> $name . "[$i_key]",
								'value'		=> $i_val
							)
						)
						. OF::br();

				return	OF::table_row
					(
						$t_col1,
						array
						(
							'class' => $class,
							'value' => $t
						)
					);
			}
		}
	}

	require "../newsys.inc";

	$dbh		= new DBH;
	list
	(
		$ns_user_id,
		$ns_user_type

	)		= newsys_log_in($dbh,NEWSYS_ADMIN);
	$ns_skip	= FALSE;
	$ns_errors	= E_NS_NONE;

	if (@$_POST["ns_submitted"])
	{
	}

	if (!$skip)
	{
		echo	newsys_get_template("header"),
			OF::header("Editing Configuration");

		if ($ns_errors != E_NS_NONE)
		{
		}

		echo	OF::form
			(
				array(),
				OF::table
				(
					array(),
					OF::table_head('Option','Value'),
					newsys_t_row('Show Errors',		'newsys_show_errors',		NS_T_BOOL),
					newsys_t_row('Time Format',		'newsys_time_format',		NS_T_STR),
					newsys_t_row('Result Limit',		'newsys_result_limit',		NS_T_INT),
					newsys_t_row('Page Limit'		'newsys_page_limit',		NS_T_INT),
					newsys_t_row('Allow Comments',		'newsys_allow_comments',	NS_T_BOOL),
					newsys_t_row('Authentication Method',	'newsys_auth_type',		NS_T_ENUM),
					newsys_t_row('Hierarchy Commenting',	'newsys_hier_comment',		NS_T_BOOL),
					newsys_t_row('Maximum Word Length',	'newsys_story_word_length',	NS_T_INT),
					newsys_t_row('Auto URLs',		'newsys_story_auto_urls',	NS_T_BOOL),
					newsys_t_row('Allowed HTML',		'newsys_story_allowed_html',	NS_T_ARRAY),
					newsys_t_row('Allow HTML Attributes',	'newsys_story_html_attr',	NS_T_BOOL),
					newsys_t_row('Extra User Fields',	'newsys_user_fields',		NS_T_FIELDS),
					newsys_t_row('Extra Story Fields',	'newsys_story_fields',		NS_T_FIELDS),
					newsys_t_row('Extra Comment Fields',	'newsys_comment_fields',	NS_T_FIELDS),
					newsys_t_row('Access Levels',		'newsys_levels',		NS_T_SET),
					newsys_t_row('Error Log',		'newsys_error_log',		NS_T_FILE),
					newsys_t_row('Mail Errors',		'newsys_mail_errors',		NS_T_BOOL),
					newsys_t_row('XML Files',		'newsys_xml_files',		NS_T_SET),
					newsys_t_row('Newsys Path',		'newsys_path',			NS_T_FILE),
					newsys_t_row('Site Name',		'newsys_site_name',		NS_T_STR),
					newsys_t_row('Site URI',		'newsys_site_uri',		NS_T_STR),
					newsys_t_row('Administrator E-mail',	'newsys_site_email',		NS_T_STR),
					newsys_t_row('Cache Directory',		'newsys_cache_dir',		NS_T_FILE),
					newsys_t_row('Use Cache',		'newsys_use_cache',		NS_T_BOOL),
					newsys_t_row('Templates Directory',	'newsys_templates_dir',		NS_T_FILE),
					newsys_t_row('Temporary Directory',	'newsys_temp_dir',		NS_T_FILE),
				)
			),
			newsys_of_actions($ns_user_type),
			newsys_get_template("footer");
	}
?>
