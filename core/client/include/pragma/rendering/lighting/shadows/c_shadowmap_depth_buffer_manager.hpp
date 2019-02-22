#ifndef __C_SHADOWMAP_DEPTH_BUFFER_MANAGER_HPP__
#define __C_SHADOWMAP_DEPTH_BUFFER_MANAGER_HPP__

#include "pragma/clientdefinitions.h"
#include <queue>

namespace prosper
{
	class Texture;
};
namespace Anvil
{
	class RenderPass;
	class Framebuffer;
	class DescriptorSet;
	class DescriptorSetGroup;
};
class DLLCLIENT ShadowMapDepthBufferManager
{
public:
	ShadowMapDepthBufferManager();
	void Initialize();
	void Clear();
	struct RenderTarget
	{
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		uint32_t index = std::numeric_limits<uint32_t>::max();
	};
	enum class Type : uint32_t
	{
		Generic = 0,
		Cube
	};
	std::weak_ptr<RenderTarget> RequestRenderTarget(Type type,uint32_t size);
	void FreeRenderTarget(const RenderTarget &rt);
	Anvil::DescriptorSet *GetDescriptorSet();
	void ClearRenderTargets();
private:
	struct BufferSet
	{
		uint32_t limit = 0;
		std::vector<std::shared_ptr<RenderTarget>> buffers;
		std::queue<std::size_t> freeBuffers;
	};
	BufferSet m_genericSet = {};
	BufferSet m_cubeSet = {};
	std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroup = nullptr;
	util::WeakHandle<prosper::Shader> m_whShadowShader = {};
};

#endif
