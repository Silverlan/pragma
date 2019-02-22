#ifndef __C_SHADOWMAPCASC_H__
#define __C_SHADOWMAPCASC_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include <pragma/math/plane.h>
#include "pragma/entities/environment/lights/c_env_light.h"
#include <mathutil/boundingvolume.h>

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT FrustumSplit
{
	FrustumSplit();
	float neard;
	float fard;
};

struct DLLCLIENT Frustum
{
	Frustum();
	FrustumSplit split = {};
	std::vector<Vector3> points;
	std::vector<Plane> planes;
	Vector3 bounds[2] = {{},{}};
	Mat4 projection = umat::identity();
	Mat4 viewProjection = umat::identity();
	Vector3 center = {};
	bounding_volume::OBB obb = {};
	bounding_volume::AABB aabb = {};
	Vector3 obbCenter = {}; // Also center of aabb
	float radius = 0.f;
};

class DLLCLIENT ShadowMapCasc
	: public ShadowMap
{
public:
	static const uint32_t MAX_CASCADE_COUNT = 4;
public:
	ShadowMapCasc();
	ShadowMap::Type GetType() override;
	virtual void ReloadDepthTextures() override;
	virtual bool ShouldUpdateLayer(uint32_t layerId) const override;
	Mat4 &GetProjectionMatrix(unsigned int layer);
	Mat4 &GetViewProjectionMatrix(unsigned int layer);
	void SetFrustumUpdateCallback(const std::function<void(void)> &f);
	void SetSplitCount(unsigned int numSplits);
	void SetMaxDistance(float dist);
	float GetMaxDistance();
	unsigned int GetSplitCount();
	void UpdateFrustum(uint32_t splitId,Camera &cam,const Mat4 &matView,const Vector3 &dir);
	void UpdateFrustum(Camera &cam,const Mat4 &matView,const Vector3 &dir);
	float *GetSplitFarDistances();
	Frustum *GetFrustumSplit(unsigned int splitId);
	//const Vulkan::DescriptorSet &GetDescriptorSet() const; // prosper TODO
	//const Vulkan::RenderPass &GetRenderPassKeep() const; // prosper TODO
	//const Vulkan::Framebuffer &GetFramebufferKeep(uint32_t layer=0) const; // prosper TODO
	//const std::vector<Vulkan::Framebuffer> &GetFramebuffersKeep() const; // prosper TODO

	const Mat4 &GetStaticPendingViewProjectionMatrix(uint32_t layer) const;
	//const std::shared_ptr<prosper::Framebuffer> &GetStaticPendingFramebuffer(uint32_t layer=0) const;
	//const std::vector<Vulkan::Framebuffer> &GetStaticPendingFramebuffers() const; // prosper TODO
	//const std::shared_ptr<prosper::RenderPass> &GetStaticPendingRenderPass() const;
	//const std::shared_ptr<prosper::Texture> &GetStaticPendingDepthTexture() const;
	const std::shared_ptr<prosper::RenderTarget> &GetStaticPendingRenderTarget() const;
	void RenderBatch(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightDirectionalComponent &light);

	//const Vulkan::Framebuffer &GetFramebuffer(CLightBase::RenderPass rp,uint32_t layer=0) const; // prosper TODO
	//const std::vector<Vulkan::Framebuffer> &GetFramebuffers(CLightBase::RenderPass rp) const; // prosper TODO
	//const Vulkan::RenderPass &GetRenderPass(CLightBase::RenderPass rp) const; // prosper TODO
	//const Vulkan::Texture &GetDepthTexture(CLightBase::RenderPass rp) const; // prosper TODO
	//virtual const Vulkan::Texture *GetDepthTexture() const override; // prosper TODO
	const std::shared_ptr<prosper::Framebuffer> &GetFramebuffer(pragma::CLightComponent::RenderPass rp,uint32_t layer=0) const;
	//const std::vector<std::shared_ptr<prosper::Framebuffer>> &GetFramebuffers(CLightBase::RenderPass rp) const;
	const std::shared_ptr<prosper::RenderPass> &GetRenderPass(pragma::CLightComponent::RenderPass rp) const;
	const std::shared_ptr<prosper::RenderTarget> &GetRenderTarget(pragma::CLightComponent::RenderPass rp) const;

	virtual const std::shared_ptr<prosper::Texture> &GetDepthTexture() const override;
	const std::shared_ptr<prosper::Texture> &GetDepthTexture(pragma::CLightComponent::RenderPass rp) const;

	virtual void FreeRenderTarget() override;

	bool IsDynamicValid() const;
	uint64_t GetLastUpdateFrameId() const;
	void SetLastUpdateFrameId(uint64_t id);
protected:
	void DestroyTextures() override;
	std::array<bool,MAX_CASCADE_COUNT> m_layerUpdate;
	std::array<Mat4,MAX_CASCADE_COUNT> m_vpMatrices;

	struct EntityInfo
	{
		EntityInfo()=default;
		EntityInfo(CBaseEntity *ent)
			: hEntity(ent->GetHandle())
		{}
		EntityHandle hEntity = {};
		std::queue<std::weak_ptr<ModelMesh>> meshes;
		bool bAlreadyPassed = false;
		//uint32_t meshGroupId = 0;
		//uint32_t meshId = 0;
		//uint32_t subMeshId = 0;
	};
	struct TranslucentEntityInfo
	{
		EntityHandle hEntity = {};
		std::queue<std::weak_ptr<ModelSubMesh>> subMeshes;;
	};
	struct CascadeMeshInfo
	{
		std::vector<EntityInfo> entityMeshes;
		std::vector<TranslucentEntityInfo> translucentMeshes;
	};
	struct TextureSet
	{
		TextureSet();
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
	};
	struct {
		TextureSet staticTextureSet;
		std::array<Mat4,MAX_CASCADE_COUNT> prevVpMatrices;
		std::array<Mat4,MAX_CASCADE_COUNT> prevProjectionMatrices;
		Vector4 prevSplitDistances = {};
		std::array<CascadeMeshInfo,MAX_CASCADE_COUNT> meshes;
	} m_pendingInfo;

	util::WeakHandle<prosper::Shader> m_whShaderCsm = {};
	util::WeakHandle<prosper::Shader> m_whShaderCsmTransparent = {};
	std::vector<Frustum> m_frustums;
	std::vector<float> m_fard;
	std::function<void(void)> m_onFrustumUpdated;
	unsigned int m_numSplits = 0u;
	float m_maxDistance = 0.f;
	uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
	//Vulkan::DescriptorSet m_descSet; // prosper TODO
	//Vulkan::RenderPass m_renderPassKeep; // prosper TODO
	//std::vector<Vulkan::Framebuffer> m_framebuffersKeep; // prosper TODO
	std::array<TextureSet,2> m_textureSets; // 0 = Used for static geometry (Rarely moving and not animated); 1 = Used for dynamic geometry

	//void GenerateTextures();
	void UpdateSplitDistances(float nd,float fd);
	void InitializeTextureSet(TextureSet &set,pragma::CLightComponent::RenderPass rp);
	virtual void InitializeDepthTextures(uint32_t size) override;
	//void ApplyTextureParameters();
};
#pragma warning(pop)

typedef ShadowMapCasc CSM;

#endif