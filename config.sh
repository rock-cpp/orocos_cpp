#!/bin/sh

# Author: Alexander Duda, Thomas Roehr, thomas.roehr@dfki.de
# This script sets up a git repository based on an existing template project

set -e

#Check for name of project
# Extract directory for config.sh
SCRIPT=$0
SCRIPT_DIR=`dirname $SCRIPT`
# Retrieve Absolute dir from subshell
DIR=`cd "$SCRIPT_DIR" && pwd`
# Extract Projectname
PACKAGE_DIR_NAME=`basename $DIR`

# If no arguments are given or help is requested
if [ "$1" = "-h" ] || [ "$1" = "--help" ]
	then echo "usage: $0 [<package-name>]"
	echo "This script prepares a cmake-template for usage. It requests required"
	echo "information from the user when needed."
	echo "Note, that if no package name is given, the name of the parent directory"
	echo " of this script (${PACKAGE_DIR_NAME}) applies."
	echo "as project name."
	exit 0
fi

if [ "$1" != "" ]; then
	PACKAGE_SHORT_NAME=$1
else
	PACKAGE_SHORT_NAME=$PACKAGE_DIR_NAME
fi

echo "Do you want to start the configuration of the cmake project: ${PACKAGE_SHORT_NAME}"

# Check and interprete answer of "Proceed [y|n]"
ANSWER=""
until [ "$ANSWER" = "y" ] || [ "$ANSWER" = "n" ] 
do
	echo "Proceed [y|n]"
	read ANSWER
	ANSWER=`echo $ANSWER | tr "[:upper:]" "[:lower:]"`
done

if [ "$ANSWER" = "n" ]
	then echo "Aborted."
	exit 0
fi

# Change into the operation directory
cd $DIR

# Select package type
PACKAGE_TYPE="CMAKE"

# CMAKE-TEMPLATE-ADAPTION
if [ $PACKAGE_TYPE = "CMAKE" ]; then
	# removing git references to prepare for new check in
	rm -rf .git

	# replace dummyproject with projectname in the files
        find . -type f ! -name 'config.sh' -exec sed -i 's#dummy_project#'$PACKAGE_SHORT_NAME'#' {} \;
        # also rename the relevant files
        find . -type f -name '*dummy_project*' | while read path; do
            newpath=`echo $path | sed "s#dummy_project#$PACKAGE_SHORT_NAME#"`
            mv $path $newpath
        done
	
	# Configure the manifest
	sh config_manifest.sh
fi
# end of CMAKE-TEMPLATE-ADAPTION

#delete setup scripts
rm config_manifest.sh
rm config.sh

# creating a new git project --add adapted dummy-project contents and commit
git init --shared=0770
git add .
git commit -m "Initial commit"


if [ "$PACKAGE_DIR_NAME" != "$PACKAGE_SHORT_NAME" ]; then
	cd ..
	mv $PACKAGE_DIR_NAME $PACKAGE_SHORT_NAME
	cd $PACKAGE_SHORT_NAME
	echo "WARNING: package directory name changed to given package name: ${PACKAGE_SHORT_NAME}"
	echo "Please 'cd' out of the directory once"
fi

echo "Done."

