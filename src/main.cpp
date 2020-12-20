#include "RenderMain.h"
#include "getopt.h"
#include <d3dx11.h>
#include <algorithm>
#include <shlwapi.h>

static void initD3D11Device()
{
	D3D_FEATURE_LEVEL levels[] = 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
					  levels, sizeof(levels) / sizeof(levels[0]), D3D11_SDK_VERSION,
					  &g_device, &g_featureLevel, &g_context);
}

struct FormatMapping 
{
	const char* formatname;
	DXGI_FORMAT format;
};

static FormatMapping sFormatMapping[] = 
{
	{ "R32G32B32A32_FLOAT", DXGI_FORMAT_R32G32B32A32_FLOAT },
	{ "R32G32B32A32_UINT", DXGI_FORMAT_R32G32B32A32_UINT },
	{ "R32G32B32A32_SINT", DXGI_FORMAT_R32G32B32A32_SINT },
	{ "R32G32B32_FLOAT", DXGI_FORMAT_R32G32B32_FLOAT },
	{ "R32G32B32_UINT", DXGI_FORMAT_R32G32B32_UINT },
	{ "R32G32B32_SINT", DXGI_FORMAT_R32G32B32_SINT },
	{ "R16G16B16A16_FLOAT", DXGI_FORMAT_R16G16B16A16_FLOAT },
	{ "R16G16B16A16_UNORM", DXGI_FORMAT_R16G16B16A16_UNORM },
	{ "R16G16B16A16_UINT", DXGI_FORMAT_R16G16B16A16_UINT },
	{ "R16G16B16A16_SNORM", DXGI_FORMAT_R16G16B16A16_SNORM },
	{ "R16G16B16A16_SINT", DXGI_FORMAT_R16G16B16A16_SINT },
	{ "R32G32_FLOAT", DXGI_FORMAT_R32G32_FLOAT },
	{ "R32G32_UINT", DXGI_FORMAT_R32G32_UINT },
	{ "R32G32_SINT", DXGI_FORMAT_R32G32_SINT },
	{ "R10G10B10A2_UNORM", DXGI_FORMAT_R10G10B10A2_UNORM },
	{ "R10G10B10A2_UINT", DXGI_FORMAT_R10G10B10A2_UINT },
	{ "R11G11B10_FLOAT", DXGI_FORMAT_R11G11B10_FLOAT },
	{ "R8G8B8A8_UNORM", DXGI_FORMAT_R8G8B8A8_UNORM },
	{ "R8G8B8A8_UNORM_SRGB", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
	{ "R8G8B8A8_UINT", DXGI_FORMAT_R8G8B8A8_UINT },
	{ "R8G8B8A8_SNORM", DXGI_FORMAT_R8G8B8A8_SNORM },
	{ "R8G8B8A8_SINT", DXGI_FORMAT_R8G8B8A8_SINT },
	{ "R16G16_FLOAT", DXGI_FORMAT_R16G16_FLOAT },
	{ "R16G16_UNORM", DXGI_FORMAT_R16G16_UNORM },
	{ "R16G16_UINT", DXGI_FORMAT_R16G16_UINT },
	{ "R16G16_SNORM", DXGI_FORMAT_R16G16_SNORM },
	{ "R16G16_SINT", DXGI_FORMAT_R16G16_SINT },
	{ "R32_FLOAT", DXGI_FORMAT_R32_FLOAT },
	{ "R32_UINT", DXGI_FORMAT_R32_UINT },
	{ "R32_SINT", DXGI_FORMAT_R32_SINT },
	{ "R8G8_UNORM", DXGI_FORMAT_R8G8_UNORM },
	{ "R8G8_UINT", DXGI_FORMAT_R8G8_UINT },
	{ "R8G8_SNORM", DXGI_FORMAT_R8G8_SNORM },
	{ "R8G8_SINT", DXGI_FORMAT_R8G8_SINT },
	{ "R16_FLOAT", DXGI_FORMAT_R16_FLOAT },
	{ "R16_UNORM", DXGI_FORMAT_R16_UNORM },
	{ "R16_UINT", DXGI_FORMAT_R16_UINT },
	{ "R16_SNORM", DXGI_FORMAT_R16_SNORM },
	{ "R16_SINT", DXGI_FORMAT_R16_SINT },
	{ "R8_UNORM", DXGI_FORMAT_R8_UNORM },
	{ "R8_UINT", DXGI_FORMAT_R8_UINT },
	{ "R8_SNORM", DXGI_FORMAT_R8_SNORM },
	{ "R8_SINT", DXGI_FORMAT_R8_SINT },
	{ "A8_UNORM", DXGI_FORMAT_A8_UNORM },
	{ "R1_UNORM", DXGI_FORMAT_R1_UNORM },
	{ "R9G9B9E5_SHAREDEXP", DXGI_FORMAT_R9G9B9E5_SHAREDEXP },
	{ "R8G8_B8G8_UNORM", DXGI_FORMAT_R8G8_B8G8_UNORM },
	{ "G8R8_G8B8_UNORM", DXGI_FORMAT_G8R8_G8B8_UNORM },
	{ "B5G6R5_UNORM", DXGI_FORMAT_B5G6R5_UNORM },
	{ "B5G5R5A1_UNORM", DXGI_FORMAT_B5G5R5A1_UNORM },
	{ "B8G8R8A8_UNORM", DXGI_FORMAT_B8G8R8A8_UNORM },
	{ "B8G8R8X8_UNORM", DXGI_FORMAT_B8G8R8X8_UNORM },
	{ "R10G10B10_XR_BIAS_A2_UNORM", DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
	{ "B8G8R8A8_UNORM_SRGB", DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
	{ "B8G8R8X8_UNORM_SRGB", DXGI_FORMAT_B8G8R8X8_UNORM_SRGB },
};

static const char* usageDoc =
	"Usage: imgconv [OPTION]... <SHADER_FILE>\n"
	"Try 'imgconv --help' for more information.\n";

static const char* helpDoc =
"TODO_wzq\n";

/// imgconv -d800*600 -iinput0.dds -iinput1.dds -fR8G8B8A8_UNORM -emain ps.hlsl
/// imgconv [options...] <shader_file>
/// dimension: default same as first input texture, if there is no input texture, default as 800*600
/// texture: default no texture
/// format: default R8G8B8A8_UNORM
/// entrypoint: default main

// TODO
/// Define/fxo/CMake/help/constant
int main(int argc, char* const argv[])
{
	char ownPth[MAX_PATH];
	GetModuleFileName(NULL, ownPth, sizeof(ownPth));
	PathRemoveFileSpec(ownPth);
	g_options.modulePath = ownPth;

	static struct option sLongOptions[] =
	{
		{ "dimension",	required_argument,	NULL, 'd' },
		{ "relative",	required_argument,	NULL, 'r' },
		{ "input",		required_argument,	NULL, 'i' },
		{ "format",		required_argument,	NULL, 'f' },
		{ "entrypoint", required_argument,	NULL, 'e' },
		{ "output",		required_argument,	NULL, 'o' },
		{ "help",		no_argument,		NULL, 'h' },
		{ NULL,			0,					NULL,  0  },
	};

	int optargc = (argv[argc - 1][0] == '-') ? argc : (argc - 1);
	int ch;
	while ((ch = getopt_long(optargc, argv, "d:i:f:e:o:r:", sLongOptions, NULL)) != -1)
	{
		switch (ch)
		{
		case 'd':
		{
			UINT width, height;
			if (sscanf(optarg, "%u*%u", &width, &height) != 2)
				printf("bad format for dimension argument, expected \"<width>*<height>\".\n");
			else if (width > 65536 || height > 65536 || width == 0 || height == 0)
				printf("invalid range for width/height, which should be in range [1, 65536].\n");
			else
				g_options.setDimension(width, height);
			break;
		}
		case 'r':
		{
			float relativeWidth, relativeHeight;
			if (sscanf(optarg, "%f*%f", &relativeWidth, &relativeHeight) != 2)
				printf("bad format for dimension argument, expected \"<width>*<height>\".\n");
			else if (relativeWidth <= 0 || relativeHeight <= 0)
				printf("invalid range for relative width/height, which should be positive.\n");
			else
				g_options.setRelativeDimension(relativeWidth, relativeHeight);
			break;
		}
		case 'i':
			g_options.inputTextures.push_back(optarg);
			break;
		case 'f':
		{
			size_t length = strlen(optarg);
			char* uppercaseOptarg = new char[length + 1];
			std::transform(optarg, optarg + length, uppercaseOptarg, toupper);
			uppercaseOptarg[length] = '\0';
			size_t formatCount = sizeof(sFormatMapping) / sizeof(FormatMapping);
			size_t idx;
			for (idx = 0; idx < formatCount; ++idx)
			{
				if (strcmp(uppercaseOptarg, sFormatMapping[idx].formatname) == 0)
				{
					g_options.format = sFormatMapping[idx].format;
					break;
				}
			}

			if (idx == formatCount)
				printf("invalid format(%s).\n", optarg);

			delete[] uppercaseOptarg;
			break;
		}
		case 'e':
			g_options.entrypoint = optarg;
			break;
		case 'o':
			g_options.outputPath = optarg;
			break;
		case 'h':
			printf("%s", helpDoc);
			exit(0);
			break;
		default:
			break;
		}
	}

	if (argc == 1 || argc == optargc)
	{
		printf("%s", usageDoc);
		return 0;
	}

	g_options.filename = argv[argc - 1];

	initD3D11Device();
	if (g_context == NULL || g_device == NULL)
	{
		printf("init d3d failed.\n");
		return 1;
	}

	createRenderResources();

	int retcode = 1;
	if (g_renderResources.isValid())
	{
		renderToTexture();
		D3DX11SaveTextureToFile(g_context, g_renderResources.rt, D3DX11_IFF_DDS, 
								g_options.outputPath.c_str());
		retcode = 0;
	}

	releaseRenderResource();
	g_context->Release();
	g_device->Release();
	return retcode;
}
