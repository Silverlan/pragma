#ifndef __PHYS_RAYCAST_FILTER_HPP__
#define __PHYS_RAYCAST_FILTER_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>

enum class RayCastHitType : uint8_t;
namespace pragma::physics
{
	class IShape;
	class IRigidBody;

	class DLLNETWORK IRayCastFilterCallback
	{
	public:
		virtual RayCastHitType PreFilter(pragma::physics::IShape &shape,pragma::physics::IRigidBody &rigidBody) const=0;
		virtual RayCastHitType PostFilter(pragma::physics::IShape &shape,pragma::physics::IRigidBody &rigidBody) const=0;
		virtual bool HasPreFilter() const=0;
		virtual bool HasPostFilter() const=0;
		virtual ~IRayCastFilterCallback()=default;
	};
};

#endif
