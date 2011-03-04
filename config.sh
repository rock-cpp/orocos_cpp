#!/bin/sh

# Author: Alexander Duda, Thomas Roehr, thomas.roehr@dfki.de
# This script sets up a git repository based on an existing template project

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
        find . -type f -exec sed -i 's/dummyproject/'$PACKAGE_SHORT_NAME'/' {} \;
        # also rename the relevant files
        find . -type f -name '*dummyproject*' | while read path; do
            newpath=`echo $path | sed "s/dummyproject/$PACKAGE_SHORT_NAME/"`
            mv $path $newpath
        done

	PKG_DESC=
	PKG_LONG_DESC=
	PKG_AUTHOR=
	PKG_EMAIL=
	PKG_URL=
	PKG_SCM=
	PKG_SCM_URL=


	# TODO 
	PKG_LICENSE=
	PKG_LOGO_URL=
	PKG_DEPENDENCIES=
	PKG_CFLAGS=
	PKG_LFLAGS=

	# File storing the package information
	MANIFEST="manifest.xml"
	echo "------------------------------------------"
	echo "We require some information to update the $MANIFEST"
	echo "------------------------------------------"
	echo "Brief package description (Press ENTER when finished):"
	read PKG_DESC

	echo "Long description: "
	read PKG_LONG_DESC
	# REMOVE http:// since it is a bit messy to pass it through shell expansion
	PKG_LONG_DESC=`echo $PKG_LONG_DESC | sed 's/http:\/\///g'`

	echo "Author: "
	read PKG_AUTHOR

	echo "Author email: "
	read PKG_EMAIL

	echo "Url (optional):"
	read PKG_URL
	# REMOVE http:// since it is a bit messy to pass it through shell expansion
	PKG_URL=`echo $PKG_URL | sed 's/http:\/\///g'`

	# Yes, we only support git as SCM
	PKG_SCM="git"
	echo "Enter your dependencies as a comma separated list. Press ENTER when finished:"
	read PKG_DEPENDENCIES

	# But if there is svn or mercurial support ever
	# we can already prompt the user ;)
	#echo "Version control system [git|svn|mercurial]: " 
	#echo 1: git
	#echo 2: svn
	#echo 3: mercury

	#while [ 1 ]
	#        do 
	#	echo "Select from list:"
	#	read PKG_SCM
	#	case $PKG_SCM in
	#	1) PKG_SCM="git"
	#	   break
	#	   ;;
	#	2) PKG_SCM="svn"
	#	   break
	#	   ;;
	#	3) PKG_SCM="mercury"
	#           break
	#	   ;;
	#	esac
	#done

	sed -i "s/\(PROJECT_DESCRIPTION\ \).*/\1\"$PKG_DESC\")/" CMakeLists.txt
	sed -i "s/dummy-brief-desc/$PKG_DESC/" $MANIFEST
	sed -i "s/dummy-long-desc/$PKG_LONG_DESC/" $MANIFEST
	sed -i "s/dummy-author/$PKG_AUTHOR/" $MANIFEST
	sed -i "s/dummy-email/$PKG_EMAIL/" $MANIFEST
	sed -i "s/dummy-url/http:\/\/$PKG_URL/" $MANIFEST
	sed -i "s/dummy-version-control/$PKG_SCM/" $MANIFEST
	sed -i "s/dummy-version-control-url/$PKG_SCM_URL/" $MANIFEST
	sed -i "s/dummy-license/$PKG_LICENSE/" $MANIFEST
	sed -i "s/dummy-logo-url/http:\/\/$PKG_LOGO_URL/" $MANIFEST

	# Enter list of dependencies if there are any
	if [ "$PKG_DEPENDENCIES" != "" ]
		then
		# Replace comma ','
		PKG_DEPENDENCIES=`echo $PKG_DEPENDENCIES | sed "s/,/ /g"`
		for dep in ${PKG_DEPENDENCIES}
			do
			# Use the <!--DEPEND-ENTRY--> as a hook for subsequent replacements
			# replace with <depend package="pkgname"> 
			sed -i "s/<\!--DEPEND-ENTRY-->/<depend package=\"${dep}\"\ \/>\n<\!--DEPEND-ENTRY-->/" $MANIFEST
		done
	fi

	sed -i "s/dummy-cflags/$PKG_CFLAGS/" $MANIFEST
	sed -i "s/dummy-lflags/$PKG_LFLAGS/" $MANIFEST
fi
# end of CMAKE-TEMPLATE-ADAPTION

#delete setup script
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

