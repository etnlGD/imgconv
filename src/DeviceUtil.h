#pragma once
#include <d3d11.h>
#include <string>

class DeviceUtil
{
public:
	DeviceUtil(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) :
		device(pDevice), context(pContext)
	{
		device->AddRef();
		context->AddRef();
	}

	~DeviceUtil()
	{
		device->Release();
		context->Release();
	}

	ID3D11Texture2D* createRT(UINT width, UINT height, DXGI_FORMAT format, UINT samples = 1);

	ID3D11Texture2D* createCpuReadableTexture(UINT width, UINT height, DXGI_FORMAT format,
											  UINT miplevels = 1, UINT arraysize = 1,
											  UINT samples = 1);

	ID3D11Buffer* createDynamicCB(UINT byteWidth);

	void updateDynamicBuffer(ID3D11Buffer* buffer, const void* data, size_t size);

	ID3DBlob* createShader(const char* path, const char* entrypoint, const char* target);

	ID3D11VertexShader* createVertexShader(const char* path, const char* entrypoint);

	ID3D11PixelShader* createPixelShader(const char* path, const char* entrypoint);

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

