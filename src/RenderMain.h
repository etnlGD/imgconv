#pragma once
#include <d3d11.h>
#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>

extern ID3D11Device* g_device;
extern ID3D11DeviceContext* g_context;
extern D3D_FEATURE_LEVEL g_featureLevel;

struct RenderOptions
{
	RenderOptions() :
		width(800), height(600), format(DXGI_FORMAT_R8G8B8A8_UNORM),
		relativeWidth(1.f), relativeHeight(1.f),
		entrypoint("main"), dimensionSpecified(false),
		outputPath("output.dds")
	{

	}

	DXGI_FORMAT format;
	std::string filename;
	std::string entrypoint;
	std::string outputPath;
	std::string modulePath;
	std::vector<std::string> inputTextures;

	void setDimension(UINT width, UINT height)
	{
		this->width = (std::max)(width, 1u);
		this->height = (std::max)(height, 1u);
		this->dimensionSpecified = true;
	}

	void setRelativeDimension(float relativeWidth, float relativeHeight)
	{
		this->relativeWidth = relativeWidth;
		this->relativeHeight = relativeHeight;
	}

	void updateDimensionIfNotSpecified(UINT width, UINT height)
	{
		if (!dimensionSpecified)
		{
			setDimension((UINT) round(width * relativeWidth), 
						 (UINT) round(height * relativeHeight));
		}
	}

	void setUseDefaultDimension()
	{
		dimensionSpecified = true;
	}

	UINT getWidth() const
	{
		assert(dimensionSpecified);
		return width;
	}

	UINT getHeight() const
	{
		assert(dimensionSpecified);
		return height;
	}

private:
	UINT width, height;
	float relativeWidth, relativeHeight;
	bool dimensionSpecified;
};

extern RenderOptions g_options;

enum EDefaultSampleState
{
	SS_POINT_CLAMP,
	SS_POINT_WRAP,
	SS_POINT_MIRROR,
	SS_LINEAR_CLAMP,
	SS_LINEAR_WRAP,
	SS_LINEAR_MIRROR,
	SS_COUNT,
};

struct RenderResources
{
	ID3D11Texture2D* rt;
	ID3D11RenderTargetView* rtv;
	std::vector<ID3D11ShaderResourceView*> srvs;
	ID3D11SamplerState* defaultSamplerStates[SS_COUNT];
	ID3D11DepthStencilState* dsOffState;
	ID3D11RasterizerState* rsState;
	ID3D11Buffer* cb;
	ID3D11VertexShader* fullScreenVS;
	ID3D11PixelShader* ps;

	RenderResources() :
		rt(NULL), rtv(NULL), dsOffState(NULL), rsState(NULL),
		cb(NULL), fullScreenVS(NULL), ps(NULL)
	{
		memset(defaultSamplerStates, 0, sizeof(defaultSamplerStates));
	}

	bool isValid() const
	{
		bool valid = true;
		valid &= (rt != NULL);
		valid &= (rtv != NULL);
		valid &= (dsOffState != NULL);
		valid &= (rsState != NULL);
		valid &= (cb != NULL);
		valid &= (fullScreenVS != NULL);
		valid &= (ps != NULL);

		for (size_t idx = 0; idx < SS_COUNT; ++idx)
			valid &= (defaultSamplerStates[idx] != NULL);

		valid &= (srvs.size() == g_options.inputTextures.size());
		for (auto it = srvs.begin(); it != srvs.end(); ++it)
			valid &= (*it != NULL);

		return valid;
	}
};

extern RenderResources g_renderResources;


void createRenderResources();

void releaseRenderResource();

void renderToTexture();