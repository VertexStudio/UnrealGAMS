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
curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | sudo bash
git lfs install
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
