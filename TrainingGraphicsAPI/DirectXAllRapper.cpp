#include "DirectXAllRapper.h"

// シェーダーコンパイル
bool DirectXAllRapper::CompileShader(const char* FileName, const char* EntryPoint, const char* ShaderVer, ID3DBlob** ReturnBlob)
{
#ifdef _DEBUG

	UINT    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT    compileFlags = 0;
#endif

	WCHAR	ConvertFileName[512];
	size_t 	wLen = 0;
	int err = 0;

	// char -> wcharに変換
	setlocale(LC_ALL, "japanese");
	err = mbstowcs_s(&wLen, ConvertFileName, 512, FileName, _TRUNCATE);

	ComPtr<ID3DBlob> pErrorBlob = NULL;
	if (FAILED(D3DCompileFromFile(ConvertFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint, ShaderVer, compileFlags, 0, ReturnBlob, pErrorBlob.ReleaseAndGetAddressOf())))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "Compile Error", MB_OK);
		pErrorBlob->Release();
		pErrorBlob = nullptr;
		return false;
	}

    return true;
}
