<?php
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)			= newsys_log_in($newsys_dbh,NEWSYS_ADMIN);
	$newsys_target_user_id	= (int)@$_REQUEST["user_id"];

	if (@$_POST["newsys_t_submitted"])
	{
		if (@$_POST["user_id"] && newsys_user_remove($newsys_dbh,$_POST["user_id"]))
		{
			echo	newsys_get_template("header"),
				$newsys_of->header("User Removed"),
				$newsys_of->p("This user was successfully removed from the database."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("The requested user could not be found."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		}
	} elseif ($newsys_target_user_id) {

		$newsys_user = newsys_user_get($newsys_dbh,$newsys_target_user_id);

		if (is_array($newsys_user))
		{
			echo	newsys_get_template("header"),
				$newsys_of->header("Removing User"),
				$newsys_of->p("Are you sure you want to remove the user ",$newsys_of->strong($newsys_user["username"]),"?"),
				$newsys_of->form
				(
					array(),
					$newsys_of->input(array('type' => "hidden",'name' => "user_id",'value' => $newsys_target_user_id)),
					$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Remove",'name' => "newsys_t_submitted",'onclick' => newsys_js_dbl_submit()))
				),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("The requested user could not be found."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		}
	} else {
		$newsys_users = array();

		$newsys_dbh->query("	SELECT
						user_id
						username
					FROM
						newsys_users",DB_ROWS);

		while (list ($newsys_target_user_id,$newsys_target_user_name) = $newsys_dbh->fetch_row())
			$newsys_users[$newsys_target_user_id] = $newsys_target_user_name;

		echo	newsys_get_template("header"),
			$newsys_of->header("Removing User"),
			$newsys_of->form
			(
				array(),
				$newsys_of->table
				(
					array(),
					$newsys_of->table_row(array('class' => "newsysDesc",'value' => "Username:"),array('class' => "newsysData1",'value' => $newsys_of->input(array('type' => "select",'name' => "user_id",'options' => $newsys_users)))),
					$newsys_of->table_row(array('class' => "newsysFooter",'colspan' => 2,'value' => $newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Remove",'onclick' => newsys_js_dbl_submit()))))
				)
			),
			newsys_of_actions($newsys_of,$newsys_user_type),
			newsys_get_template("footer");
	}
?>
