Reqs:
 - DirectX 11.1 / 11.0 / 10.1 / 10.0 hardware/warp device support
 - Shader Model 5.0 hardware support
 - Recent hardware drivers, they most likly to be more performant
    - I use GTX 980 with driver version 431.60 from 07/23/2019
    - Intel i5 6600 3.30 GHz
 - x64 or x86 system

SSPBR Shader:
Input semantics
 - In.Texcoord  - float2 in range of [0.f; 1.f]
 - In.CameraPos - unused

Textures
 - _Cubemap   - Cubemap with HDR support and mip mapping
 - _Heightmap - single channel texture

cbuffer
 - _Texel = 1.f / _Heightmap.Width; same for height
 - _Dummy2 = 0.f; // Always
 - _LightPos = float3(mouse coord divided by window size, LightZ);

Usage of rest variables is shown here:

			Name  		Value       Step   Min      Max
    ImGui::DragFloat("Exposure"      , &Exposure   , .01f ,  0.f  , 12.f);
    ImGui::DragFloat("Albedo"        , &Albedo     , .005f,  .001f, 1.f);
    ImGui::DragFloat("Metalness"     , &Metalness  , .005f,  .001f, 1.f);
    ImGui::DragFloat("Rougness"      , &Rougness   , .005f,  .001f, 1.f);
    ImGui::DragFloat("LightZ"        , &LightZ     , .005f, -2.f  , 2.f); // Light's z
    ImGui::DragFloat3("Repeats"      , Repeats     , .1f  ,  .5f  , 5.f); // Repeats for texture in each dim
    ImGui::DragFloat3("Light Diffuse", LightDiffuse, .1f  ,  0.f  , 1.f); // Mouse Point Light Color

