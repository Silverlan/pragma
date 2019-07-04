#ifndef __PHYS_BASE_HPP__
#define __PHYS_BASE_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <memory>
#include <sharedutils/util_shared_handle.hpp>
#include <pragma/physics/physapi.h>
#include <vector>

namespace pragma::physics
{
	class IEnvironment;
	class DLLNETWORK IBase
		: public std::enable_shared_from_this<IBase>
	{
	public:
		IBase(const IBase&)=delete;
		IBase &operator=(const IBase&)=delete;
		virtual ~IBase()=default;

		util::TWeakSharedHandle<IBase> GetHandle() const;
		util::TSharedHandle<IBase> ClaimOwnership() const;

		virtual bool IsConstraint() const;
		virtual bool IsCollisionObject() const;
		virtual bool IsController() const;

		virtual void OnRemove();
		virtual void InitializeLuaObject(lua_State *lua);
		luabind::object &GetLuaObject();
		const luabind::object &GetLuaObject() const;
		void Push(lua_State *l);
		void *userData = nullptr;
	protected:
		friend IEnvironment;
		IBase(IEnvironment &env);
		void SetHandle(const util::TWeakSharedHandle<IBase> &handle);

		IEnvironment &m_physEnv;
		util::TWeakSharedHandle<IBase> m_handle = {};

		std::unique_ptr<luabind::object> m_luaObj = nullptr;
	};
};

#endif
