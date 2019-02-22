#ifndef __WORLD_H__
#define __WORLD_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/baseworld.h>

namespace pragma
{
	class DLLSERVER SWorldComponent final
		: public BaseWorldComponent
	{
	public:
		SWorldComponent(BaseEntity &ent) : BaseWorldComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class SPolyMesh;
class DLLSERVER World
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
	virtual bool IsWorld() const override {return true;} // TODO: Move this to component
};

#endif