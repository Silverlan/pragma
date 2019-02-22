#ifndef __C_SHADER_POSTPROCESSING_H__
#define __C_SHADER_POSTPROCESSING_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader.h"
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT PostProcessing
		: public Base
	{
	protected:
		virtual void InitializeShader() override;
		MaterialHandle m_matNoise;
		std::string m_texNoise;
		void ReloadNightVisionNoiseTexture();
	public:
		PostProcessing();
		void Render(unsigned int frameBuffer,unsigned int screenTexture);
		void SetNightVisionEnabled(bool b);
		void SetNightVisionLuminanceThreshold(float threshold);
		void SetNightVisionColorAmplification(float amplification);
		void SetNightVisionNoiseTexture(const std::string &tex);
	};
};
#endif
#endif