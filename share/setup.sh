#!/bin/sh

#####################################################################

echo -n "Generating encryption key"

key=$(nice -n 15 php -q gen-key.php | sed 's/\(["\\]\)/\\\1/g')

echo " done."
echo "------------------------------------------------------------------------"

#####################################################################

echo -n "Building configuration file: "

touch newsys-config.inc

chmod 600 newsys-config.inc

sed "s/@@CRYPT_KEY@@/$key/g" newsys-config.inc.in > newsys-config.inc

echo "done."
echo "------------------------------------------------------------------------"

#####################################################################

echo "You will now be prompted to enter information about the"
echo "first administrator account for this newsys installation."
echo

username=

while [ "$username" = "" ]; do
	echo -n "Username: "
	read username

	if [ $(echo $username | grep '[^a-zA-Z0-9_-]') ]; then
		username=
		echo "Please enter an alpha-numeric username."
	fi
done

password=
password2=

while [ "$password" = "" ]; do
	echo -n "Password (will not echo): "
	stty -echo
	read password
	stty echo
	echo

	grep "['\"\\]" <<EOF
$password
EOF

	if [ $? = 0 ]; then
		password=
		echo "Please enter a password with none of the following characters: '\"\\"
	fi

	echo -n "Verify password: "
	stty -echo
	read password2
	stty echo
	echo

	if [ "$password" != "$password2" ]; then
		password=
		password2=
		echo "Your passwords do not match."
	fi
done

email=

echo -n "E-mail Address: "
read email

echo
echo "You will now be asked to select the length of"
echo "passwords to be stored in the database."
echo
echo "If your system supports.............Choose"
echo
echo "    Blowfish........................60"
echo "    MD5.............................32"
echo "    Other...........................13"
echo
echo -n "Length: "
read password_len

if [ $password_len -lt 1 ]; then
	password_len=13
fi

#####################################################################

echo -n "Building SQL dumpfile: "

touch create.sql

chmod 600 create.sql

sed	-e "s/@@USERNAME@@/$username/g"	\
	-e "s/@@PASSWORD@@/$password/g"		\
	-e "s/@@EMAIL@@/$email/g"		\
	-e "s/@@PASSWORD_LEN@@/$password_len/g"	\
	create.sql.in > create.sql

echo " done."
echo "------------------------------------------------------------------------"

#####################################################################

echo "The following files have been created in the current directory:"
echo
echo "	newsys-config.inc"
echo "	create.sql"
echo
echo "These files contain sensitive information. Make sure that only"
echo "users that should be reading them are and no one else."
echo
echo "Consult doc/INSTALL for details on proceeding with the"
echo "installation."

#####################################################################

exit 0
