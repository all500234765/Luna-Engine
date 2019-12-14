inline uint pack_unitvector(in float3 value)
{
	uint retVal = 0;
	retVal |= (uint)((value.x * 0.5f + 0.5f) * 255.0f) << 0;
	retVal |= (uint)((value.y * 0.5f + 0.5f) * 255.0f) << 8;
	retVal |= (uint)((value.z * 0.5f + 0.5f) * 255.0f) << 16;
	return retVal;
}
inline float3 unpack_unitvector(in uint value)
{
	float3 retVal;
	retVal.x = (float)((value >> 0) & 0x000000FF) / 255.0f * 2.0f - 1.0f;
	retVal.y = (float)((value >> 8) & 0x000000FF) / 255.0f * 2.0f - 1.0f;
	retVal.z = (float)((value >> 16) & 0x000000FF) / 255.0f * 2.0f - 1.0f;
	return retVal;
}

inline uint pack_rgba(in float4 value)
{
	uint retVal = 0;
	retVal |= (uint)(value.x * 255.0f) << 0;
	retVal |= (uint)(value.y * 255.0f) << 8;
	retVal |= (uint)(value.z * 255.0f) << 16;
	retVal |= (uint)(value.w * 255.0f) << 24;
	return retVal;
}
inline float4 unpack_rgba(in uint value)
{
	float4 retVal;
	retVal.x = (float)((value >> 0) & 0x000000FF) / 255.0f;
	retVal.y = (float)((value >> 8) & 0x000000FF) / 255.0f;
	retVal.z = (float)((value >> 16) & 0x000000FF) / 255.0f;
	retVal.w = (float)((value >> 24) & 0x000000FF) / 255.0f;
	return retVal;
}

inline uint2 pack_half3(in float3 value)
{
	uint2 retVal = 0;
	retVal.x = f32tof16(value.x) | (f32tof16(value.y) << 16);
	retVal.y = f32tof16(value.z);
	return retVal;
}
inline float3 unpack_half3(in uint2 value)
{
	float3 retVal;
	retVal.x = f16tof32(value.x);
	retVal.y = f16tof32(value.x >> 16);
	retVal.z = f16tof32(value.y);
	return retVal;
}
inline uint2 pack_half4(in float4 value)
{
	uint2 retVal = 0;
	retVal.x = f32tof16(value.x) | (f32tof16(value.y) << 16);
	retVal.y = f32tof16(value.z) | (f32tof16(value.w) << 16);
	return retVal;
}
inline float4 unpack_half4(in uint2 value)
{
	float4 retVal;
	retVal.x = f16tof32(value.x);
	retVal.y = f16tof32(value.x >> 16);
	retVal.z = f16tof32(value.y);
	retVal.w = f16tof32(value.y >> 16);
	return retVal;
}

// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
inline uint expandBits(uint v)
{
	v = (v * 0x00010001u) & 0xFF0000FFu;
	v = (v * 0x00000101u) & 0x0F00F00Fu;
	v = (v * 0x00000011u) & 0xC30C30C3u;
	v = (v * 0x00000005u) & 0x49249249u;
	return v;
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
inline uint morton3D(in float3 pos)
{
	pos.x = min(max(pos.x * 1024.0f, 0.0f), 1023.0f);
	pos.y = min(max(pos.y * 1024.0f, 0.0f), 1023.0f);
	pos.z = min(max(pos.z * 1024.0f, 0.0f), 1023.0f);
	uint xx = expandBits((uint)pos.x);
	uint yy = expandBits((uint)pos.y);
	uint zz = expandBits((uint)pos.z);
	return xx * 4 + yy * 2 + zz;
}

