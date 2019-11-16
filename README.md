# Luna-Engine
 ![Icon](Misc/Luna150_Wide.png) 
 
 Luna Engine is DirectX 11 based engine that i develop. Main reason why is that i want to learn more and try it all by myself

I made trello panel, so you can see what i want to do, what i already done and what i am working on atm.
https://trello.com/b/T8T6vkBN/directx-11-engine-2019

# Version 0.1.120
* Added Attribute Vertex Clouds example. Quick and dirty.
* Some basic profiling tools for graphics debugging
    * ScopedRangeProfiler
          * Basically it calls RangeProfiler::Begin(ScopeName) and (when out of scope) RangeProfiler::End() for you.
    * RangeProfiler
          * RangeProfiler::Begin(Name); - Enters event scope
          * RangeProfiler::End(); - End of last event scope

When using RenderDoc you will see this:

![RenderDoc](https://user-images.githubusercontent.com/8898684/68993358-c83a9880-08a9-11ea-8a74-a85246d0034e.png)

```cpp
{
    ScopedRangeProfiler s1(L"Render depth buffer for directional light");
    rtDepth->Bind();
    rtDepth->Clear(0.f, 0, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
    //                                        Default is 1 ^^^

    //rsFrontCull->Bind();
    gContext->OMSetDepthStencilState(pDSS_Default, 1);

    RenderScene(cLight, RendererFlags::DepthPass | RendererFlags::OpaquePass);
}
```


Didn't mentioned it in any of the prev. updates. Engine now uses inverse depth buffer. So the logic for shaders and CPU side code must consider this.

For example, depth msaa resolve function was: 
```cpp
[unroll(16)] // 32 - is max; 8 - max for me
for( uint i = 0; i < _SampleCount; i++ )
    depth = min(depth, _In.Load(DTid.xy, i));
```

Now it is:
```cpp
[unroll(16)] // 32 - is max; 8 - max for me
for( uint i = 0; i < _SampleCount; i++ )
    depth = max(depth, _In.Load(DTid.xy, i));
```

Depth sampling was:
```cpp
float depth = _DepthTexture.Sample(_LinearSampler, In.Texcoord);
```
Now:
```cpp
float depth = 1.f - _DepthTexture.Sample(_LinearSampler, In.Texcoord);
```
Clear for depth buffer was:
```cpp
rtDepth->Clear(1.f, 0, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
```
Now:
```cpp
rtDepth->Clear(0.f, 0, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
```

# Version 0.1.111
* MSAA support for RenderTargets

* Deleted old RenderBuffer* classes
* Created RenderTarget class
    * 1D/2D/3D RTV/DSV support
    * UAV for RTVs if needed
    * MSAA support
    * ArraySize support
    * Custom amount of RenderBuffers [0-8]
    * Depth buffer can present if needed
    * Cubemaps and Cubemap arrays will be added later
```cpp
RenderTarget<
    size_t dim, 
    size_t BufferNum, 
    bool DepthBuffer=false, 
    size_t ArraySize=1, 
    bool WillHaveMSAA=false, 
    bool Cube=false
>
```

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

