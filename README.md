## Setup Instructions

### Install UnrealEngine

```
git clone -b 4.21 git@github.com:EpicGames/UnrealEngine.git
cd UnrealEngine
./Setup.sh
./GenerateProjectFiles.sh
make
```

For more info visit [Building on Linux](https://wiki.unrealengine.com/Building_On_Linux).

### Git Large File Storage (Git LFS)

```
sudo apt-get install git-lfs
git-lfs install
```

For more info visit [Git LFS](https://git-lfs.github.com/)

## Setup UnrealGAMS (this repo)

Clone repository:

```
git clone git@github.com:VertexStudio/UnrealGAMS.git
```

Environment variables:

```
export UE4_ROOT=/path/to/UE4.21
export UE4_GAMS=/path/to/UnrealGAMS
```

Generate project files and build

```
cd $UE4_GAMS
./generate.sh
./build.sh
```

Run:

```
./run_editor.sh
```

# Using Quadcopter OSC

In UnrealEngine open and play map `Robots/Quadcopter/Maps/MAP_Quadcopter`.

Download [Open Stage Control](https://osc.ammd.net/)

```
./open-stage-control --load $UE4_GAMS/Config/UnrealGAMS-OSC.json -d --send 127.0.0.1:5555 --port 8000
```

Or use your favorite OSC controller.

### Inputs:

XY velocity:
`/agent/0/velocity/xy [f32,f32]`

Z velocity:
`/agent/0/velocity/z [f32]`

Yaw:
`/agent/0/velocity/yaw [f32]`

### Outputs:

Vector3 Position: `/agent/0/pos [f32,f32,f32]`

Quaternion Rotation: `/agent/0/rot [f32,f32,f32]`

## Spawning Quadcopters

Sending OSC message to `/spawn/quadcopter` address. The message must be a blob containing a string serialized JSON structure like the following:

```
{
  "id": 0,
  "port": 8000,
  "location": {
    "x": 0, "y": 0, "z": 0
  },
  "rotation": {
    "x": 0, "y": 0, "z": 0
  }
}
```
## Build a release

Execute the `cook.sh` script.

Usage:
```
cook.sh [RELEASE_VERSION] [PROJECT_NAME] [MAPS_TO_COOK] [WINDOW_RESOLUTION]
```
- **RELEASE_VERSION:** Version of the release. Default `1.0`.
- **PROJECT_NAME:** Name of the UE4 project. By default, it takes the name of the current folder.
- **MAPS_TO_COOK:** List of maps to cook. Each map name must be separted by a `+` character. Example: `GrassMountains+Desert`.
- **WINDOW_RESOLUTION:** Dimensions of the window. Default dimensions `1024x576`.

## Create a DLC

1. Copy new content in the `Content` directory of the `LoadMap` plugin.
2. Execute the `cook_dlc.sh` script.

Usage:
```
cook_dlc.sh [BASED_ON_RELEASE_VERSION] [PROJECT_NAME] [DLC_NAME]
```
- **BASED_ON_RELEASE_VERSION:** Release version the DLC will be based on. Default `1.0`.
- **PROJECT_NAME:** Name of the UE4 project. By default, it takes the name of the current folder.
- **DLC_NAME:** Name of the generated `.pak` file (DLC). Default value is `DLC_based_on_{version it's based on}_{timestamp}`.
