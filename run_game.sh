#!/usr/bin/env bash
"$UE4_ROOT"/Engine/Binaries/Linux/UE4Editor "$UE4_GAMS"/UnrealGAMS.uproject -windowed -game -ResX=1024 -ResY=576 $@
