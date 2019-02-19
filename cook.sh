$UE4_ROOT/Engine/Build/BatchFiles/RunUAT.sh -ScriptsForProject=$UE4_GAMS/UnrealGAMS.uproject BuildCookRun -nocompileeditor -nop4 \
-project=$UE4_GAMS/UnrealGAMS.uproject -cook -stage -archive \
-archivedirectory=$UE4_GAMS/out/ -package -clientconfig=Development \
-ue4exe=UE4Editor -nodebuginfo -targetplatform=Linux -build -utf8output -pak
