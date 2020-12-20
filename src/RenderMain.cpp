#include "RenderMain.h"
#include "DeviceUtil.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h"

using namespace std;

ID3D11Device* g_device = NULL;
ID3D11DeviceContext* g_context = NULL;
D3D_FEATURE_LEVEL g_featureLevel;
RenderResources g_renderResources;
RenderOptions g_options;


struct PSConstantBuffer
{
	float dimension[4];
};

void createRenderResources()
{
	DeviceUtil deviceUtil(g_device, g_context);
	// samplers
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	ID3D11SamplerState** defaultSamplerStates = g_renderResources.defaultSamplerStates;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	g_device->CreateSamplerState(&samplerDesc, &defaultSamplerStates[SS_POINT_CLAMP]);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	g_device->CreateSamplerState(&samplerDesc, &defaultSamplerStates[SS_POINT_WRAP]);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	g_device->CreateSamplerState(&samplerDesc, &defaultSamplerStates[SS_POINT_MIRROR]);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	g_device->CreateSamplerState(&samplerDesc, &defaultSamplerStates[SS_LINEAR_CLAMP]);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	g_device->CreateSamplerState(&samplerDesc, &defaultSamplerStates[SS_LINEAR_WRAP]);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	g_device->CreateSamplerState(&samplerDesc, &defaultSamplerStates[SS_LINEAR_MIRROR]);

	// textures
	size_t idx = 0;
	g_renderResources.srvs.resize(g_options.inputTextures.size(), NULL);
	for (auto it = g_options.inputTextures.begin(); it != g_options.inputTextures.end(); ++it)
	{
		D3DX11_IMAGE_INFO srcInfo;
		if (FAILED(D3DX11GetImageInfoFromFileA(it->c_str(), NULL, &srcInfo, NULL)))
		{
			printf("create texture(%s) failed.\n", it->c_str());
			continue;
		}

		if (it == g_options.inputTextures.begin())
			g_options.updateDimensionIfNotSpecified(srcInfo.Width, srcInfo.Height);

		D3DX11_IMAGE_LOAD_INFO loadInfo;
		loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		loadInfo.CpuAccessFlags = 0;
		loadInfo.MiscFlags = 0;
		loadInfo.FirstMipLevel = 0;
		loadInfo.MipLevels = 0;
		loadInfo.pSrcInfo = &srcInfo;

		ID3D11Resource* pTex;
		D3DX11CreateTextureFromFileA(g_device, it->c_str(), &loadInfo, NULL, &pTex, NULL);

		if (pTex != NULL)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Format = srcInfo.Format;
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.MipLevels = (UINT)-1;
			g_device->CreateShaderResourceView(pTex, &desc, &g_renderResources.srvs[idx++]);

			pTex->Release();
		}
		else
		{
			printf("create texture(%s) failed.\n", it->c_str());
		}
	}

	if (g_options.inputTextures.empty() || g_renderResources.srvs[0] == NULL)
		g_options.setUseDefaultDimension(); // use default dimension.

	// render target
	g_renderResources.rt = deviceUtil.createRT(g_options.getWidth(), g_options.getHeight(), g_options.format);
	if (g_renderResources.rt == NULL)
		printf("create render target failed.\n");
	else
		g_device->CreateRenderTargetView(g_renderResources.rt, NULL, &g_renderResources.rtv);

	// constant buffer
	g_renderResources.cb = deviceUtil.createDynamicCB(sizeof(PSConstantBuffer));
	PSConstantBuffer cbData;
	cbData.dimension[0] = (float)g_options.getWidth();
	cbData.dimension[1] = (float)g_options.getHeight();
	cbData.dimension[2] = 1.0f / g_options.getWidth();
	cbData.dimension[3] = 1.0f / g_options.getHeight();
	deviceUtil.updateDynamicBuffer(g_renderResources.cb, &cbData, sizeof(cbData));

	// shaders
	g_renderResources.fullScreenVS = deviceUtil.createVertexShader("shader\\fsvs.hlsl", "VSMain");
	g_renderResources.ps = deviceUtil.createPixelShader(g_options.filename.c_str(),
														g_options.entrypoint.c_str());
	if (g_renderResources.ps == NULL)
	{
		printf("create pixel shader(%s@%s) failed.\n", 
			   g_options.entrypoint.c_str(), g_options.filename.c_str());
	}

	// depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsOffDesc;
	memset(&dsOffDesc, 0, sizeof(dsOffDesc));
	dsOffDesc.DepthEnable = FALSE;
	dsOffDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsOffDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dsOffDesc.StencilEnable = FALSE;
	dsOffDesc.StencilReadMask = 0;
	dsOffDesc.StencilWriteMask = 0;
	dsOffDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsOffDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsOffDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsOffDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;
	dsOffDesc.BackFace = dsOffDesc.FrontFace;
	g_device->CreateDepthStencilState(&dsOffDesc, &g_renderResources.dsOffState);

	// rasterizer state
	D3D11_RASTERIZER_DESC rsDesc;
	rsDesc.AntialiasedLineEnable = FALSE;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0;
	rsDesc.DepthClipEnable = TRUE;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.MultisampleEnable = FALSE;
	rsDesc.ScissorEnable = FALSE;
	rsDesc.SlopeScaledDepthBias = 0;
	g_device->CreateRasterizerState(&rsDesc, &g_renderResources.rsState);
}

