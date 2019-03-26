#!/usr/bin/env bash

set -o errexit

BASED_ON_RELEASE_VERSION=$1 # Default value is "1.0"
PROJECT_NAME=$2 # By default it takes the name of the current folder
DLC_NAME=$3 # Default value is "DLC_based_on_{version it's based on}_{timestamp}"

# Required env variables
PROJECT_ROOT=$UE4_GAMS
UE4_PATH=$UE4_ROOT

if [ -z "$PROJECT_ROOT" ] || [ -z "$UE4_PATH" ]
then
  echo "ERROR! Absolute path to UE4 and/or to project are missing. "
  echo "Required environmental variables are not defined or have different names!"
  echo "Modify this script (Lines 8-9) to point to the desired variables."
  exit 1
fi

if [ -z "$BASED_ON_RELEASE_VERSION" ]
then
  BASED_ON_RELEASE_VERSION=1.0
fi

if [ -z "$PROJECT_NAME" ]
then
  PROJECT_NAME="${PWD##*/}"
  echo "======="
  echo "Project name not specified!"
  echo "Default name is: $PROJECT_NAME"
fi

if [ -z "$DLC_NAME" ]
then
  DLC_NAME="DLC_based_on_${BASED_ON_RELEASE_VERSION}_$(date +%s)"
fi

"$UE4_PATH"/Engine/Build/BatchFiles/RunUAT.sh -ScriptsForProject="$PROJECT_ROOT"/"$PROJECT_NAME".uproject BuildCookRun \
-project="$PROJECT_ROOT"/"$PROJECT_NAME".uproject -noP4 -clientconfig=Development -serverconfig=Development -nocompileeditor \
-ue4exe=UE4Editor -utf8output -platform=Linux -targetplatform=Linux -build -cook -map= -unversionedcookedcontent -pak -dlcname=LoadMap \
-DLCIncludeEngineContent -basedonreleaseversion=$BASED_ON_RELEASE_VERSION -stagebasereleasepaks -stage -compressed

mv "$PROJECT_ROOT"/Plugins/LoadMap/Saved/StagedBuilds/LinuxNoEditor/"$PROJECT_NAME"/Plugins/LoadMap/Content/Paks/LinuxNoEditor/LoadMap"${PROJECT_NAME}"-LinuxNoEditor.pak \
   "$PROJECT_ROOT"/Plugins/LoadMap/Saved/StagedBuilds/LinuxNoEditor/"$PROJECT_NAME"/Plugins/LoadMap/Content/Paks/LinuxNoEditor/"${DLC_NAME}".pak

cp "$PROJECT_ROOT"/Plugins/LoadMap/Saved/StagedBuilds/LinuxNoEditor/"$PROJECT_NAME"/Plugins/LoadMap/Content/Paks/LinuxNoEditor/"${DLC_NAME}".pak \
   "$PROJECT_ROOT"/out/LinuxNoEditor/"$PROJECT_NAME"/Content/Paks/

DLC_SIZE=$(du -h "$PROJECT_ROOT"/Plugins/LoadMap/Saved/StagedBuilds/LinuxNoEditor/"$PROJECT_NAME"/Plugins/LoadMap/Content/Paks/LinuxNoEditor/"${DLC_NAME}".pak)

rm -r "$PROJECT_ROOT"/Plugins/LoadMap/Saved

cat <<EOT

SUMMARY
===============
Script executed with params: cook_dlc.sh $BASED_ON_RELEASE_VERSION $PROJECT_NAME $DLC_NAME
Based on release version: $BASED_ON_RELEASE_VERSION
Project name: $PROJECT_NAME
DLC name: $DLC_NAME
DLC size and path: $DLC_SIZE
Project root path: $PROJECT_ROOT
UE4 root path: $UE4_PATH
=======

EOT
