#!/bin/sh
# Newsys config script

NEWSYS_USERNAME=admin_account_username
NEWSYS_PASSWORD=admin_account_password
NEWSYS_EMAIL=admin_account_email_address

# Don't edit these
NEWSYS_PASSWORD_LEN=@@PASSWORD_LEN@@
NEWSYS_CRYPT_KEY_LEN=@@CRYPT_KEY_LEN@@

# Ignore this line
export NEWSYS_USERNAME NEWSYS_PASSWORD NEWSYS_PASSWORD_LEN NEWSYS_EMAIL NEWSYS_CRYPT_KEY_LEN

exit 0
