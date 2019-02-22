#ifndef __C_BSP_COMPONENT_HPP__
#define __C_BSP_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace bsp {class File;};
namespace pragma
{
	class DLLCLIENT CBSPComponent final
		: public BaseEntityComponent
	{
	public:
		CBSPComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		void InitializeBSPTree(bsp::File &bsp);
	};
};

#endif
