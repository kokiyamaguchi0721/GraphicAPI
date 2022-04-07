#pragma once
#include <Windows.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <Directxmath.h>
#include <locale.h>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

#define SAFE_RELEASE(p)      { if( NULL != p ) { p->Release(); p = NULL; } }

using namespace DirectX;
using namespace Microsoft::WRL;

class DirectXAllRapper
{
protected:
	bool CompileShader(const char* FileName,const char* EntryPoint,const char* ShaderVer,ID3DBlob** ReturnBlob);
};

