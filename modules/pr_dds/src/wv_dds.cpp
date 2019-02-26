#include "wv_dds.hpp"
#include <pragma/ishared.hpp>
#include <luainterface.hpp>
#include <luasystem.h>
#include <util_image.h>
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_string.h>
#include <sharedutils/scope_guard.h>
#include <pragma/lua/libraries/lfile.h>
#include <fsys/filesystem.h>
#include <nvtt/nvtt.h>
#include <texturemanager/loadimagedata.h>
#include <png_info.h>
#include <pragma/pragma_module.hpp>

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"ishared.lib")
#pragma comment(lib,"nvtt.lib")
#pragma comment(lib,"nvimage.lib")
#pragma comment(lib,"nvcore.lib")
#pragma comment(lib,"nvmath.lib")
#pragma comment(lib,"squish.lib")
#pragma comment(lib,"bc7.lib")
#pragma comment(lib,"bc6h.lib")
#pragma comment(lib,"nvthread.lib")
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"util_image.lib")
#pragma comment(lib,"libpng16.lib")
#pragma comment(lib,"zlib.lib")

#pragma optimize("",off)

#undef small

struct DDSConvertInfo
{
	DDSConvertInfo()=default;

	enum class Flags : uint32_t
	{
		None = 0u,
		ConvertToNormalMap = 1u,
		GenerateMipmaps = ConvertToNormalMap<<1u,
		NormalizeMipmaps = GenerateMipmaps<<1u,
		NormalMap = NormalizeMipmaps<<1u,
	};
	struct ColorWeights
	{
		float red = 1.f;
		float green = 1.f;
		float blue = 1.f;
		float alpha = 1.f;
	};

	struct HeightEvaluation
	{
		float redScale = 0.f;
		float greenScale = 0.f;
		float blueScale = 0.f;
		float alphaScale = 1.f;
	};

	struct KaiserParameters
	{
		float width = 3.f;
		float alpha = 4.f;
		float stretch = 1.f;
	};

	struct Quantization
	{
		bool colorDithering;
		bool alphaDithering;
		bool binaryAlpha;
		int32_t alphaThreshold;
	};

	struct PixelFormat0
	{
		uint32_t bitCount;
		uint32_t rMask;
		uint32_t gMask;
		uint32_t bMask;
		uint32_t aMask;
	};

	struct PixelFormat1
	{
		uint8_t rSize;
		uint8_t gSize;
		uint8_t bSize;
		uint8_t aSize;
	};

	struct NormalFilter
	{
		float small;
		float medium;
		float big;
		float large;
	};

	struct TextureLayout
	{
		TextureLayout(nvtt::TextureType type,int width,int height,int depth=1,int arraySize=1)
			: type(type),width(width),height(height),depth(depth),arraySize(arraySize)
		{}
		nvtt::TextureType type;
		int32_t width;
		int32_t height;
		int32_t depth = 1;
		int32_t arraySize = 1;
	};

	Flags flags = Flags::None;
	float inputGamma = 2.2;
	float outputGamma = 2.2;
	HeightEvaluation heightEvaluation = {};
	KaiserParameters kaiserParameters = {};
	int32_t maxExtents = 0;
	nvtt::MipmapFilter mipmapFilter = nvtt::MipmapFilter::MipmapFilter_Box;
	int32_t maxMipmapLevel = -1;
	nvtt::RoundMode roundMode = nvtt::RoundMode::RoundMode_None;
	nvtt::WrapMode wrapMode = nvtt::WrapMode::WrapMode_Mirror;
	ColorWeights colorWeights = {};
	nvtt::Format format = nvtt::Format::Format_DXT1;
	uint32_t pitchAlignment = 1;
	float rgbmThreshold = 0.15f;
	nvtt::Quality quality = nvtt::Quality::Quality_Normal;
	nvtt::PixelType pixelType = nvtt::PixelType::PixelType_UnsignedNorm;
	std::shared_ptr<Quantization> quantization = nullptr;
	std::shared_ptr<PixelFormat0> pixelFormat0 = nullptr;
	std::shared_ptr<PixelFormat1> pixelFormat1 = nullptr;
	std::shared_ptr<NormalFilter> normalFilter = nullptr;
	std::shared_ptr<TextureLayout> textureLayout = nullptr;

