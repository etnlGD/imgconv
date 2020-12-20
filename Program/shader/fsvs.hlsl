#include <imgconv.h>

PS_Input VSMain(uint vid : SV_VertexID) 
{
	PS_Input output;
	if (vid == 0)
	{
		output.clippos = float4(-1, 3, 0, 1);
		output.texcoord = float2(0, -1); 
	}
	else if (vid == 1)
	{
		output.clippos = float4(-1, -1, 0, 1);
		output.texcoord = float2(0, 1); 
	}
	else 
	{
		output.clippos = float4(3, -1, 0, 1);
		output.texcoord = float2(2, 1); 
	}
	
	return output;
}
