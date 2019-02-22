#ifndef __C_LIGHT_SPOT_H__
#define __C_LIGHT_SPOT_H__

// TODO: Remove this file
#if 0
#include "pragma/rendering/lighting/c_light.h"
#include "pragma/rendering/lighting/c_light_ranged.h"

#define LIGHT_SPOT_BUFFER_CUTOFF_OUTER (LIGHT_RANGED_BUFFER_OFFSET_DIRECTION +1)
#define LIGHT_SPOT_BUFFER_CUTOFF_INNER (LIGHT_SPOT_BUFFER_CUTOFF_OUTER +1)
#define LIGHT_SPOT_BUFFER_CUTOFF_ATTENUATION (LIGHT_SPOT_BUFFER_CUTOFF_INNER +1)

class DLLCLIENT CLightSpot
	: public CLight,public CLightRanged
{
protected:
	virtual void ScheduleRender() override;
	virtual void UpdateViewMatrices() override;
	void UpdateProjectionMatrix();
	virtual bool ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags) override;
	virtual bool ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags) override;
public:
	virtual LightType GetType() const override;
	virtual float GetDistance() const override;
	virtual void SetDistance(float distance) override;
	virtual void SetOuterCutoffAngle(float ang) override;
	using CLightBase::GetOuterCutoffAngle;
	using CLightBase::GetInnerCutoffAngle;
	using CLightBase::GetDirection;
	using CLightBase::GetPosition;
	virtual void Initialize() override;
	virtual void SetDirection(const Vector3 &dir) override;
	virtual const Vector3 &GetPosition() const override;
	virtual void SetPosition(const Vector3 &pos) override;
	virtual bool ShouldRender() override;
};

#endif
#endif