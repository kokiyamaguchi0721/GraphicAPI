#pragma once
#include <Windows.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <Directxmath.h>
#include <locale.h>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

#define SAFE_RELEASE(p)      { if( NULL != p ) { p->Release(); p = NULL; } }
#define PI 3.141593

using namespace DirectX;
using namespace Microsoft::WRL;

struct Vertex {
	float pos[3];
	float col[4];
	float nor[3];
	float tex[2];
};

struct ConstantBuffer {
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4   LightDir;
};

class DirectXAllRapper
{
protected:
	bool CompileShader(const char* FileName,const char* EntryPoint,const char* ShaderVer,ID3DBlob** ReturnBlob);
};

