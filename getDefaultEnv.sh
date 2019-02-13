#!/usr/bin/env bash

set -o errexit

# UE4 environment
ENVIRONMENT_URL=https://github.com/VertexStudio/UnrealGAMS/releases/download/v0.1/GrassMountains.zip

if [ ! -d "$(pwd)/Content" ]; then
    echo "ERROR! You must execute command in the root of UE4 project."
    echo "If you are in the right place, make sure the 'Content' folder exists."
    echo "Exiting."
    exit
fi

echo "Downloading..."
wget "$ENVIRONMENT_URL" -O environment.zip
mkdir -p ./Content/Environments
mv ./environment.zip ./Content/Environments
pushd ./Content/Environments
unzip environment.zip
rm environment.zip
popd

echo
echo -n $'\u2714'; echo " Environment successfully added."
echo