	void SetQuantization(bool colorDithering,bool alphaDithering,bool binaryAlpha,int32_t alphaThreshold=127)
	{
		quantization = std::make_shared<Quantization>(Quantization{colorDithering,alphaDithering,binaryAlpha,alphaThreshold});
	}
	void SetPixelFormat(uint32_t bitcount,uint32_t rmask,uint32_t gmask,uint32_t bmask,uint32_t amask)
	{
		pixelFormat0 = std::make_shared<PixelFormat0>(PixelFormat0{bitcount,rmask,gmask,bmask,amask});
	}
	void SetPixelFormat(uint8_t rsize,uint8_t gsize,uint8_t bsize,uint8_t asize)
	{
		pixelFormat1 = std::make_shared<PixelFormat1>(PixelFormat1{rsize,gsize,bsize,asize});
	}
	void SetNormalFilter(float small,float medium,float big,float large)
	{
		normalFilter = std::make_shared<NormalFilter>(NormalFilter{small,medium,big,large});
	}
	void SetTextureLayout(nvtt::TextureType type,int width,int height,int depth=1,int arraySize=1)
	{
		textureLayout = std::make_shared<TextureLayout>(TextureLayout{type,width,height,depth,arraySize});
	}
};
REGISTER_BASIC_BITWISE_OPERATORS(DDSConvertInfo::Flags);
lua_registercheck(DDSConvertInfo,DDSConvertInfo);

