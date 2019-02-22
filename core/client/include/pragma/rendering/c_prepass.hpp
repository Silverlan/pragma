#ifndef __C_PREPASS_HPP__
#define __C_PREPASS_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <sharedutils/util_weak_handle.hpp>

namespace Anvil {class PrimaryCommandBuffer;};
namespace prosper {class Context; class RenderTarget; class Texture; class Shader; class PrimaryCommandBuffer;};
#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma
{
	class ShaderPrepassBase;
	namespace rendering
	{
		class DLLCLIENT Prepass
		{
		public:
			bool Initialize(prosper::Context &context,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits sampleCount,bool bExtended);
			pragma::ShaderPrepassBase &GetShader() const;
			void BeginRenderPass(prosper::PrimaryCommandBuffer &cmdBuffer);
			void EndRenderPass(prosper::PrimaryCommandBuffer &cmdBuffer);
			std::shared_ptr<prosper::Texture> textureNormals = nullptr;

			// Required for SSAO
			std::shared_ptr<prosper::Texture> textureDepth = nullptr;

			// Depth buffer used for sampling (e.g. particle render pass)
			std::shared_ptr<prosper::Texture> textureDepthSampled = nullptr;

			std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;

			void SetUseExtendedPrepass(bool b,bool bForceReload=false);
			bool IsExtended() const;
		private:
			bool m_bExtended = false;
			std::vector<vk::ClearValue> m_clearValues = {};
			util::WeakHandle<prosper::Shader> m_shaderPrepass = {};
			util::WeakHandle<prosper::Shader> m_shaderPrepassDepth = {};
		};
	};
};
#pragma warning(pop)

#endif
