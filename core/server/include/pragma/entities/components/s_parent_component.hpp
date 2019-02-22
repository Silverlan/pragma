#ifndef __S_PARENT_COMPONENT_HPP__
#define __S_PARENT_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_parent_component.hpp>

namespace pragma
{
	class DLLSERVER SParentComponent final
		: public BaseParentComponent
	{
	public:
		SParentComponent(BaseEntity &ent) : BaseParentComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
