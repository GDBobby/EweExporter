# This program converts from the FBX file format to the EWE file format.
Overall, this program is not very robust. It's been working, and I've been settling while I have bigger fish to fry.

## Dependencies
https://github.com/assimp/assimp

EweExporter does not carry over textures. Textures will need to be extracted from the source file, separately.

## Two options exists for moving forward with this functionality
* writing a blender plugin (python) to replace this functionality. Probably the best option.
* making this program much more robust. That would include
  * reading bone count on import so that the source doesn't need to be changed per import file
  * probably some kind of GUI, or command line usage
  * support for more data types than just FBX

## The main perks of the EWE file format, as opposed to using an existing 3D model format like GLB/GLTF, or FBX
* supports instancing skinned meshes, with an independent animation state each, out the box.
  * I've heard this is impossible with GLB/GLTF.
  * It is MORE difficult with the native interpreted format from FBX with Assimp, and this library largely serves to do that data outside of the runtime of the game.
* Partial animations, while not storing bones that aren't utilized in the partial animation. Every animation can be played at once, or two or three and so on. 
  * For example, in Eight Winds I use this to play the attack animation, which is the right arm, and the block animation which is on the left arm, and dash/walk/jump on the legs all asynchronously.
* Baked animations, no interpolation necessary. (this is situational, could be a bad fit for your needs). You can set the animation speed in the source, my needs required 250 frames per second.
* Vertex/Index Data is preprocessed for Vulkan. Load to memory, load to GPU, delete. Programs that import the EWE File Format will not need to do any additional computation.

## Downsides of the File Format -
* Model Data is not very space efficient. I believe on average, the file size is 2x the FBX format.
* Baked Animation. (situationally good or bad). GENERALLY speaking, this is going to be bad for any game that isn't a fighting game. If you don't require PREDICTABLE and REACTABLE animation states, baked animations are probably bad.

## Import example
This file format was created specifically for the Eight Winds Engine.
https://github.com/GDBobby/Eight-Winds-Engine/blob/main/src/SkeletonHandlerBase.cpp


## Attributions
https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation - the FBX import code is largely based on this
