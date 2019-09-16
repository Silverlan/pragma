#ifndef __C_SETTINGS_HPP__
#define __C_SETTINGS_HPP__

namespace pragma::rendering
{
	enum class AntiAliasing : uint8_t
	{
		None = 0u,
		MSAA,
		FXAA
	};

	enum class ToneMapping : uint32_t
	{
		GammaCorrection = 0u,
		Reinhard,
		HejilRichard,
		Uncharted,
		Aces,
		GranTurismo
	};
};

#endif
