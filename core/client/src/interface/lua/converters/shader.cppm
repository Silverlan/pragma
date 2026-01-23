// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.converters.shader;

export import pragma.lua;
export import pragma.prosper;
import :rendering.shaders.compose_rma;
import :rendering.shaders.scene;
import :rendering.shaders.textured;

export namespace luabind {
	template<typename T, typename TConverter>
	struct shader_converter {
		enum { consumed_args = 1 };

		template<class U>
		T to_cpp(lua::State *L, U u, int index);
		void to_lua(lua::State *L, T x);

		template<class U>
		int match(lua::State *L, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}
	  private:
		TConverter m_converter;
	};

	template<typename T>
	concept is_shader = std::derived_from<pragma::util::base_type<T>, prosper::Shader>;

	template<typename T>
	    requires(is_shader<T> && std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : shader_converter<T, detail::pointer_converter> {};
	template<typename T>
	    requires(is_shader<T> && std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : shader_converter<T, detail::const_pointer_converter> {};

	template<typename T>
	    requires(is_shader<T> && std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : shader_converter<T, detail::ref_converter> {};
	template<typename T>
	    requires(is_shader<T> && std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : shader_converter<T, detail::const_ref_converter> {};

	// Implementation
	template<typename T, typename TConverter>
	template<class U>
	T shader_converter<T, TConverter>::to_cpp(lua::State *L, U u, int index)
	{
		return m_converter.to_cpp(L, u, index);
	}

	template<typename T, typename TConverter>
	void shader_converter<T, TConverter>::to_lua(lua::State *L, T x)
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
				if(dynamic_cast<pragma::ShaderScene *>(&shader) != nullptr) {
					if(dynamic_cast<pragma::ShaderSceneLit *>(&shader) != nullptr) {
						if(dynamic_cast<pragma::ShaderEntity *>(&shader) != nullptr) {
							if(dynamic_cast<pragma::ShaderGameWorldLightingPass *>(&shader) != nullptr)
								Lua::PushRaw<pragma::ShaderGameWorldLightingPass *>(L, static_cast<pragma::ShaderGameWorldLightingPass *>(&shader));
							else
								Lua::PushRaw<pragma::ShaderEntity *>(L, static_cast<pragma::ShaderEntity *>(&shader));
						}
						else
							Lua::PushRaw<pragma::ShaderSceneLit *>(L, static_cast<pragma::ShaderSceneLit *>(&shader));
					}
					else
						Lua::PushRaw<pragma::ShaderScene *>(L, static_cast<pragma::ShaderScene *>(&shader));
				}
				else if(dynamic_cast<pragma::ShaderComposeRMA *>(&shader))
					Lua::PushRaw<pragma::ShaderComposeRMA *>(L, static_cast<pragma::ShaderComposeRMA *>(&shader));
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
	int shader_converter<T, TConverter>::match(lua::State *L, U u, int index)
	{
		return m_converter.match(L, u, index);
	}
};
