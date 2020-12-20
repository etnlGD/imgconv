#include "DeviceUtil.h"
#include <d3dcompiler.h>
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h"
#include <assert.h>
#include "RenderMain.h"

using namespace std;
ID3D11Texture2D* DeviceUtil::createRT(UINT width, UINT height, DXGI_FORMAT format, UINT samples)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = samples;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	ID3D11Texture2D* rt = NULL;
	device->CreateTexture2D(&desc, NULL, &rt);
	return rt;
}

ID3D11Texture2D* DeviceUtil::createCpuReadableTexture(UINT width, UINT height, DXGI_FORMAT format, 
													  UINT miplevels, UINT arraysize, UINT samples)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = miplevels;
	desc.ArraySize = arraysize;
	desc.Format = format;
	desc.SampleDesc.Count = samples;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;

	ID3D11Texture2D* tex = NULL;
	device->CreateTexture2D(&desc, NULL, &tex);
	return tex;
}

ID3D11Buffer* DeviceUtil::createDynamicCB(UINT byteWidth)
{
	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.StructureByteStride = 0;
	desc.MiscFlags = 0;
	desc.ByteWidth = byteWidth;

	ID3D11Buffer* cb = NULL;
	device->CreateBuffer(&desc, NULL, &cb);
	return cb;
}

void DeviceUtil::updateDynamicBuffer(ID3D11Buffer* buffer, const void* data, size_t size)
{
	D3D11_MAPPED_SUBRESOURCE subres;
	context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &data, size);
	context->Unmap(buffer, 0);
}

static char* readFile(const char* path, long* size)
{
	FILE* file = fopen(path, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	char* buf = new char[*size];
	fseek(file, 0, SEEK_SET);
	fread(buf, *size, 1, file);
	fclose(file);

	return buf;
}

ID3DBlob* DeviceUtil::createShader(const char* path, const char* entrypoint, const char* target)
{
	class CShaderInclude : public ID3DInclude
	{
	public:
		// systemDir: Default shader includes directory, used by #include <FILE>
		CShaderInclude(const string& systemDir) : m_SystemDir(systemDir) {}

		HRESULT __stdcall Open(
			D3D_INCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID *ppData,
			UINT *pBytes)
		{
			string finalPath = pFileName;
			switch (IncludeType)
			{
			case D3D_INCLUDE_LOCAL: // #include "FILE"
				break;
			case D3D_INCLUDE_SYSTEM: // #include <FILE>
				finalPath = m_SystemDir + pFileName;
				break;
			default:
				assert(0);
			}

			long size;
			char* source = readFile(finalPath.c_str(), &size);
			if (source == NULL)
				return E_FAIL;

			*ppData = source;
			*pBytes = size;
			return S_OK;
		}

		HRESULT __stdcall Close(LPCVOID pData)
		{
			// Here we must correctly free buffer created in Open.
			char* buf = (char*)pData;
			delete[] buf;
			return S_OK;

		}

	private:
		string m_SystemDir;
	};

	long size;
	char* source = readFile(path, &size);
	if (source == NULL)
	{
		string path2 = g_options.modulePath + "\\" + path;
		source = readFile(path2.c_str(), &size);
		if (source == NULL)
			return NULL;
	}

	CShaderInclude shaderInclude(g_options.modulePath + "\\shader\\");
	ID3DBlob *code = NULL, *errorMsg = NULL;
	if (FAILED(D3DCompile(source, size, path, 
		NULL, &shaderInclude, entrypoint, target, 
		0, 0, // compile flags
		&code, &errorMsg)))
	{
		if (errorMsg != NULL)
		{
			printf("%s", (const char*)errorMsg->GetBufferPointer());
			errorMsg->Release();
		}

		if (code != NULL)
			code->Release();

		delete[] source;
		return NULL;
	}

	delete[] source;
	if (errorMsg != NULL)
		errorMsg->Release();

	return code;
}

ID3D11VertexShader* DeviceUtil::createVertexShader(const char* path, const char* entrypoint)
{
	ID3DBlob* code = createShader(path, entrypoint, "vs_5_0");
	if (code != NULL)
	{
		ID3D11VertexShader* vs = NULL;
		device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), NULL, &vs);
		code->Release();
		return vs;
	}

	return NULL;
}

ID3D11PixelShader* DeviceUtil::createPixelShader(const char* path, const char* entrypoint)
{
	ID3DBlob* code = createShader(path, entrypoint, "ps_5_0");
	if (code != NULL)
	{
		ID3D11PixelShader* ps = NULL;
		device->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), NULL, &ps);
		code->Release();
		return ps;
	}

	return NULL;
}