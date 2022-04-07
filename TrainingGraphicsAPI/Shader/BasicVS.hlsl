#include "psvsHeader.hlsli"
 
cbuffer ConstantBuffer
{
    float4x4 World; //ワールド変換行列
    float4x4 View; //ビュー変換行列
    float4x4 Projection; //透視射影変換行列
}
 
VS_OUT main(VS_IN input)
{
    VS_OUT output;
 
    output.pos = mul(input.pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.col = input.col;
    output.tex = input.tex;
    return output;
}