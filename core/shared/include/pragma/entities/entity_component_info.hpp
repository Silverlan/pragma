#ifndef __ENTITY_COMPONENT_INFO_HPP__
#define __ENTITY_COMPONENT_INFO_HPP__

namespace pragma
{
	using ComponentId = uint32_t;
	using ComponentEventId = uint32_t;
	const auto INVALID_COMPONENT_ID = std::numeric_limits<ComponentId>::max();
};

#endif
