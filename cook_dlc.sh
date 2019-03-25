#!/usr/bin/env bash

BASED_ON_RELEASE_VERSION=$1
DLC_NAME=$2
PROJECT_NAME=UnrealGAMS

if [ -z "$BASED_ON_RELEASE_VERSION" ]
then
  BASED_ON_RELEASE_VERSION=1.0
fi

if [ -z "$DLC_NAME" ]
then
  DLC_NAME="DLC_based_on_$BASED_ON_RELEASE_VERSION"
fi

echo "======="
echo "Creating DLC with name: $DLC_NAME".
echo "Based on release version: $BASED_ON_RELEASE_VERSION"
echo "======="

"$UE4_ROOT"/Engine/Build/BatchFiles/RunUAT.sh -ScriptsForProject="$UE4_GAMS"/$PROJECT_NAME.uproject BuildCookRun \
-project="$UE4_GAMS"/$PROJECT_NAME.uproject -noP4 -clientconfig=Development -serverconfig=Development -nocompileeditor \
-ue4exe=UE4Editor -utf8output -platform=Linux -targetplatform=Linux -build -cook -map=GrassMountains -unversionedcookedcontent -pak -dlcname=LoadMap \
-DLCIncludeEngineContent -basedonreleaseversion=$BASED_ON_RELEASE_VERSION -stagebasereleasepaks -stage -compressed

mv "$UE4_GAMS"/Plugins/LoadMap/Saved/StagedBuilds/LinuxNoEditor/$PROJECT_NAME/Plugins/LoadMap/Content/Paks/LinuxNoEditor/LoadMapUnrealGAMS-LinuxNoEditor.pak \
   "$UE4_GAMS"/Plugins/LoadMap/Saved/StagedBuilds/LinuxNoEditor/$PROJECT_NAME/Plugins/LoadMap/Content/Paks/LinuxNoEditor/${DLC_NAME}.pak

cp "$UE4_GAMS"/Plugins/LoadMap/Saved/StagedBuilds/LinuxNoEditor/$PROJECT_NAME/Plugins/LoadMap/Content/Paks/LinuxNoEditor/${DLC_NAME}.pak \
   "$UE4_GAMS"/out/LinuxNoEditor/UnrealGAMS/Content/Paks/

rm -r "$UE4_GAMS"/Plugins/LoadMap/Saved