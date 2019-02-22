#ifndef __BASE_GENERIC_COMPONENT_HPP__
#define __BASE_GENERIC_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK BaseGenericComponent
		: public BaseEntityComponent
	{
	protected:
		BaseGenericComponent(BaseEntity &ent);
	};
};

#endif
