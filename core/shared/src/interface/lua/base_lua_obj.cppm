// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"
#include <memory>

export module pragma.shared:scripting.lua.base_lua_obj;

export {
	class DLLNETWORK BaseLuaObj {
	protected:
		BaseLuaObj();
		virtual ~BaseLuaObj();
		bool m_bExternalHandle; // If true, m_handle will not be deleted
		std::unique_ptr<luabind::object> m_luaObj = nullptr;
	public:
		virtual luabind::object *GetLuaObject();
	};

	template<class THandle>
	class LuaObj : public BaseLuaObj {
	protected:
		mutable THandle *m_handle;
		virtual void InitializeHandle() = 0;
		template<class TCustomHandle>
		void InitializeLuaObject(lua_State *lua);
		virtual void InitializeLuaObject(lua_State *lua);
	public:
		LuaObj();
		virtual ~LuaObj() override;
		THandle GetHandle() const;
	};

	template<class THandle>
	LuaObj<THandle>::LuaObj() : BaseLuaObj()
	{
	}

	template<class THandle>
	LuaObj<THandle>::~LuaObj()
	{
		m_handle->reset();
		if(m_bExternalHandle == false)
			delete m_handle;
	}

	template<class THandle>
	THandle LuaObj<THandle>::GetHandle() const
	{
		return *m_handle;
	}

	template<class THandle>
	void LuaObj<THandle>::InitializeLuaObject(lua_State *lua)
	{
		InitializeLuaObject<THandle>(lua);
	}

	template<class THandle>
	template<class TCustomHandle>
	void LuaObj<THandle>::InitializeLuaObject(lua_State *lua)
	{
		if(m_luaObj != nullptr)
			return;
		m_luaObj = std::make_unique<luabind::object>(lua, *(dynamic_cast<TCustomHandle *>(m_handle))); // dynamic_cast required for virtual inheritance
	}
};
