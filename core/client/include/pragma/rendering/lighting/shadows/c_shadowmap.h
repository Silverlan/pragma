#ifndef __C_SHADOWMAP_H__
#define __C_SHADOWMAP_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include "pragma/indexmap.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap_depth_buffer_manager.hpp"

namespace prosper {class Framebuffer; class RenderPass;};
class DLLCLIENT ShadowMap
{
public:
	enum class DLLCLIENT Type : uint32_t
	{
		Generic = 1,
		Cube = 2,
		Cascaded = 3
	};
private:
	static IndexMap<ShadowMap> m_shadowMaps;
	static ShadowMapDepthBufferManager s_shadowBufferManager;
public:
	static IndexMap<ShadowMap> &GetAll();
	static void InitializeDescriptorSet();
	static Anvil::DescriptorSet *GetDescriptorSet();
	static void ClearShadowMapDepthBuffers();
protected:
	virtual void DestroyTextures();
	unsigned int m_index;
	uint32_t m_layerCount;
	bool m_bUseDualTextureSet;
	bool m_bValid;
	std::weak_ptr<ShadowMapDepthBufferManager::RenderTarget> m_shadowRt = {};
	std::function<void(void)> m_onTexturesReloaded;
	virtual void GenerateTextures();
	virtual void ApplyTextureParameters();
	virtual void InitializeDepthTextures(uint32_t size);
public:
	ShadowMap(uint32_t layerCount=1,bool bUseDualTextureSet=true);
	virtual ~ShadowMap();
	virtual void Initialize();
	virtual void ReloadDepthTextures();
	virtual Type GetType();
	virtual bool ShouldUpdateLayer(uint32_t layerId) const;
	uint32_t GetLayerCount() const;
	bool IsValid() const;
	void SetTextureReloadCallback(const std::function<void(void)> &f);

	const std::shared_ptr<prosper::RenderTarget> &GetDepthRenderTarget() const;
	virtual const std::shared_ptr<prosper::Texture> &GetDepthTexture() const;
	const std::shared_ptr<prosper::RenderPass> &GetRenderPass() const;
	const std::shared_ptr<prosper::Framebuffer> &GetFramebuffer(uint32_t layerId=0u);

	std::weak_ptr<ShadowMapDepthBufferManager::RenderTarget> RequestRenderTarget();
	virtual void FreeRenderTarget();
	bool HasRenderTarget() const;
};

#endif