// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.base;

export import :physics.enums;
export import :scripting.lua.api;
export import :types;
export import pragma.util;

export {
	namespace pragma::physics {
		class IEnvironment;
		class DLLNETWORK IBase : public std::enable_shared_from_this<IBase> {
		  public:
			IBase(const IBase &) = delete;
			IBase &operator=(const IBase &) = delete;
			virtual ~IBase() = default;

			util::TWeakSharedHandle<IBase> GetHandle() const;
			util::TSharedHandle<IBase> ClaimOwnership() const;

			virtual bool IsConstraint() const;
			virtual bool IsCollisionObject() const;
			virtual bool IsController() const;

			virtual void OnRemove();
			virtual void Initialize();
			virtual void InitializeLuaObject(lua::State *lua);
			luabind::object &GetLuaObject(lua::State *lua);
			const luabind::object &GetLuaObject(lua::State *lua) const;
			void Push(lua::State *l);

			void *GetUserData() const;
			PhysObj *GetPhysObj() const;
			void SetPhysObj(PhysObj &physObj);
		  protected:
			friend IEnvironment;
			friend PhysObj;
			IBase(IEnvironment &env);
			void SetUserData(void *userData) const;
			virtual void InitializeLuaHandle(const util::TWeakSharedHandle<IBase> &handle);
			template<class T>
			void InitializeLuaObject(lua::State *lua);

			IEnvironment &m_physEnv;
			util::TWeakSharedHandle<IBase> m_handle = {};

			std::unique_ptr<luabind::object> m_luaObj = nullptr;
		  private:
			mutable void *m_userData = nullptr;
			mutable PhysObj *m_physObj = nullptr;
		};

		class DLLNETWORK IWorldObject {
		  public:
			bool IsSpawned() const;
			void AddWorldObject();
			void Spawn();
			virtual void RemoveWorldObject() = 0;
			virtual void DoAddWorldObject() = 0;
		  protected:
			virtual void DoSpawn();
		  private:
			bool m_bSpawned = false;
		};

		template<class T>
		void IBase::InitializeLuaObject(lua::State *lua)
		{
			auto handle = ClaimOwnership();
			if(handle.IsValid())
				m_luaObj = std::make_unique<luabind::object>(lua, LuaCore::raw_object_to_luabind_object(lua, util::shared_handle_cast<IBase, T>(handle)));
			else
				m_luaObj = std::make_unique<luabind::object>(lua, LuaCore::raw_object_to_luabind_object(lua, std::dynamic_pointer_cast<T>(shared_from_this())));
		}
	};
};
