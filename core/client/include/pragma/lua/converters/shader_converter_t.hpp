// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_SHADER_CONVERTERS_T_HPP__
#define __LUA_SHADER_CONVERTERS_T_HPP__

#include "pragma/lua/converters/shader_converter.hpp"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"

namespace pragma {
	class ShaderEntity;
	class ShaderSceneLit;
	class ShaderScene;
	class ShaderComposeRMA;
	class LuaShaderWrapperBase;
};

namespace prosper {
	class ShaderGraphics;
	class ShaderCompute;
};

template<typename T, typename TConverter>
template<class U>
T luabind::shader_converter<T, TConverter>::to_cpp(lua_State *L, U u, int index)
{
	return m_converter.to_cpp(L, u, index);
}

template<typename T, typename TConverter>
void luabind::shader_converter<T, TConverter>::to_lua(lua_State *L, T x)
{
	prosper::Shader *pshader = nullptr;
	if constexpr(std::is_pointer_v<T>)
		pshader = static_cast<prosper::Shader *>(x);
	else
		pshader = &static_cast<prosper::Shader &>(x);
	auto &shader = *pshader;

	// auto *luaShader = dynamic_cast<::pragma::LShaderBase*>(&shader);
	// if(luaShader != nullptr)
	// 	luaShader->GetShaderObject().push(L);
	// else
	{
		if(shader.IsGraphicsShader()) {
			if(dynamic_cast<::pragma::ShaderScene *>(&shader) != nullptr) {
				if(dynamic_cast<::pragma::ShaderSceneLit *>(&shader) != nullptr) {
					if(dynamic_cast<::pragma::ShaderEntity *>(&shader) != nullptr) {
						if(dynamic_cast<::pragma::ShaderGameWorldLightingPass *>(&shader) != nullptr)
							Lua::PushRaw<::pragma::ShaderGameWorldLightingPass *>(L, static_cast<::pragma::ShaderGameWorldLightingPass *>(&shader));
						else
							Lua::PushRaw<::pragma::ShaderEntity *>(L, static_cast<::pragma::ShaderEntity *>(&shader));
					}
					else
						Lua::PushRaw<::pragma::ShaderSceneLit *>(L, static_cast<::pragma::ShaderSceneLit *>(&shader));
				}
				else
					Lua::PushRaw<::pragma::ShaderScene *>(L, static_cast<::pragma::ShaderScene *>(&shader));
			}
			else if(dynamic_cast<::pragma::ShaderComposeRMA *>(&shader))
				Lua::PushRaw<::pragma::ShaderComposeRMA *>(L, static_cast<::pragma::ShaderComposeRMA *>(&shader));
			else
				Lua::PushRaw<prosper::ShaderGraphics *>(L, static_cast<prosper::ShaderGraphics *>(&shader));
		}
		else if(shader.IsComputeShader())
			Lua::PushRaw<prosper::ShaderCompute *>(L, static_cast<prosper::ShaderCompute *>(&shader));
		else
			Lua::PushRaw<prosper::Shader *>(L, &shader);
	}
}

template<typename T, typename TConverter>
template<class U>
int luabind::shader_converter<T, TConverter>::match(lua_State *L, U u, int index)
{
	return m_converter.match(L, u, index);
}

#endif
