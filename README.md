ogalib
======

This is the OGA™ library for C++, and plugins for Unreal Engine and Unity.  This project contains the open-source C++ code and plugins to interact with the OGA™Hub API to download aseets associated with a player's blockchain wallet.

In order to make things easier to learn, this project also includes the Prime Engine.  This is a lightweight game engine that has the ability to load popular asset types such as PNG images, GLTF and FBX models, and JSON data.

Here is a description of all of the Visual Studio projects inside this repo.

- ogalib
  - Static Library
  - This is the minimal amount of code needed to connect to the OGA™Hub API and download raw asset data and JSON data
- Prime
  - Static Library
  - This is the lightweight game engine that can load popular asset types like PNG, GLTF, and FBX.
- Hello Prime
  - Application
  - A simple application built on Prime to load a font and display a "Hello, World!" message.
- Prime Demo
  - Application
  - A more involved application which loads and displays 3 asset types: images, 2D skeletal characters, and 3D animated models.
- Asset Browser
  - Application
  - The most involved application which interacts with OGA™Hub API to view all uploaded assets that will eventually be minted to the blockchain.

OGA™Hub API
===========

The Asset Browser application can be used to browse all of the assets that were uploaded to OGA™Hub via the API.  For more information on how to upload your own assets to test, visit https://ogahub.com/API/Docs.  From there you can read the docs and perform live demos of the API calls.

Visit the [OGA™ Developer Wiki](https://github.com/NineTalesDigital/ogalib/wiki/OGA%E2%84%A2-Developer-Wiki) to follow a tutorial about getting started right away.
