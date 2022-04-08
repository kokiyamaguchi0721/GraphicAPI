#include "psvsHeader.hlsli"

Texture2D TextureData    : register(t0);
SamplerState SamplerData : register(s0);

float4 main(PS_IN Input) : SV_TARGET
{
    return TextureData.Sample(SamplerData, Input.tex);
    //return Input.Col + TextureData.Sample(SamplerData, Input.tex);
    //return float4(1.0f, 0.0f, 1.0f, 1.0f);

}
