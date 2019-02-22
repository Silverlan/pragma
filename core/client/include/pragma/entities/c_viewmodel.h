#ifndef __C_VIEWMODEL_H__
#define __C_VIEWMODEL_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"

namespace pragma
{
	class DLLCLIENT CViewModelComponent final
		: public BaseEntityComponent
	{
	public:
		CViewModelComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		void SetViewModelOffset(const Vector3 &offset);
		const Vector3 &GetViewModelOffset() const;
		void SetViewFOV(float fov);
		float GetViewFOV() const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		Vector3 m_viewModelOffset;
		float m_viewFov = std::numeric_limits<float>::quiet_NaN();
	};
};

class DLLCLIENT CViewModel
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif