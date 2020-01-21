void main(in  float3 P        : POSITION0  , in uint InstanceID : SV_InstanceID, 
          out float4 Position : SV_Position, out uint Instance  : TEXCOORD0) {
    Position = float4(0., 0., 0., 1.);
    Instance = InstanceID;
}
