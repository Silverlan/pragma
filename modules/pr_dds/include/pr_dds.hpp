#ifndef __PR_DDS_HPP__
#define __PR_DDS_HPP__

#include <mathutil/umath.h>
#include <sharedutils/util_library.hpp>

#ifndef DLLSPEC_IDDS
#define DLLSPEC_IDDS
#endif

struct ImageWriteInfo
{
	enum class InputFormat : uint8_t
	{
		KeepInputImageFormat = 0u,
		R8G8B8A8_UInt,
		R16G16B16A16_Float,
		R32G32B32A32_Float,
		R32_Float
	};
	enum class OutputFormat : uint8_t
	{
		KeepInputImageFormat,
		RGB,
		RGBA,
		DXT1,
		DXT1a,
		DXT3,
		DXT5,
		DXT5n,
		BC1,
		BC1a,
		BC2,
		BC3,
		BC3n,
		BC4,
		BC5,
		DXT1n,
		CTX1,
		BC6,
		BC7,
		BC3_RGBM,
		ETC1,
		ETC2_R,
		ETC2_RG,
		ETC2_RGB,
		ETC2_RGBA,
		ETC2_RGB_A1,
		ETC2_RGBM,

		Count,

		// Helper types for common use-cases
		ColorMap = DXT1,
		ColorMap1BitAlpha = DXT1a,
		ColorMapSharpAlpha = DXT3,
		ColorMapSmoothAlpha = DXT5,
		NormalMap = DXT5n, // TODO: BC5 might be a better choice, but is not supported by cycles!
		HDRColorMap = BC6,
		GradientMap = BC1 // TODO: Is this a good idea?
	};
	enum class ContainerFormat : uint8_t
	{
		DDS = 0u,
		KTX,

		Count
	};
	enum class Flags : uint32_t
	{
		None = 0u,
		NormalMap = 1u,
		ConvertToNormalMap = NormalMap<<1u,
		SRGB = ConvertToNormalMap<<1u,
		GenerateMipmaps = SRGB<<1u
	};
	enum class MipmapFilter : uint8_t
	{
		Box = 0u,
		Kaiser
	};
	enum class WrapMode : uint8_t
	{
		Clamp = 0u,
		Repeat,
		Mirror
	};
	InputFormat inputFormat = InputFormat::R8G8B8A8_UInt;
	OutputFormat outputFormat = OutputFormat::BC3;
	ContainerFormat containerFormat = ContainerFormat::KTX;
	Flags flags = Flags::None;
	MipmapFilter mipMapFilter = MipmapFilter::Box;
	WrapMode wrapMode = WrapMode::Mirror;
	void SetNormalMap()
	{
		flags = static_cast<Flags>(umath::to_integral(flags) | umath::to_integral(Flags::NormalMap));
		outputFormat = OutputFormat::BC5;
		mipMapFilter = MipmapFilter::Kaiser;
	}
};
REGISTER_BASIC_BITWISE_OPERATORS(ImageWriteInfo::Flags)

struct DLLSPEC_IDDS IDDS final
{
	IDDS(util::Library &lib)
		: initialize{lib.FindSymbolAddress<decltype(initialize)>("pr_dds_initialize")}
	{
		m_bValid = initialize != nullptr;
	}
	IDDS()=default;
	bool(* const initialize)() = nullptr;

	bool valid() const {return m_bValid;}
private:
	bool m_bValid = false;
};

#endif
