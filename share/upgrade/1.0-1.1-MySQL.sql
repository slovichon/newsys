CREATE TABLE
	newsys_users
(
/*	user_id		INT			NOT NULL	AUTO_INCREMENT,	*/
/*	username	VARCHAR(30)		NOT NULL,			*/
/*	password	CHAR(60)		NOT NULL,			*/
/*	email		VARCHAR(30),						*/
/*	type		ENUM('1','2','3')	NOT NULL,			*/

	new_password	CHAR(60),
	recover_key	CHAR(20),

	cancel_key	CHAR(20),

	PRIMARY KEY(user_id),
	UNIQUE(user_id),
	UNIQUE(username)

) SELECT
	user_id,		/* user_id		*/
	username,		/* username		*/
	password,		/* password		*/
	email,			/* email		*/
	type			/* type			*/
				/* new_password		*/
				/* recover_key		*/
				/* cancel_key		*/
FROM
	users;

UPDATE
	newsys_users
SET
	type = '3'
WHERE
	type = '4';

ALTER TABLE
	newsys_users
CHANGE 
	type		type		ENUM('1','2','3')	NOT NULL;

ALTER TABLE
	newsys_users
CHANGE
	password	password	CHAR(60)		NOT NULL;

ALTER TABLE
	newsys_users
CHANGE
	email		email		CHAR(30);

/**************************************
 * Remember to update user passwords! *
 **************************************/

/*************************************************
 * DROP TABLE users;
 *************************************************/

CREATE TABLE
	newsys_stories
(
/*	story_id	INT			NOT NULL	AUTO_INCREMENT,		*/
/*	author_id	INT			NOT NULL,				*/
/*	headline	VARCHAR(64)		NOT NULL,
	overview	VARCHAR(255)		NOT NULL,
	story		MEDIUMTEXT		NOT NULL,
	create_date	TIMESTAMP,							*/
	mod_date	TIMESTAMP,
/*	num_comments	INT,								*/

	/* Whether or not further comments can be posted */
	allow_comments	  BOOL							    DEFAULT 1,

	PRIMARY KEY(story_id),
	UNIQUE(story_id)

) SELECT
	article_id as story_id,		/* story_id		*/
	author_id,			/* author_id		*/
	headline,			/* headline		*/
	story as overview,		/* overview		*/
	story,				/* story		*/
	create_date,			/* create_date		*/
					/* mod_date		*/
	num_comments			/* num_comments		*/
					/* allow_comments	*/
FROM
	news;

ALTER TABLE
	newsys_stories
CHANGE
	headline	headline	VARCHAR(64)	NOT NULL;

ALTER TABLE
	newsys_stories
CHANGE
	overview	overview	VARCHAR(255)	NOT NULL;

ALTER TABLE
	newsys_stories
CHANGE
	story_id	story_id	INT		NOT NULL				AUTO_INCREMENT;

ALTER TABLE
	newsys_stories
CHANGE
	num_comments	num_comments	INT		NOT NULL		DEFAULT 0;

UPDATE
	newsys_stories
SET
	allow_comments = 1;

/*************************************************
 * DROP TABLE news;
 *************************************************/

CREATE TABLE
	newsys_comments
(
/*	story_id	INT			NOT NULL,	*/
/*	comment_id      INT			NOT NULL,
	author_id       INT			NOT NULL,	*/
	subject		VARCHAR(64)		NOT NULL,
/*	comment		MEDIUMTEXT		NOT NULL,
	create_date	TIMESTAMP,				*/
	mod_date	TIMESTAMP,

	/* For hierarchical commenting */
	parent_comment_id       INT		NOT NULL				DEFAULT 0,

	INDEX(story_id),
	INDEX(comment_id)

) SELECT
	article_id as story_id,		/* story_id		*/
	comment_id,			/* comment_id		*/
	author_id,			/* author_id		*/
					/* subject		*/
	comment,			/* comment		*/
	create_date			/* create_date	*/
					/* mod_date		*/
					/* parent_comment_id	*/
FROM
	comments;

UPDATE
	newsys_comments
SET
	subject = 'Comment';

/*************************************************
 * DROP TABLE comments;
 *************************************************/

CREATE TABLE newsys_last_stories
(
	story_id		INT		     NOT NULL,
	author_id	       INT		     NOT NULL,
	create_date	TIMESTAMP,

	PRIMARY KEY(story_id),
	INDEX(author_id)
);

/* using variables seems to be too database-specific */
CREATE TABLE newsys_config
(
	crypt_key_sig	   VARCHAR(22),

	user_fields_sig	 TEXT,
	story_fields_sig	TEXT,
	comment_fields_sig      TEXT
);
