#ifndef __S_GENERIC_COMPONENT_HPP__
#define __S_GENERIC_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_generic_component.hpp>

namespace pragma
{
	class DLLSERVER SGenericComponent final
		: public BaseGenericComponent
	{
	public:
		SGenericComponent(BaseEntity &ent) : BaseGenericComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
