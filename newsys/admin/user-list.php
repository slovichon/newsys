<?php
	define("NEWSYS_SKIP_CHECK",TRUE,TRUE);
	require_once "newsys/main.inc";

	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)			= newsys_log_in($newsys_dbh,NEWSYS_ADMIN);

	echo	newsys_get_template("header"),
		$newsys_of->header("Listing Users"),
		$newsys_of->table_start(array('class' => "newsysTable")),
			$newsys_of->table_row(array('class' => "newsysHeader",'value' => 'Username'),array('class' => "newsysHeader",'value' => 'Edit Profile'),array('class' => "newsysHeader",'value' => 'Delete'));

	$newsys_dbh->query("	SELECT
					username,
					user_id
				FROM
					newsys_users",DB_ROWS);

	$newsys_path = newsys_build_path();

	while ($newsys_user = $newsys_dbh->fetch_row())
	{
		$newsys_class = newsys_gen_class();

		echo	$newsys_of->table_row
			(
				array('class' => $newsys_class,'value' => $newsys_user["username"]),
				array
				(
					'class' => $newsys_class,
					'value' =>	$newsys_of->form
							(
								array('action' => "$newsys_path/admin/user-info.php"),
								$newsys_of->input(array('type' => "hidden",'name' => "target_user_id",'value' => $newsys_user["user_id"])),
								$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Edit",'onclick' => newsys_js_dbl_submit()))
							)
				),
				array
				(
					'class' => $newsys_class,
					'value' =>	$newsys_of->form
							(
								array('action' => "$newsys_path/admin/user-remove.php"),
								$newsys_of->input(array('type' => "hidden",'name' => "user_id",'value' => $newsys_user["user_id"])),
								$newsys_of->input(array('type' => "submit",'class' => "newsysButton",'value' => "Remove",'onclick' => newsys_js_dbl_submit()))
							)
				)
			);
	}

	echo	$newsys_of->table_end(),
		newsys_of_actions($newsys_of,$newsys_user_type),
		newsys_get_template("footer");
?>