static int32_t convert_dds(lua_State *l)
{
	auto &srcFile = *Lua::CheckFile(l,1);
	auto imgFormat = static_cast<uimg::ImageFormat>(Lua::CheckInt(l,2));
	std::string dstFile = FileManager::GetCanonicalizedPath(Lua::CheckString(l,3));
	DDSConvertInfo convertInfo {};
	if(Lua::IsSet(l,4))
		convertInfo = *Lua::CheckDDSConvertInfo(l,4);
	if(Lua::file::validate_write_operation(l,dstFile) == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	FileManager::CreatePath(ufile::get_path_from_filename(dstFile).c_str());
	dstFile = util::get_program_path() +"\\" +dstFile;

	auto image = uimg::load_image(srcFile->GetHandle(),imgFormat);
	if(image == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	image->ConvertToRGBA();

	auto width = image->GetWidth();
	auto height = image->GetHeight();
	auto &data = image->GetData();

	//
	nvtt::InputOptions inputOptions {};
	inputOptions.reset();
	inputOptions.setTextureLayout(nvtt::TextureType_2D,width,height);
	inputOptions.setMipmapData(data.data(),width,height);
	inputOptions.setFormat(nvtt::InputFormat_BGRA_8UB);

	auto bConvertToNormalMap = (convertInfo.flags &DDSConvertInfo::Flags::ConvertToNormalMap) != DDSConvertInfo::Flags::None;
	auto bGenerateMipmaps = (convertInfo.flags &DDSConvertInfo::Flags::GenerateMipmaps) != DDSConvertInfo::Flags::None;
	auto bNormalizeMipmaps = (convertInfo.flags &DDSConvertInfo::Flags::NormalizeMipmaps) != DDSConvertInfo::Flags::None;
	auto bNormalMap = (convertInfo.flags &DDSConvertInfo::Flags::NormalMap) != DDSConvertInfo::Flags::None;

	inputOptions.setGamma(convertInfo.inputGamma,convertInfo.outputGamma);
	inputOptions.setHeightEvaluation(convertInfo.heightEvaluation.redScale,convertInfo.heightEvaluation.greenScale,convertInfo.heightEvaluation.blueScale,convertInfo.heightEvaluation.alphaScale);
	inputOptions.setKaiserParameters(convertInfo.kaiserParameters.width,convertInfo.kaiserParameters.alpha,convertInfo.kaiserParameters.stretch);
	inputOptions.setMaxExtents(convertInfo.maxExtents);
	inputOptions.setMipmapFilter(convertInfo.mipmapFilter);
	inputOptions.setMipmapGeneration(bGenerateMipmaps,convertInfo.maxMipmapLevel);
	inputOptions.setRoundMode(convertInfo.roundMode);
	inputOptions.setWrapMode(convertInfo.wrapMode);
	inputOptions.setNormalMap(bNormalMap);
	inputOptions.setNormalizeMipmaps(bNormalizeMipmaps);
	inputOptions.setConvertToNormalMap(bConvertToNormalMap);
	if(convertInfo.normalFilter != nullptr)
		inputOptions.setNormalFilter(convertInfo.normalFilter->small,convertInfo.normalFilter->medium,convertInfo.normalFilter->big,convertInfo.normalFilter->large);
	if(convertInfo.textureLayout != nullptr)
		inputOptions.setTextureLayout(convertInfo.textureLayout->type,convertInfo.textureLayout->width,convertInfo.textureLayout->height,convertInfo.textureLayout->depth,convertInfo.textureLayout->arraySize);

	nvtt::OutputOptions outputOptions {};
	outputOptions.reset();
	outputOptions.setFileName(dstFile.c_str());

	nvtt::CompressionOptions compressionOptions {};
	compressionOptions.reset();
	compressionOptions.setFormat(nvtt::Format_DXT1);

	compressionOptions.setColorWeights(convertInfo.colorWeights.red,convertInfo.colorWeights.green,convertInfo.colorWeights.blue,convertInfo.colorWeights.alpha);
	compressionOptions.setFormat(convertInfo.format);
	compressionOptions.setPitchAlignment(convertInfo.pitchAlignment);
	compressionOptions.setQuality(convertInfo.quality);
	if(convertInfo.quantization != nullptr)
		compressionOptions.setQuantization(convertInfo.quantization->colorDithering,convertInfo.quantization->alphaDithering,convertInfo.quantization->binaryAlpha,convertInfo.quantization->alphaThreshold);
	if(convertInfo.pixelFormat0 != nullptr)
		compressionOptions.setPixelFormat(convertInfo.pixelFormat0->bitCount,convertInfo.pixelFormat0->rMask,convertInfo.pixelFormat0->gMask,convertInfo.pixelFormat0->bMask,convertInfo.pixelFormat0->aMask);
	if(convertInfo.pixelFormat1 != nullptr)
		compressionOptions.setPixelFormat(convertInfo.pixelFormat1->rSize,convertInfo.pixelFormat1->gSize,convertInfo.pixelFormat1->bSize,convertInfo.pixelFormat1->aSize);
	compressionOptions.setPixelType(convertInfo.pixelType);
	compressionOptions.setRGBMThreshold(convertInfo.rgbmThreshold);

	nvtt::Compressor compressor {};
	auto result = compressor.process(inputOptions,compressionOptions,outputOptions);
	Lua::PushBool(l,result);
	return 1;
}
void Lua::dds::register_lua_library(Lua::Interface &l)
{
	ishared::load_documentation_file("doc/module_dds.wdd");

	auto &modConvert = l.RegisterLibrary("convert",std::unordered_map<std::string,int32_t(*)(lua_State*)>{
		{"image_to_dds",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			return convert_dds(l);
		})}
	});

	auto defColorWeights = luabind::class_<DDSConvertInfo::ColorWeights>("ColorWeights");
	defColorWeights.def(luabind::constructor<>());
	defColorWeights.def_readwrite("red",&DDSConvertInfo::ColorWeights::red);
	defColorWeights.def_readwrite("green",&DDSConvertInfo::ColorWeights::green);
	defColorWeights.def_readwrite("blue",&DDSConvertInfo::ColorWeights::blue);
	defColorWeights.def_readwrite("alpha",&DDSConvertInfo::ColorWeights::alpha);

	auto defHeightEvaluation = luabind::class_<DDSConvertInfo::HeightEvaluation>("HeightEvaluation");
	defHeightEvaluation.def(luabind::constructor<>());
	defHeightEvaluation.def_readwrite("redScale",&DDSConvertInfo::HeightEvaluation::redScale);
	defHeightEvaluation.def_readwrite("greenScale",&DDSConvertInfo::HeightEvaluation::greenScale);
	defHeightEvaluation.def_readwrite("blueScale",&DDSConvertInfo::HeightEvaluation::blueScale);
	defHeightEvaluation.def_readwrite("alphaScale",&DDSConvertInfo::HeightEvaluation::alphaScale);
	
	auto defKaiserParameters = luabind::class_<DDSConvertInfo::KaiserParameters>("KaiserParameters");
	defKaiserParameters.def(luabind::constructor<>());
	defKaiserParameters.def_readwrite("width",&DDSConvertInfo::KaiserParameters::width);
	defKaiserParameters.def_readwrite("alpha",&DDSConvertInfo::KaiserParameters::alpha);
	defKaiserParameters.def_readwrite("stretch",&DDSConvertInfo::KaiserParameters::stretch);

	auto ddsConvertInfo = luabind::class_<DDSConvertInfo>("DDSConvertInfo");
	ddsConvertInfo.def(luabind::constructor<>());
	ddsConvertInfo.def("SetQuantization",static_cast<void(*)(lua_State*,DDSConvertInfo&,bool,bool,bool,int32_t)>([](
		lua_State *l,DDSConvertInfo &convertInfo,bool colorDithering,bool alphaDithering,bool binaryAlpha,int32_t alphaThreshold
	) {
		convertInfo.SetQuantization(colorDithering,alphaDithering,binaryAlpha,alphaThreshold);
	}));
	ddsConvertInfo.def("SetQuantization",static_cast<void(*)(lua_State*,DDSConvertInfo&,bool,bool,bool)>([](
		lua_State *l,DDSConvertInfo &convertInfo,bool colorDithering,bool alphaDithering,bool binaryAlpha
	) {
		convertInfo.SetQuantization(colorDithering,alphaDithering,binaryAlpha);
	}));
	ddsConvertInfo.def("SetPixelFormat",static_cast<void(*)(lua_State*,DDSConvertInfo&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](
		lua_State *l,DDSConvertInfo &convertInfo,uint32_t bitcount,uint32_t rmask,uint32_t gmask,uint32_t bmask,uint32_t amask
	) {
		convertInfo.SetPixelFormat(bitcount,rmask,gmask,bmask,amask);
	}));
	ddsConvertInfo.def("SetPixelFormat",static_cast<void(*)(lua_State*,DDSConvertInfo&,uint8_t,uint8_t,uint8_t,uint8_t)>([](
		lua_State *l,DDSConvertInfo &convertInfo,uint8_t rsize,uint8_t gsize,uint8_t bsize,uint8_t asize
	) {
		convertInfo.SetPixelFormat(rsize,gsize,bsize,asize);
	}));
	ddsConvertInfo.def("SetNormalFilter",static_cast<void(*)(lua_State*,DDSConvertInfo&,float,float,float,float)>([](
		lua_State *l,DDSConvertInfo &convertInfo,float small,float medium,float big,float large
	) {
		convertInfo.SetNormalFilter(small,medium,big,large);
	}));
	ddsConvertInfo.def("SetTextureLayout",static_cast<void(*)(lua_State*,DDSConvertInfo&,nvtt::TextureType,int,int,int,int)>([](
		lua_State *l,DDSConvertInfo &convertInfo,nvtt::TextureType type,int width,int height,int depth,int arraySize
	) {
		convertInfo.SetTextureLayout(type,width,height,depth,arraySize);
	}));
	ddsConvertInfo.def("SetTextureLayout",static_cast<void(*)(lua_State*,DDSConvertInfo&,nvtt::TextureType,int,int,int)>([](
		lua_State *l,DDSConvertInfo &convertInfo,nvtt::TextureType type,int width,int height,int depth
	) {
		convertInfo.SetTextureLayout(type,width,height,depth);
	}));
	ddsConvertInfo.def("SetTextureLayout",static_cast<void(*)(lua_State*,DDSConvertInfo&,nvtt::TextureType,int,int)>([](
		lua_State *l,DDSConvertInfo &convertInfo,nvtt::TextureType type,int width,int height
	) {
		convertInfo.SetTextureLayout(type,width,height);
	}));
	ddsConvertInfo.def_readwrite("flags",reinterpret_cast<std::underlying_type_t<decltype(DDSConvertInfo::flags)> DDSConvertInfo::*>(&DDSConvertInfo::flags));
	ddsConvertInfo.def_readwrite("inputGamma",&DDSConvertInfo::inputGamma);
	ddsConvertInfo.def_readwrite("outputGamma",&DDSConvertInfo::outputGamma);
	ddsConvertInfo.def_readwrite("heightEvaluation",&DDSConvertInfo::heightEvaluation);
	ddsConvertInfo.def_readwrite("kaiserParameters",&DDSConvertInfo::kaiserParameters);
	ddsConvertInfo.def_readwrite("maxExtents",&DDSConvertInfo::maxExtents);
	ddsConvertInfo.def_readwrite("mipmapFilter",&DDSConvertInfo::mipmapFilter);
	ddsConvertInfo.def_readwrite("maxMipmapLevel",&DDSConvertInfo::maxMipmapLevel);
	ddsConvertInfo.def_readwrite("roundMode",&DDSConvertInfo::roundMode);
	ddsConvertInfo.def_readwrite("wrapMode",&DDSConvertInfo::wrapMode);
	ddsConvertInfo.def_readwrite("colorWeights",&DDSConvertInfo::colorWeights);
	ddsConvertInfo.def_readwrite("format",&DDSConvertInfo::format);
	ddsConvertInfo.def_readwrite("pitchAlignment",&DDSConvertInfo::pitchAlignment);
	ddsConvertInfo.def_readwrite("rgbmThreshold",&DDSConvertInfo::rgbmThreshold);
	ddsConvertInfo.def_readwrite("quality",&DDSConvertInfo::quality);
	ddsConvertInfo.def_readwrite("pixelType",&DDSConvertInfo::pixelType);
	ddsConvertInfo.add_static_constant("CONVERSION_FLAG_NONE",umath::to_integral(DDSConvertInfo::Flags::None));
	ddsConvertInfo.add_static_constant("CONVERSION_FLAG_BIT_CONVERT_TO_NORMAL_MAP",umath::to_integral(DDSConvertInfo::Flags::ConvertToNormalMap));
	ddsConvertInfo.add_static_constant("CONVERSION_FLAG_BIT_GENERATE_MIPMAPS",umath::to_integral(DDSConvertInfo::Flags::GenerateMipmaps));
	ddsConvertInfo.add_static_constant("CONVERSION_FLAG_BIT_NORMALIZE_MIPMAPS",umath::to_integral(DDSConvertInfo::Flags::NormalizeMipmaps));
	ddsConvertInfo.add_static_constant("CONVERSION_FLAG_BIT_NORMAL_MAP",umath::to_integral(DDSConvertInfo::Flags::NormalMap));

	ddsConvertInfo.add_static_constant("MIPMAP_FILTER_BOX",nvtt::MipmapFilter::MipmapFilter_Box);
	ddsConvertInfo.add_static_constant("MIPMAP_FILTER_TRIANGLE",nvtt::MipmapFilter::MipmapFilter_Triangle);
	ddsConvertInfo.add_static_constant("MIPMAP_FILTER_KAISER",nvtt::MipmapFilter::MipmapFilter_Kaiser);

	ddsConvertInfo.add_static_constant("ROUND_MODE_NONE",nvtt::RoundMode::RoundMode_None);
	ddsConvertInfo.add_static_constant("ROUND_MODE_TO_NEXT_POWER_OF_TWO",nvtt::RoundMode::RoundMode_ToNextPowerOfTwo);
	ddsConvertInfo.add_static_constant("ROUND_MODE_TO_NEAREST_POWER_OF_TWO",nvtt::RoundMode::RoundMode_ToNearestPowerOfTwo);
	ddsConvertInfo.add_static_constant("ROUND_MODE_TO_PREVIOUS_POWER_OF_TWO",nvtt::RoundMode::RoundMode_ToPreviousPowerOfTwo);
	ddsConvertInfo.add_static_constant("ROUND_MODE_TO_NEXT_MULTIPLE_OF_FOUR",nvtt::RoundMode::RoundMode_ToNextMultipleOfFour);
	ddsConvertInfo.add_static_constant("ROUND_MODE_TO_NEAREST_MULTIPLE_OF_FOUR",nvtt::RoundMode::RoundMode_ToNearestMultipleOfFour);
	ddsConvertInfo.add_static_constant("ROUND_MODE_TO_PREVIOUS_MULTIPLE_OF_FOUR",nvtt::RoundMode::RoundMode_ToPreviousMultipleOfFour);

	ddsConvertInfo.add_static_constant("WRAP_MODE_CLAMP",nvtt::WrapMode::WrapMode_Clamp);
	ddsConvertInfo.add_static_constant("WRAP_MODE_REPEAT",nvtt::WrapMode::WrapMode_Repeat);
	ddsConvertInfo.add_static_constant("WRAP_MODE_MIRROR",nvtt::WrapMode::WrapMode_Mirror);

	ddsConvertInfo.add_static_constant("QUALITY_FASTEST",nvtt::Quality::Quality_Fastest);
	ddsConvertInfo.add_static_constant("QUALITY_NORMAL",nvtt::Quality::Quality_Normal);
	ddsConvertInfo.add_static_constant("QUALITY_PRODUCTION",nvtt::Quality::Quality_Production);
	ddsConvertInfo.add_static_constant("QUALITY_HIGHEST",nvtt::Quality::Quality_Highest);

	ddsConvertInfo.add_static_constant("FORMAT_RGB",nvtt::Format::Format_RGB);
	ddsConvertInfo.add_static_constant("FORMAT_RGBA",nvtt::Format::Format_RGBA);
	ddsConvertInfo.add_static_constant("FORMAT_DXT1",nvtt::Format::Format_DXT1);
	ddsConvertInfo.add_static_constant("FORMAT_DXT1A",nvtt::Format::Format_DXT1a);
	ddsConvertInfo.add_static_constant("FORMAT_DXT3",nvtt::Format::Format_DXT3);
	ddsConvertInfo.add_static_constant("FORMAT_DXT5",nvtt::Format::Format_DXT5);
	ddsConvertInfo.add_static_constant("FORMAT_DXT5N",nvtt::Format::Format_DXT5n);
	ddsConvertInfo.add_static_constant("FORMAT_BC1",nvtt::Format::Format_BC1);
	ddsConvertInfo.add_static_constant("FORMAT_BC1A",nvtt::Format::Format_BC1a);
	ddsConvertInfo.add_static_constant("FORMAT_BC2",nvtt::Format::Format_BC2);
	ddsConvertInfo.add_static_constant("FORMAT_BC3",nvtt::Format::Format_BC3);
	ddsConvertInfo.add_static_constant("FORMAT_BC3N",nvtt::Format::Format_BC3n);
	ddsConvertInfo.add_static_constant("FORMAT_BC4",nvtt::Format::Format_BC4);
	ddsConvertInfo.add_static_constant("FORMAT_BC5",nvtt::Format::Format_BC5);
	ddsConvertInfo.add_static_constant("FORMAT_DXT1N",nvtt::Format::Format_DXT1n);
	ddsConvertInfo.add_static_constant("FORMAT_CTX1",nvtt::Format::Format_CTX1);
	ddsConvertInfo.add_static_constant("FORMAT_BC6",nvtt::Format::Format_BC6);
	ddsConvertInfo.add_static_constant("FORMAT_BC7",nvtt::Format::Format_BC7);
	ddsConvertInfo.add_static_constant("FORMAT_BC3_RGBM",nvtt::Format::Format_BC3_RGBM);
	ddsConvertInfo.add_static_constant("FORMAT_ETC1",nvtt::Format::Format_ETC1);
	ddsConvertInfo.add_static_constant("FORMAT_ETC2_R",nvtt::Format::Format_ETC2_R);
	ddsConvertInfo.add_static_constant("FORMAT_ETC2_RG",nvtt::Format::Format_ETC2_RG);
	ddsConvertInfo.add_static_constant("FORMAT_ETC2_RGB",nvtt::Format::Format_ETC2_RGB);
	ddsConvertInfo.add_static_constant("FORMAT_ETC2_RGBA",nvtt::Format::Format_ETC2_RGBA);
	ddsConvertInfo.add_static_constant("FORMAT_ETC2_RGB_A1",nvtt::Format::Format_ETC2_RGB_A1);
	ddsConvertInfo.add_static_constant("FORMAT_ETC2_RGBM",nvtt::Format::Format_ETC2_RGBM);
	ddsConvertInfo.add_static_constant("FORMAT_PVR_2BPP_RGB",nvtt::Format::Format_PVR_2BPP_RGB);
	ddsConvertInfo.add_static_constant("FORMAT_PVR_4BPP_RGB",nvtt::Format::Format_PVR_4BPP_RGB);
	ddsConvertInfo.add_static_constant("FORMAT_PVR_2BPP_RGBA",nvtt::Format::Format_PVR_2BPP_RGBA);
	ddsConvertInfo.add_static_constant("FORMAT_PVR_4BPP_RGBA",nvtt::Format::Format_PVR_4BPP_RGBA);
	ddsConvertInfo.add_static_constant("FORMAT_COUNT",nvtt::Format::Format_Count);

	ddsConvertInfo.add_static_constant("PIXEL_TYPE_UNSIGNED_NORM",nvtt::PixelType::PixelType_UnsignedNorm);
	ddsConvertInfo.add_static_constant("PIXEL_TYPE_SIGNED_NORM",nvtt::PixelType::PixelType_SignedNorm);
	ddsConvertInfo.add_static_constant("PIXEL_TYPE_UNSIGNED_INT",nvtt::PixelType::PixelType_UnsignedInt);
	ddsConvertInfo.add_static_constant("PIXEL_TYPE_SIGNED_INT",nvtt::PixelType::PixelType_SignedInt);
	ddsConvertInfo.add_static_constant("PIXEL_TYPE_FLOAT",nvtt::PixelType::PixelType_Float);
	ddsConvertInfo.add_static_constant("PIXEL_TYPE_UNSIGNED_FLOAT",nvtt::PixelType::PixelType_UnsignedFloat);
	ddsConvertInfo.add_static_constant("PIXEL_TYPE_SHARED_EXP",nvtt::PixelType::PixelType_SharedExp);

	ddsConvertInfo.add_static_constant("IMAGE_FORMAT_TGA",umath::to_integral(uimg::ImageFormat::TGA));
	ddsConvertInfo.add_static_constant("IMAGE_FORMAT_PNG",umath::to_integral(uimg::ImageFormat::PNG));

	ddsConvertInfo.scope[defColorWeights];
	ddsConvertInfo.scope[defHeightEvaluation];
	ddsConvertInfo.scope[defKaiserParameters];
	modConvert[ddsConvertInfo];
}

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		Lua::dds::register_lua_library(l);
	}
};
#pragma optimize("",on)
