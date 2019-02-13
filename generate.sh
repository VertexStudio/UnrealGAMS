#!/usr/bin/env bash
echo "Generating UnrealGAMS project files"
echo "----------------------"
"$UE4_ROOT"/GenerateProjectFiles.sh -project="$UE4_GAMS"/UnrealGAMS.uproject -game -engine -Makefile -vscode
