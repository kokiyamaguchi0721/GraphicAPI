#include "psvsHeader.hlsli"

Texture2D TextureData    : register(t0);
SamplerState SamplerData : register(s0);

float4 main(PS_IN Input) : SV_TARGET
{
    return TextureData.Sample(SamplerData, Input.tex);
}
