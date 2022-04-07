#include "psvsHeader.hlsli"
 
cbuffer ConstantBuffer
{
    float4x4 World; //���[���h�ϊ��s��
    float4x4 View; //�r���[�ϊ��s��
    float4x4 Projection; //�����ˉe�ϊ��s��
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