#!/usr/bin/env bash
echo "----------------------"
$UE4_ROOT/GenerateProjectFiles.sh -project=$UE4_GAMS/UnrealGAMS.uproject -game -engine -Makefile -vscode