#define SAFE_RELEASE(p) { if ((p) != NULL) { (p)->Release(); (p) = NULL; } }

void releaseRenderResource()
{
	for (auto it = g_renderResources.srvs.begin(); it != g_renderResources.srvs.end(); ++it)
		SAFE_RELEASE(*it);
	g_renderResources.srvs.clear();

	for (size_t idx = 0; idx < SS_COUNT; ++idx)
		SAFE_RELEASE(g_renderResources.defaultSamplerStates[idx]);

	SAFE_RELEASE(g_renderResources.dsOffState);
	SAFE_RELEASE(g_renderResources.rsState);
	SAFE_RELEASE(g_renderResources.cb);
	SAFE_RELEASE(g_renderResources.fullScreenVS);
	SAFE_RELEASE(g_renderResources.ps);
	SAFE_RELEASE(g_renderResources.rtv);
	SAFE_RELEASE(g_renderResources.rt);
}

void renderToTexture()
{
	g_context->OMSetRenderTargets(1, &g_renderResources.rtv, NULL);
	g_context->OMSetBlendState(NULL, NULL, 0xffffffff);
	g_context->OMSetDepthStencilState(g_renderResources.dsOffState, 0);

	g_context->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);
	g_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	g_context->IASetInputLayout(NULL);
	g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_context->VSSetConstantBuffers(0, 0, NULL);
	g_context->VSSetSamplers(0, 0, NULL);
	g_context->VSSetShader(g_renderResources.fullScreenVS, NULL, 0);
	g_context->VSSetShaderResources(0, 0, NULL);

	g_context->HSSetShader(NULL, NULL, 0);
	g_context->DSSetShader(NULL, NULL, 0);
	g_context->GSSetShader(NULL, NULL, 0);
	g_context->SOSetTargets(0, NULL, NULL);
	g_context->CSSetShader(NULL, NULL, 0);

	g_context->RSSetScissorRects(0, NULL);
	g_context->RSSetState(g_renderResources.rsState);
	// need set viewport!
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = viewport.TopLeftY = 0;
	viewport.Width = (float)g_options.getWidth();
	viewport.Height = (float)g_options.getHeight();
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	g_context->RSSetViewports(1, &viewport);

	g_context->PSSetConstantBuffers(0, 1, &g_renderResources.cb);
	// [point/linear][clamp/wrap/mirror]
	g_context->PSSetSamplers(0, SS_COUNT, g_renderResources.defaultSamplerStates);
	g_context->PSSetShaderResources(0, (UINT)g_renderResources.srvs.size(),
									g_renderResources.srvs.empty() ? NULL : &g_renderResources.srvs[0]);
	g_context->PSSetShader(g_renderResources.ps, NULL, 0);

	FLOAT color[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	g_context->ClearRenderTargetView(g_renderResources.rtv, color);
	g_context->Draw(3, 0);
}


