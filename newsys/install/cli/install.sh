#!/bin/sh

# Newsys installer
# Actions
#	Build newsys-config.inc
#	Build create.sql
#	Generate crypto key

echo "+--------------------------------------"
echo -n "| Generating crypto key... "

key=$(php -q gen-key.php)

if [ -z $key ]; then
	echo "error!"
	echo
	echo "Please make sure a PHP executable is in your"
	echo "PATH environmental variable and re-run this"
	echo "installer."

	exit 0
fi

echo "done"
echo "+--------------------------------------"
echo
echo "+--------------------------------------"
echo -n "| Building config file... "

sed -e "s/@@CRYPTO_KEY@@/$key/" newsys-config.inc.in > newsys-config.inc

echo "done"
echo "+--------------------------------------"
echo
echo "+--------------------------------------"
echo -n "| Generating installer config file... "

key_type=$(php -q detect-key-type.php)

password_len=0
crypt_key_len=0

case in $key_type
	blowfish)
		password_len=60
		crypt_key_len=22
		;;
	md5)
		password_len=32
		crypt_key_len=12
		;;
	ext_des)
		password_len=13
		crypt_key_len=9
		;;
	default)
		password_len=13
		crypt_key_len=2
		;;
esac

sed	-e "s/@@PASSWORD_LEN@@/$password_len/" \
	-e "s/@@CRYPT_KEY_LEN@@/$crypt_key_len/" config.sh.in > config.sh

echo "done"
echo "+--------------------------------------"
echo
echo "You must now further configure settings."
echo "Press ^Z now to stop this script execution,"
echo "edit config.sh, and then resume execution"
echo "and press [Enter]..."

read

./config.sh

echo "+--------------------------------------"
echo -n "| Generating SQL file... "

sed	-e "s/@@USERNAME@@/$NEWSYS_USERNAME/g"			\
	-e "s/@@PASSWORD@@/$NEWSYS_PASSWORD/g"			\
	-e "s/@@EMAIL@@/$NEWSYS_EMAIL/g"			\
	-e "s/@@PASSWORD_LEN@@/$NEWSYS_PASSWORD_LEN/g"		\
	-e "s/@@CRYPT_KEY_LEN@@/$NEWSYS_CRYPT_KEY_LEN/g"	\
	create.sql.in > create.sql

echo "done"
echo "+--------------------------------------"
echo
echo "Setup is complete. The last few steps you will need"
echo "to take are:"
echo
echo "	1) Run the newly-generated create.sql on your RDBMS"
echo "	2) Copy templates-dist and newsys-config.inc to a"
echo "		place outside of your Web-accessible tree and"
echo "		add this path to PHP's include_path directive"
echo "	3) Create a cache directory with permissions suitable"
echo "		for files to be written to as the user PHP runs"
echo "		as"
echo "	4) Configure newsys-config.inc to your likings"

exit 0
