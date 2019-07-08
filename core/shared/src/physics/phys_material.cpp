#include "pragma/physics/phys_material.hpp"

void pragma::physics::IMaterial::SetFriction(Float friction)
{
	SetStaticFriction(friction);
	SetDynamicFriction(friction);
}
void pragma::physics::IMaterial::SetSurfaceMaterial(SurfaceMaterial &surfMat) {m_surfaceMaterial = &surfMat;}
SurfaceMaterial *pragma::physics::IMaterial::GetSurfaceMaterial() const {return m_surfaceMaterial;}
void pragma::physics::IMaterial::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IMaterial>(lua);
}
