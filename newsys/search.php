<?php
	/*
	 * This page is the heart of displaying data
	 * in Newsys. Here is a list of some of its
	 * directives:
	 *
	 *	offset=num	start listing results starting at `num'
	 *	show=template	explain what output format is desired
	 *	cri_keys[]=key	select data based on a condition associated with a characteristic of a record defined by `key
	 *	cri_vals[]=val	select data based on a condition where a given value matches a given key
	 *	cri_ops[]=op	describe the relationship between key/value criterion
	 *	cri_groups[]=g	describe other rules which require completion at a certain level ("group")
	 */
	require_once "newsys/main.inc";

	$newsys_of		= newsys_get_of();
	$newsys_template	= @$_REQUEST["show"];

	# If no valid template is given, show a search form
	if (!newsys_template_exists($newsys_template))
		$newsys_template = NULL;

	# Gather search criterion
	$newsys_cri_keys	= @$_REQUEST["cri_keys"];
	$newsys_cri_vals	= @$_REQUEST["cri_vals"];
	$newsys_cri_ops		= @$_REQUEST["cri_ops"];
	$newsys_cri_groups	= @$_REQUEST["cri_groups"];
	
	$newsys_where_clause	= "";
	
	if ($newsys_len = count($newsys_cri_keys))
	{
		$newsys_where	= array();

		for ($newsys_i = 0; $newsys_i < $newsys_len; $newsys_i++)
		{
			if (newsys_)
			{
				array_push
				(
					$newsys_where[(int)$newsys_cri_groups[$newsys_i]],
				);
			}
		}
		
		$newsys_sql	.= " WHERE ";
	}

	$newsys_sql		= "	SELECT
						$newsys_template_fields
					FROM
						$newsys_search_tables
					$newsys_where_clause";









	

	if (@$_GET["search_string"])
	{
		$newsys_db_table	= "";
		$newsys_db_tables	= "";
		$newsys_db_field	= "";
		$newsys_format_name	= "";
		$newsys_offset		= (int)@$_GET["offset"];
		$newsys_order		= @$_GET["order"];
		$newsys_dbh		= newsys_get_dbh();
		$newsys_search_str	= $newsys_dbh->prepare_str($_GET["search_string"],SQL_WILD);
		$newsys_total		= 0;
		$newsys_cond		= "";
		$newsys_count		= 0; # Theoretically $offset + $limit

		# What to search in, story or headline.
		switch (@$_GET["what"])
		{
			case "story":
			{
				$newsys_db_table	= "newsys_stories";
				$newsys_db_tables	= "newsys_stories";
				$newsys_db_field	= "story";
				$newsys_format_name	= "Story";
				break;
			}

			case "comment":
			{
				$newsys_db_table	= "newsys_comments,newsys_stories";
				$newsys_db_tables	= "newsys_comments,newsys_stories";
				$newsys_db_field	= "comment";
				$newsys_format_name	= "Comments";
				$newsys_cond		.= " newsys_comments.story_id = newsys_stories.story_id AND ";
				break;
			}

			case "headline":
			default:
			{
				$newsys_db_table	= "newsys_stories";
				$newsys_db_tables	= "newsys_stories";
				$newsys_db_field	= "headline";
				$newsys_format_name	= "Headline";
			}
		}

		# Default action to list in an ascending fashion
		if ($newsys_order != "ASC" && $newsys_order != "DESC")
			$newsys_order = "ASC";

		$newsys_cond .= " newsys_stories.author_id = newsys_users.user_id AND ";

		# If user checked case-sensitive checkbox.
		if (@$_GET["case"])
		{
			$newsys_cond .= " $newsys_db_table.$newsys_db_field		LIKE '%$newsys_search_str%' ";
		} else {
			$newsys_cond .= " LOWER($newsys_db_table.$newsys_db_field)	LIKE '%" . strToLower($newsys_search_str) . "%' ";
		}

		$newsys_total = $newsys_dbh->query("	SELECT DISTINCT
								COUNT(*)
							FROM
								$newsys_db_tables,
								newsys_users
							WHERE
								$newsys_cond",DB_COL);

		$newsys_count = 0;

		if ($newsys_total)
		{
			if ($newsys_offset > $newsys_total)
				$newsys_offset = 0;

			$newsys_dbh->query("	SELECT DISTINCT
							newsys_stories.story_id,
							newsys_stories.headline,
							newsys_stories.create_date,
							newsys_users.username,
							newsys_users.user_id
						FROM
							$newsys_db_tables,
							newsys_users
						WHERE
							$newsys_cond
						ORDER BY
							create_date $newsys_order
						LIMIT
							$newsys_offset,
							" . newsys_conf("result_limit"),DB_ROWS);

			$newsys_path = newsys_build_path();

			echo	newsys_get_template("header"),
				$newsys_of->header("Search Results"),
				$newsys_of->p("Showing results " . ($newsys_offset + 1) . "-" . $newsys_dbh->num_rows() . " for " . $newsys_of->strong(htmlEntities($_GET["search_string"])) . "."),
				$newsys_of->table_start(array('class' => "newsysTable")),
					$newsys_of->table_row(array('class' => "newsysHeader",'value' => "Author"),array('class' => "newsysHeader",'value' => "Headline"),array('class' => "newsysHeader",'value' => "Date"));

			$newsys_hi_match = preg_quote($_GET["search_string"],"/");

			while ($newsys_row = $newsys_dbh->fetch_row())
			{
				$newsys_col_class	= newsys_gen_class();
				$newsys_ts		= new TimeStamp($newsys_row["create_date"]);

				/* Highlight search terms */
				$newsys_row[$newsys_db_field] = preg_replace("/$newsys_hi_match/ie",'$newsys_of->strong("$0")',$newsys_row[$newsys_db_field]);

				echo	$newsys_of->table_row
					(
						array('class' => $newsys_col_class,'value' => $newsys_of->link($newsys_row["username"],$newsys_path . "/profile.php?user_id=" . $newsys_row["user_id"])),
						array('class' => $newsys_col_class,'value' => $newsys_of->link($newsys_row["headline"],$newsys_path . "/view.php?story_id="   . $newsys_row["story_id"])),
						array('class' => $newsys_col_class,'value' => $newsys_ts->format(newsys_conf("time_format")))
					);
			}

			$newsys_base_url =	$_SERVER["PHP_SELF"]
						. "?search_string=" . urlEncode($_GET["search_string"]) /* Preserve original user input */
						. "&amp;what=$newsys_db_field"
						. "&amp;order=$newsys_order"
						. "&amp;case=" . (@$_GET["case"] ? 1 : 0)
						. "&amp;offset=";

			echo		$newsys_of->table_row
					(
						array
						(
							'class'		=> "newsysFooter",
							'colspan'	=> 3,
							'value'		=>	newsys_of_nav_menu
										(
											$newsys_of,
											array
											(
												"offset"	=> $newsys_offset,
												"total"		=> $newsys_total,
												"url"		=> $newsys_base_url
											)
										)
						)
					),
					$newsys_of->table_end(),
					$newsys_of->p($newsys_of->link("Conduct a new search","$newsys_path/search.php")),
					newsys_get_template("footer");
		} else {
			# No results were returned
			echo	newsys_get_template("header"),
				$newsys_of->header("Search Results"),
				$newsys_of->p("No results were returned by your search parameters."),
				$newsys_of->p($newsys_of->link("Conduct a new search",newsys_build_path() . "/search.php")),
				newsys_get_template("footer");
		}
	} else {
		# No search string specified, show search form
		echo	newsys_get_template("header"),
			$newsys_of->header("Search"),
			$newsys_of->form
			(
				array('method' => "get"),
				$newsys_of->table
				(
					array('class' => "newsysTable"),
					$newsys_of->table_row(array('class' => "newsysHeader",'colspan' => 2,'value' => "Search")),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Search:"),	array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => 'select','name' => "what",'options' => array('headlines' => "Headlines",'comment' => "Comments",'story' => "Stories"))))),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "For:"),	array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "text",  'name' => "search_string")))),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Sort:"),	array('class' => newsys_gen_class(),'value' => $newsys_of->input(array('type' => "select",'name' => "order",'options' => array('ASC' => "Ascending",'DESC' => "Descending"))))),
#					$newsys_of->input(array('type' => "checkbox",'class' => "newsysCheckbox",'name' => "case",'label' => "Case-sensitive")),
					$newsys_of->table_row(array('colspan' => 2,'class' => 'newsysFooter','value' => $newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Search"))))
				)
			),
			newsys_get_template("footer");
	}
?>
