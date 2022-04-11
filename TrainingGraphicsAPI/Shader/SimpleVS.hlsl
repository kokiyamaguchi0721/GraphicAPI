#include "psvsHeader.hlsli"


cbuffer Transform : register( b0 )
{
    float4x4 World : packoffset( c0 ); // ���[���h�s��ł�.
    float4x4 View  : packoffset( c4 ); // �r���[�s��ł�.
    float4x4 Proj  : packoffset( c8 ); // �ˉe�s��ł�.
}

VS_OUT main( VS_IN input )
{
    VS_OUT output = (VS_OUT) 0;

    float4 localPos = input.pos;
    float4 worldPos = mul( World, localPos );
    float4 viewPos  = mul( View,  worldPos );
    float4 projPos  = mul( Proj,  viewPos );

    output.pos = projPos;
    output.col    = input.col;
    output.tex = input.tex;

    return output;
}