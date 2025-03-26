
# UE-LASAA
This project contains a plug-and-play Unreal Engine 5 plugin for Large-area Spatially Aligned Anchors (LASAA). 
The current version only supports Meta headsets. Other headsets can be integrated by changing the SDK and the code inside `LASAA/Source/LASAA/Private/Anchor.cpp` and `LASAA/Source/LASAA/Public/Anchor.h`.


## Examples Videos

### Approach
[![Large Area Spatially Aligned Anchors](https://img.youtube.com/vi/2xleN8whWSI/0.jpg)](https://www.youtube.com/watch?v=2xleN8whWSI)

### Stereopsia 2024
[![Stereopsia 2024](https://img.youtube.com/vi/TntOdb3ymrY/0.jpg)](https://www.youtube.com/watch?v=TntOdb3ymrY)

### On-site Visualizing of Building Information Models (BIM)
[![Large Area AR for BIM visualisations and interactions](https://img.youtube.com/vi/CEKtPZjClpc/0.jpg)](https://www.youtube.com/watch?v=CEKtPZjClpc)



## Installation

- Install the OculusXR plugin if developing for Meta devices. The following link contains a tutorial on how to set it up: https://developer.oculus.com/documentation/unreal/unreal-quick-start-guide-quest/.
- Create a new Unreal project following the Meta documentation.
- Enable the `MetaXR` and the `OSC` plugin.
- In your project folder, create a folder called `Plugins` 
- Download this repository as a zip file.
- Copy to a `LASAA` folder in the `Plugins` folder.
- Regenerate project files, restart your project and the plugin is automatically compiled. 
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

> [!NOTE]  
> At the moment the tracking is dependent on using an external camera to initialise the markers positions.

## About
![](/img/edm_logo.png) &nbsp;&nbsp;&nbsp;
![](/img/Max-R_Logo.png)

LASAA is a development by [Hasselt University](https://www.uhasselt.be/), [Expertise Centre for Digital Media](https://www.uhasselt.be/en/instituten-en/expertise-centre-for-digital-media) in the scope of the EU funded Projects [MAX-R](https://max-r.eu/) (101070072) and funds on behalf of Hasselt University.

## Funding

![Funded by EU](/img/EN_Co-fundedbytheEU_RGB_POS.png)

This project has received funding from the European Union's Horizon Europe Research and Innovation Programme under Grant Agreement No 101070072 MAX-R.


## License
LASAA is a open-sorce development by Hasselt University Expertise Centre for Digital Media  
The client is licensed under [MIT](LICENSE). See [License file](LICENSE) for more details.
