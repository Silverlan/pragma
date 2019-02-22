#ifndef __C_LIGHT_DIRECTIONAL_H__
#define __C_LIGHT_DIRECTIONAL_H__

// TODO: Remove this file
#if 0
#include "pragma/rendering/lighting/c_light.h"
#include "pragma/rendering/lighting/c_light_ranged.h"

class DLLCLIENT CLightDirectional
	: public CLight,public CLightRanged
{
protected:
	bool m_bShadowBufferUpdateScheduled;
	void InitializeShadowMap();
	//std::vector<Vulkan::SwapCommandBuffer> m_cmdShadowBuffers; // prosper TODO
	virtual void UpdateEntity(CBaseEntity *ent) override;
	virtual void ScheduleRender() override;
	void RenderStaticWorldGeometry();
public:
	CLightDirectional(const Vector3 &pos,const Color &col,const Vector3 &dir,ShadowType shadowType=ShadowType::Full);
	virtual void TurnOn() override;
	virtual void TurnOff() override;
	void UpdateFrustum(uint32_t frustumId);
	void UpdateFrustum();
	virtual LightType GetType() const override;
	using CLightBase::GetDirection;
	//void SetDirection(const Vector3 &dir);
	virtual void Initialize() override;
	virtual bool ShouldUpdateRenderPass(RenderPass rp) const override;
	virtual void Think() override;
	virtual float GetDistance() const override;
	virtual void SetDistance(float distance) override;
	virtual const Vector3 &GetPosition() const override;
	virtual void SetDirection(const Vector3 &dir) override;
	virtual bool ShouldRender() override;
	void ReloadShadowCommandBuffers();
	//const std::vector<Vulkan::SwapCommandBuffer> &GetShadowCommandBuffers() const; // prosper TODO
	//const Vulkan::SwapCommandBuffer *GetShadowCommandBuffer(uint32_t layer) const; // prosper TODO
	using CLight::ShouldPass;
	bool ShouldPass(uint32_t layer,const Vector3 &min,const Vector3 &max);
	virtual bool ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags) override;
	virtual bool ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags) override;
};

#endif
#endif
