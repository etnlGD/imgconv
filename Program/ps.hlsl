#include "shader/imgconv.h"

Texture2D tex0 : register(t0);

float4 f(float2 texcoord, int dx, int dy)
{
	return tex0.SampleLevel(pointClamp, texcoord, 0, int2(dx, dy));
}

float4 f2(float2 texcoord, int dx, int dy)
{
	return tex0.SampleLevel(pointWrap, texcoord, 0, int2(dx, dy));
}


float4 mainEdge(PS_Input input) : SV_Target0
{
	float2 tc = input.texcoord;
	float4 Dx = f(tc, -1,  1) +
				f(tc,  0,  1) +
				f(tc,  1,  1) -
				f(tc, -1, -1) - 
				f(tc,  0, -1) - 
				f(tc, +1, -1);
				
	float4 Dy = f(tc, -1, -1) +
				f(tc, -1,  0) +
				f(tc, -1,  1) -
				f(tc,  1, -1) -
				f(tc,  1,  0) -
				f(tc,  1,  1);
				
	float4 diff = sqrt(Dx * Dx + Dy * Dy);
	float diff0 = dot(diff.rgb, 1.0) / 3.0;
	return float4(diff0.xxx, 1.0);
}

float4 mainPoster(PS_Input input) : SV_Target0
{
	float4 color = f(input.texcoord, 0, 0);
	int3 icolor = (int3) (color.rgb * 255);
	icolor &= 0xe0;
	return float4((float3) icolor / 255.0, color.a);
}

float4 mainGray(PS_Input input) : SV_Target0
{
	float4 color = f(input.texcoord, 0, 0);
	return float4(dot(color.rgb, float3(0.11, 0.59, 0.3)).xxx, color.a);
}

//float4 main(PS_Input input) : SV_Target0
//{
//	return float4(1.0, 0.0, 0.0, 1.0);
//}

float4 main(PS_Input input) : SV_Target0
{
	float4 a = f2(input.texcoord, 0, 0);
	float4 b = f2(input.texcoord - float2(37.0, 17.0) / 256.0, 0, 0);
	return float4(a.x, b.x, a.z, b.z);
}