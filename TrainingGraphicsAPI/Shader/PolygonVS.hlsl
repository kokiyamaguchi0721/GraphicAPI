#include "psvsHeader.hlsli"
 
VS_OUT main(VS_IN input)
{
    VS_OUT output;
 
    output.pos = input.pos;
    output.col = input.col;
    output.tex = input.tex;
    return output;
}