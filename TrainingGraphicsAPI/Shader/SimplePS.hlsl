#include "psvsHeader.hlsli"

Texture2D TextureData : register(t0);
SamplerState SamplerData : register(s0);

//-----------------------------------------------------------------------------
//      ピクセルシェーダのメインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(VS_OUT input):SV_Target
{
   // PSOutput output =(PSOutput)0;

    //output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    //output.Color = input.Color;

    //return float4(1.0f,1.0f,0.0f,1.0f);
    return TextureData.Sample(SamplerData, input.tex);
}