#!/usr/bin/env bash

set -o errexit

RELEASE_VERSION=$1 # Default value is "1.0"
PROJECT_NAME=$2 # By default it takes the name of the current folder
MAPS_TO_COOK=$3 # Each map name must be separated by a "+"
WINDOW_RESOLUTION=$4 # Default dimensions are "1024x576"

# Required env variables
PROJECT_ROOT=$UE4_GAMS
UE4_PATH=$UE4_ROOT

if [ -z "$PROJECT_ROOT" ] || [ -z "$UE4_PATH" ]
then
  echo "ERROR! Absolute path to UE4 and/or to project are missing. "
  echo "Required environmental variables are not defined or have different names!"
  echo "Modify this script (Lines 11-12) to point to the desired variables."
  exit 1
fi

if [ -z "$RELEASE_VERSION" ]
then
  RELEASE_VERSION=1.0
fi

if [ -z "$PROJECT_NAME" ]
then
  PROJECT_NAME="${PWD##*/}"
  echo "======="
  echo "Project name not specified!"
  echo "Default name is: $PROJECT_NAME"
fi

if [ -z "$MAPS_TO_COOK" ]
then
  MAPS_TO_COOK=Sandbox
fi

if [ -z "$WINDOW_RESOLUTION" ]
then
  WINDOW_RESOLUTION="1024x576"
fi

IFS="x" read -ra RESOLUTION <<< "$WINDOW_RESOLUTION"
RES_X=${RESOLUTION[0]}
RES_Y=${RESOLUTION[1]}

echo "======="
echo "Creating release version $RELEASE_VERSION"
echo "======="

"$UE4_PATH"/Engine/Build/BatchFiles/RunUAT.sh -ScriptsForProject="$PROJECT_ROOT"/"$PROJECT_NAME".uproject BuildCookRun -project="$PROJECT_ROOT"/"$PROJECT_NAME".uproject \
-noP4 -clientconfig=Development -serverconfig=Development -nocompileeditor -ue4exe=UE4Editor -utf8output -platform=Linux -targetplatform=Linux \
-build -cook -map=$MAPS_TO_COOK -unversionedcookedcontent -pak -createreleaseversion=$RELEASE_VERSION -compressed -stage -package \
-stagingdirectory="$PROJECT_ROOT"/out -cmdline= -Messaging

echo "======="
echo "Modifying ${PROJECT_NAME}.sh to open with a window resolution of $WINDOW_RESOLUTION"
echo "======="

REPLACEMENT="-windowed -ResX=$RES_X -ResY=$RES_Y \"\$@\""

NEW_CONTENT=$(sed "s/\$@/$REPLACEMENT/" "${PROJECT_ROOT}"/out/LinuxNoEditor/"${PROJECT_NAME}".sh)

echo "$NEW_CONTENT" > "${PROJECT_ROOT}"/out/LinuxNoEditor/"${PROJECT_NAME}".sh

cat <<EOT

SUMMARY
===============
Script executed with params: cook.sh $RELEASE_VERSION $PROJECT_NAME $MAPS_TO_COOK $WINDOW_RESOLUTION
Release version: $RELEASE_VERSION
Project name: $PROJECT_NAME
Maps to cook: $MAPS_TO_COOK
Window resolution: $WINDOW_RESOLUTION
Project root path: $PROJECT_ROOT
UE4 root path: $UE4_PATH
Release: ${PROJECT_ROOT}/out/LinuxNoEditor/${PROJECT_NAME}.sh
=======

EOT

