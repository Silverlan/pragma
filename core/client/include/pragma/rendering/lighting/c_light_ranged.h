#ifndef __C_LIGHT_RANGED_H__
#define __C_LIGHT_RANGED_H__

// TODO: Remove this file
#if 0
#include "pragma/clientdefinitions.h"

class UniformBlockWriter;
class DLLCLIENT CLightRanged
{
protected:
	CLightRanged();
	virtual ~CLightRanged()=default;
	double m_tLastRender;
	bool m_bScheduleRender;
	bool IsInCone(const CBaseEntity &ent,const Vector3 &dir,float angle) const;
	virtual bool ShouldPass(const CBaseEntity &ent);
	virtual bool ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags);
	bool IsRenderScheduled() const;
	void SetRenderScheduled(bool b);
	virtual void ScheduleRender();
public:
	virtual float GetDistance() const=0;
	virtual void SetDistance(float distance)=0;
	virtual const Vector3 &GetPosition() const=0;
	virtual bool ShouldRender();
};
#endif

#endif