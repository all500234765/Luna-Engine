# Luna-Engine
 ![Icon](Misc/Luna150_Wide.png) 
 
 Luna Engine is DirectX 11 based engine that i develop. Main reason why is that i want to learn more and try it all by myself

I made trello panel, so you can see what i want to do, what i already done and what i am working on atm.
https://trello.com/b/T8T6vkBN/directx-11-engine-2019

# Version 0.1.101
* Screen-Space Ambient Occlusion
* Memory usage functions
    * GPU / CPU
        * Current Usage
        * Avaliable For Reservation
        * Budget
        * Current Reservation
* Working on HDR/WCG Support
* Bokeh (WIP; It's very unstanble atm)
* Depth Of Field (WIP; Must remove linear interpolation and do it with rsqrt)
* Graphs to see performance of various things
* Timer class got an update. Now you can specify several more settings
    * Log values to the console?
    * Lambda or any other type of function that will operate on destruction of Timer
* PlotData(Windows project) structure for easier work with ImGui::Plot
* Switched to inverse depth buffer (This means that only Windows project is working properly atm)
![Screen-Space Ambient Occlusion](Misc/SSAO.png)

# Version 0.1.080
* Million particles test

![1kk Particles](Misc/1kkParticles.gif)

# Requirements
* MSVS 2017+ 
   * 2019+ might ask about upgrading to newer tools, it's should still work. I haven't checked it in a while
* Platform Toolset v141+
* x86 / x64-bit system
* Windows SDK Version - 10.0.17763+
* Videocard with DX 11 support
* Target platform is Windows 10
   * It might not work on older versions
   * I plan on adding DX 12 with DXR support, so stay tuned!

# Features
* x86 / x64 builds
* 3D Particles
* 2D Particles
* Semi-Automated generartion of
    * Structured Buffer with or without UAV
    * Constant Buffer
    * Vertex Buffer
    * Index Buffer
    * Shaders
    * IA/OM/... States
    * And more!
* Render Buffers
* Model loading (avaliable all formats from Assimp)
* 2D Textures
* Cube Maps
* Physics Engine
    * Avaliable Colliders:
         * Plane
         * Sphere
         * AABB
* Audio Engine
    * Loading of WAV files
    * Usage of XInput2 library
    * Separate volume for each sound
    * Master volume
* Text Engine
    * Multiple Fonts
    * Real-Time text generation
    * Text alignment
    * SDF Rendering
* Material system
    * Support for loading and using those texture types:
      * Diffuse
      * Normal
      * Opacity
      * Specular
* Input system
    * Keyboard
    * Mouse
    * Multiple Gamepad support
* 2D Drawing API (HighLevel/DirectX/Utilities.h)
    * Circles (Filled and Outlined)
    * Rectangles
    * Lines
    * Set/Get Draw Color

* What do I plan to do in the future? / What am i working on?
    * You can check out my Trello.

# 3rd-party libs
* Assimp - For loading 
* FreeType
* ImGUI
* STB
    * Image - For loading textures
    * Vorbis (not yet implemented) - For loading ogg files

![Oldish Structure](Misc/EngineStructure.png)

