#!/usr/bin/env bash

RELEASE_VERSION=$1
WINDOW_RESOLUTION=$2
PROJECT_NAME=UnrealGAMS

if [ -z "$RELEASE_VERSION" ]
then
  RELEASE_VERSION=1.0
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

"$UE4_ROOT"/Engine/Build/BatchFiles/RunUAT.sh -ScriptsForProject="$UE4_GAMS"/$PROJECT_NAME.uproject BuildCookRun -project="$UE4_GAMS"/$PROJECT_NAME.uproject \
-noP4 -clientconfig=Development -serverconfig=Development -nocompileeditor -ue4exe=UE4Editor -utf8output -platform=Linux -targetplatform=Linux \
-build -cook -map=Sandbox+MAP_Quadcopter -unversionedcookedcontent -pak -createreleaseversion=$RELEASE_VERSION -compressed -stage -package \
-stagingdirectory="$UE4_GAMS"/out -cmdline= -Messaging

echo "======="
echo "Modifying ${PROJECT_NAME}.sh to open with a window resolution of $WINDOW_RESOLUTION"
echo "======="

REPLACEMENT="-windowed -ResX=$RES_X -ResY=$RES_Y \"\$@\""

NEW_CONTENT=$(sed "s/\$@/$REPLACEMENT/" "${UE4_GAMS}"/out/LinuxNoEditor/${PROJECT_NAME}.sh)

echo "$NEW_CONTENT" > "${UE4_GAMS}"/out/LinuxNoEditor/${PROJECT_NAME}.sh

