#include "psvsHeader.hlsli"

float4 main(PS_IN Input) : SV_TARGET
{
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return float4(Input.tex, 1, 1);
}