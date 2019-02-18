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

### Outputs:

Vector3 Position: `/agent/0/pos/xyz [f32,f32,f32]`

Quaternion Rotation: `/agent/0/pos/xyz [f32,f32,f32,f32]`
