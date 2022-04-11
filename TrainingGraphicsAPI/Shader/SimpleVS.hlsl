#include "psvsHeader.hlsli"

//struct VSInput
//{
//    float3  Position : POSITION;    // 位置座標です.
//    float4  Color    : COLOR;       // 頂点カラーです.
//};

/////////////////////////////////////////////////////////////////////////////////
//// VSOutput structure
/////////////////////////////////////////////////////////////////////////////////
//struct VSOutput
//{
//    float4  Position : SV_POSITION; // 位置座標です.
//    float4  Color    : COLOR;       // 頂点カラーです.
//};

///////////////////////////////////////////////////////////////////////////////
// Transform constant buffer
///////////////////////////////////////////////////////////////////////////////
cbuffer Transform : register( b0 )
{
    float4x4 World : packoffset( c0 ); // ワールド行列です.
    float4x4 View  : packoffset( c4 ); // ビュー行列です.
    float4x4 Proj  : packoffset( c8 ); // 射影行列です.
}

//-----------------------------------------------------------------------------
//      頂点シェーダのメインエントリーポイントです.
//-----------------------------------------------------------------------------
VS_OUT main( VS_IN input )
{
    VS_OUT output = (VS_OUT) 0;

    float4 localPos = input.pos;
    float4 worldPos = mul( World, localPos );
    float4 viewPos  = mul( View,  worldPos );
    float4 projPos  = mul( Proj,  viewPos );

    output.pos = projPos;
    output.col    = input.col;

    return output;
}