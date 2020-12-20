#ifndef IMG_CONV_H
#define IMG_CONV_H

SamplerState pointClamp  : register(s0);
SamplerState pointWrap   : register(s1);
SamplerState pointMirror : register(s2);
SamplerState linearClamp : register(s3);
SamplerState linearWrap  : register(s4);
SamplerState linearMirror: register(s5);

struct PS_Input
{
	float2 texcoord : TEXCOORD0;
	float4 clippos : SV_Position;
};

#endif
