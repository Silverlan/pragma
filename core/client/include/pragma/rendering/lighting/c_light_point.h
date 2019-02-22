#ifndef __C_LIGHT_POINT_H__
#define __C_LIGHT_POINT_H__

// TODO: Remove this file
#if 0
#include "pragma/rendering/lighting/c_light.h"
#include "pragma/rendering/lighting/c_light_ranged.h"

class DLLCLIENT CLightPoint
	: public CLightBase,public MVPBias<6>,public CLightRanged
{
protected:
	void InitializeShadowMap();
	bool bSkipUpdate;
	virtual void UpdateTransformationMatrix(unsigned int j) override;
	virtual void ScheduleRender() override;
	virtual void UpdateViewMatrices();
	virtual bool ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags) override;
	virtual bool ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags) override;
public:
	CLightPoint(const Vector3 &pos,const Color &col,float distance,ShadowType shadowType=ShadowType::Full);
	virtual Mat4 &GetTransformationMatrix(unsigned int j) override;
	virtual void Initialize() override;
	virtual LightType GetType() const override;
	virtual float GetDistance() const override;
	virtual void SetDistance(float distance) override;
	virtual void SetPosition(const Vector3 &pos) override;
	virtual const Vector3 &GetPosition() const override;
	virtual bool ShouldRender() override;

	using CLightBase::GetDistance;
	using CLightBase::SetDistance;
};

#endif
#endif
