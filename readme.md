
# UE-LASAA
This project contains a plug-and-play Unreal Engine 5 plugin for Large-area Spatially Aligned Anchors (LASAA). 
The current version only supports Meta headsets. Other headsets can be integrated by changing the SDK and the code inside `LASAA/Source/LASAA/Private/Anchor.cpp` and `LASAA/Source/LASAA/Public/Anchor.h`.

## Installation

- Create a blank Unreal project for mobile or use an existing one (as always a blank one is the easiest, since there are no interfering dependencies). 
- Install the OculusXR plugin if developing for Meta devices. The following link contains a tutorial on how to set it up: https://developer.oculus.com/documentation/unreal/unreal-quick-start-guide-quest/.
- Enable the `MetaXR` and the `OSC` plugin.
- In your project folder, create a folder called `Plugins` 
- Download this repository as a zip file.
- Copy the `LASAA` folder to the `Plugins` folder.
- Restart your project and the plugin is automatically compiled. 
- Change Gamemode to `AnchorGameMode`.
- In the MetaXR plugin settings, enable anchor support and passthrough support and add your headset as a supported Meta Quest device. 
- Make sure to add the `Prep` and `Operational` levels to your packaging in project settings. 
- Set default map to `Prep`.

## Usage
In the content browser, find the `LASAA (C++) Content` folders. You can find all content the plugin provides. 

### Anchor
The `Anchor` class contains functions to create a spatial anchor using the OculusXR plugin. If you are developing for another platform, you need to change these function calls to the appropiate SDK.

### Maps
Inside `Maps`, two startup Levels are given: `Prep` and `Operational`. The easiest way is to copy these two levels and add your content to them. For both, the actor called `AnchorsConfig` contains all configuration parameters necessary. 

`Prep`:
- Can be used as is. 

`Operational`:
- Add your content under the `AlignedActor` actor. 

### OSC receiver
The OSC receiver by default listens at the local IP of the device and port 8000. These settings can be changed inside the `AnchorPlacer` actor.

## Usage
By default the Levels are configured to work only with the right controller:
- Press `B` to switch Levels from `Prep` to `Operational`.
- Press `grip trigger` to switch between the anchor calibration and controller calibration parameters (only for evaluation).
- Press `a` while selecting an anchor to remove it.


**Prep Level:**
- Press `index trigger` to place an anchor (make sure the external tracking method is working and sending packages over OSC to the /pose endpoint).
- Hold and release `a` to remove all anchors.
- Hold and release `grip trigger` to calibrate the external camera to the headset using the placed anchors.

**Operational Level:**
- Press `index trigger` to save this pose together with its external pose to a file (only for evaluation, and make sure the external tracking method is working)
- Press `right thumbstick` once to change the opacity of the mesh.
- Hold and release `right thumbstick` to change the interpolation mode from static to weighted and vice versa.



