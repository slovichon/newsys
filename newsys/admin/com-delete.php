<?php
	define("NEWSYS_SKIP_CHECK",TRUE,TRUE);
	require_once "newsys/main.inc";

/*
 * Note that this page used to require only
 * NEWSYS_COMMENTER but it was decided that
 * users should not be allowed to remove their
 * own comments.
 *
 * There are a variety of reasons for choosing
 * to do this, the most obvious leaving the
 * question of what to do about child comments
 * in hierarchial posting.
*/
	$newsys_dbh		= newsys_get_dbh();
	$newsys_of		= newsys_get_of();
	list
	(
		$newsys_user_id,
		$newsys_user_type

	)			= newsys_log_in($newsys_dbh,NEWSYS_ADMIN);
	$newsys_story_id	= (int)@$_REQUEST["story_id"];
	$newsys_comment_id	= (int)@$_REQUEST["comment_id"];
	$newsys_author_id	= $newsys_dbh->query("	SELECT
								author_id
							FROM
								newsys_comments
							WHERE
								story_id   = $newsys_story_id	AND
								comment_id = $newsys_comment_id",DB_COL);

#	if ($newsys_author_id == $newsys_user_id || $newsys_user_type > NEWSYS_REGULAR)
#	{
		if (@$_POST["newsys_t_submitted"])
		{
			newsys_comment_delete($newsys_dbh,$newsys_story_id,$newsys_comment_id);

			echo	newsys_get_template("header"),
				$newsys_of->header("Comment Deleted"),
				$newsys_of->p("This comment has been successfully deleted."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");

		} elseif (newsys_comment_get($newsys_dbh,$newsys_story_id,$newsys_comment_id)) {

			echo	newsys_get_template("header"),
				$newsys_of->header("Deleting Comment"),
				$newsys_of->form
				(
		#			array('method' => "get"),
					array(),
					$newsys_of->p("You are about to delete this comment. Are you sure?"),
					newsys_get_template("comment",$newsys_story_id,$newsys_comment_id),
					$newsys_of->input(array('type' => 'hidden','name' => "story_id",		'value' => $newsys_story_id)),
					$newsys_of->input(array('type' => 'hidden','name' => "comment_id",		'value' => $newsys_comment_id)),
					$newsys_of->input(array('type' => "submit",'name' => "newsys_t_submitted",	'value' => "Delete",'class' => "newsysButton",'onclick' => newsys_js_dbl_submit()))
				),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		} else {
			echo	newsys_get_template("header"),
				$newsys_of->header("Error"),
				$newsys_of->p("The requested comment could not be found."),
				newsys_of_actions($newsys_of,$newsys_user_type),
				newsys_get_template("footer");
		}
#	} else {
#		echo	newsys_get_template("header"),
#			$newsys_of->header("Access Denied"),
#			$newsys_of->p("Your level of authority does not permit you to delete this comment."),
#			newsys_of_actions($newsys_of,$newsys_user_type),
#			newsys_get_template("footer");
#	}
?>
