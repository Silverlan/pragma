#ifndef __LPHYSICS_H__
#define __LPHYSICS_H__
#include "pragma/networkdefinitions.h"
#include <pragma/definitions.h>
#include <pragma/physics/physapi.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/physconstraint.h"

class DLLNETWORK LPhysShape
{
public:
	LPhysShape(PhysShape *shape)
		: m_ptr(shape) {}
	LPhysShape(std::shared_ptr<PhysShape> other)
		: m_ptr(other) {}
	const PhysShape *get() const {return m_ptr.get();}
	PhysShape *get() {return m_ptr.get();}
	bool operator==(nullptr_t) const {return m_ptr == nullptr;}
	bool operator!=(nullptr_t) const {return !(*this == nullptr);}
	PhysShape *operator->() {return m_ptr.get();}
	const PhysShape *operator->() const {return m_ptr.get();}
	PhysShape &operator*() {return *m_ptr.get();}
	const PhysShape &operator*() const {return *m_ptr.get();}

	const std::shared_ptr<PhysShape> &GetSharedPointer() const {return m_ptr;}
private:
	std::shared_ptr<PhysShape> m_ptr;
};

class DLLNETWORK LPhysConvexShape
	: public LPhysShape
{
public:
	using LPhysShape::LPhysShape;
	LPhysConvexShape(std::shared_ptr<PhysConvexShape> other) : LPhysShape(std::static_pointer_cast<PhysShape>(other)) {}
};

class DLLNETWORK LPhysTriangleShape
	: public LPhysShape
{
public:
	using LPhysShape::LPhysShape;
	LPhysTriangleShape(std::shared_ptr<PhysTriangleShape> other) : LPhysShape(std::static_pointer_cast<PhysShape>(other)) {}
};

class DLLNETWORK LPhysConvexHullShape
	: public LPhysConvexShape
{
public:
	using LPhysConvexShape::LPhysConvexShape;
	LPhysConvexHullShape(std::shared_ptr<PhysConvexHullShape> other) : LPhysConvexShape(std::static_pointer_cast<PhysConvexShape>(other)) {}
};

class DLLNETWORK LPhysHeightfield
	: public LPhysShape
{
public:
	using LPhysShape::LPhysShape;
	LPhysHeightfield(std::shared_ptr<PhysHeightfield> other) : LPhysShape(std::static_pointer_cast<PhysShape>(other)) {}
};

lua_registercheck(PhysShape,LPhysShape);
lua_registercheck(PhysConvexHullShape,LPhysConvexHullShape);
lua_registercheck(PhysHeightfield,LPhysHeightfield);
lua_registercheck(PhysRigidBody,PhysRigidBodyHandle);

#define LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys) \
	if(hPhys == nullptr) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL PhysCollisionObj"); \
		lua_error(l); \
		return; \
	}

LUA_SETUP_HANDLE_CHECK(PhysCollisionObject,PhysCollisionObject,PhysCollisionObjectHandle);

namespace Lua
{
	namespace physenv
	{
		DLLNETWORK void register_library(Lua::Interface &lua);
	};
	namespace PhysShape
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
	namespace PhysCollisionObj
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
	namespace PhysKinematicCharacterController
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
	namespace PhysContact
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
};

namespace Lua
{
	DLLNETWORK void PushConstraint(lua_State *l,const ConstraintHandle &hConstraint);
	DLLNETWORK void PushConstraint(lua_State *l,::PhysConstraint *constraint);
};

#define LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint) \
	if(hConstraint == nullptr) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL PhysConstraint"); \
		lua_error(l); \
		return; \
	}

LUA_SETUP_HANDLE_CHECK(PhysConstraint,::PhysConstraint,ConstraintHandle);

namespace Lua
{
	namespace PhysConstraint
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
};

#endif
