#ifndef __C_SHADERLUA_H__
#define __C_SHADERLUA_H__
// prosper TODO
#if 0
#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_textured.h"
#include <pragma/lua/ldefinitions.h>
#include <sharedutils/functioncallback.h>
/*
namespace Shader
{
	class DLLCLIENT BaseLua
		: public Textured3D
	{
	protected:
		std::unique_ptr<luabind::object> m_luaObj;
		std::string m_fragmentShader;
		std::string m_vertexShader;
		CallbackHandle m_cbGameEnd;
		virtual void InitializeShader() override;
	public:
		BaseLua(std::string identifier,luabind::object &o);
		virtual ~BaseLua() override;
		virtual void Initialize() override;
		void Reset(luabind::object &o);
		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
	};
};

struct DLLCLIENT LuaShaderWrapper
	: luabind::wrap_base
{
public:
	void Initialize();
	static void default_Initialize(lua_State *l,Shader::BaseLua *shader);
};
*/
#endif
#endif