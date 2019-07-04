#include "pragma/physics/phys_material.hpp"

void pragma::physics::IMaterial::SetFriction(Float friction)
{
	SetStaticFriction(friction);
	SetDynamicFriction(friction);
}
