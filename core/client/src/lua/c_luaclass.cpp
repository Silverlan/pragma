#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/c_listener.h"
#include "pragma/lua/classes/c_lentity.h"
#include "pragma/lua/classes/c_llistener.h"
#include "pragma/lua/classes/c_lshaderinfo.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/lua/classes/c_lmaterial.h"
#include "pragma/lua/classes/c_lpoint_rendertarget.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/lua/classes/lmaterial.h"
#include "pragma/lua/classes/lentity.h"
#include "pragma/entities/point/c_point_rendertarget.h"
#include "pragma/lua/classes/c_lshaderinfo.h"
#include "pragma/lua/classes/lshaderinfo.h"
#include "pragma/rendering/shaders/c_shaderlua.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/c_wheel.hpp"
#include "pragma/model/c_side.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/lua/classes/c_lmodel.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "pragma/lua/classes/c_lmodelmesh.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/lua/classes/c_ltexture.h"
#include "luasystem.h"
#include "textureinfo.h"
#include "pragma/model/brush/c_brushmesh.h"
#include <pragma/game/damageinfo.h>
#include "pragma/lua/classes/c_lua_entity.h"
#include "pragma/lua/c_lua_weapon.h"
#include "pragma/lua/c_lua_vehicle.h"
#include "pragma/lua/c_lua_npc.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/classes/c_lworldenvironment.hpp"
#include <wgui/fontmanager.h>
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <pragma/entities/func/basefuncwater.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_render_pass.hpp>
#include <luainterface.hpp>
#include <misc/compute_pipeline_create_info.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

//#define TEST_LUABIND_SMART_POINTE_DERIVED
#ifdef TEST_LUABIND_SMART_POINTE_DERIVED
namespace pragma
{
	struct LuaTestClass
	{

	};

	struct LuaTestDerived
		: public LuaTestClass
	{

	};
};

namespace util
{
	template<class T>
		struct TestHandle
	{
	public:
		TestHandle()=default;
		TestHandle(T *o,float x,float y,float z) : object(o) {};
		T *object = nullptr;
	};
};

static void test_func(lua_State *l,pragma::LuaTestClass &o)
{
	std::cout<<"FUNC CALLED!"<<std::endl;
}

static void test_func_derived(lua_State *l,pragma::LuaTestDerived &o)
{
	std::cout<<"DERIVED CALLED!"<<std::endl;
}

static void test_func_derived_handle(lua_State *l,util::TestHandle<pragma::LuaTestDerived> &o)
{
	std::cout<<"DERIVED HANDLE CALLED!"<<std::endl;
}

//static std::shared_ptr<LuaTestClass> testObj = nullptr;
static pragma::LuaTestDerived *testObj = nullptr;
static int32_t test_create(lua_State *l)
{
	std::cout<<"CREATE CALLED!"<<std::endl;
	testObj = new pragma::LuaTestDerived();//std::make_shared<LuaTestClass>();
	Lua::Push<util::TestHandle<pragma::LuaTestDerived>>(l,util::TestHandle<pragma::LuaTestDerived>(testObj,1.f,1.f,1.f));
	return 1;
}

static pragma::LuaTestClass *testObjBase = nullptr;
static int32_t test_create_base(lua_State *l)
{
	std::cout<<"BASE CREATE CALLED!"<<std::endl;
	testObjBase = new pragma::LuaTestClass();//std::make_shared<LuaTestClass>();
	Lua::Push<util::TestHandle<pragma::LuaTestClass>>(l,util::TestHandle<pragma::LuaTestClass>(testObjBase,1.f,1.f,1.f));
	return 1;
}

namespace pragma
{
	/*pragma::LuaTestClass *get_pointer(const util::TestHandle<pragma::LuaTestClass> &hObj)
	{
		return hObj.object;
	}

	pragma::LuaTestDerived *get_pointer(const util::TestHandle<pragma::LuaTestDerived> &hObj)
	{
		return hObj.object;
	}*/
	template<class T>
		T *get_pointer(const util::TestHandle<T> &hObj)
	{
		return hObj.object;
	}
};
#endif
void ClientState::RegisterSharedLuaClasses(Lua::Interface &lua,bool bGUI)
{
	auto &modEngine = lua.RegisterLibrary("engine");
	auto defFontInfo = luabind::class_<FontInfo>("FontInfo");
	modEngine[defFontInfo];

#ifdef TEST_LUABIND_SMART_POINTE_DERIVED
	// TEST
	auto defTest = luabind::class_<pragma::LuaTestClass,util::TestHandle<pragma::LuaTestClass>>("LuaTestClass");
	defTest.def("TestFunc",&test_func);
	modEngine[defTest];

	auto defTestDerived = luabind::class_<pragma::LuaTestDerived,pragma::LuaTestClass,util::TestHandle<pragma::LuaTestDerived>>("LuaTestDerived");
	defTestDerived.def("TestDerived",&test_func_derived);
	defTestDerived.def("TestDerivedHandle",&test_func_derived_handle);
	modEngine[defTestDerived];
	auto &vmod = lua.RegisterLibrary("test",std::unordered_map<std::string,int(*)(lua_State*)>{
		{"create",test_create},
			{"create_base",test_create_base}
	});
	//
#endif
	
	auto &modUtil = lua.RegisterLibrary("util");
	auto defTexture = luabind::class_<Texture>("Texture");
	defTexture.def("GetWidth",&Lua::Texture::GetWidth);
	defTexture.def("GetHeight",&Lua::Texture::GetHeight);
	defTexture.def("GetVkTexture",&Lua::Texture::GetVkTexture);
	modUtil[defTexture];

	auto defTexInfo = luabind::class_<TextureInfo>("TextureInfo");
	defTexInfo.def("GetTexture",&Lua::TextureInfo::GetTexture);
	defTexInfo.def("GetSize",&Lua::TextureInfo::GetSize);
	defTexInfo.def("GetWidth",&Lua::TextureInfo::GetWidth);
	defTexInfo.def("GetHeight",&Lua::TextureInfo::GetHeight);
	defTexInfo.def("GetName",static_cast<void(*)(lua_State*,TextureInfo&)>([](lua_State *l,TextureInfo &textureInfo) {
		Lua::PushString(l,textureInfo.name);
	}));
	modUtil[defTexInfo];

	auto &modGame = lua.RegisterLibrary("game");
	auto materialClassDef = luabind::class_<Material>("Material");
	Lua::Material::register_class(materialClassDef);
	materialClassDef.def("SetTexture",static_cast<void(*)(lua_State*,Material*,const std::string&,const std::string&)>(&Lua::Material::Client::SetTexture));
	materialClassDef.def("SetTexture",static_cast<void(*)(lua_State*,Material*,const std::string&,Texture&)>(&Lua::Material::Client::SetTexture));
	materialClassDef.def("SetTexture",static_cast<void(*)(lua_State*,Material*,const std::string&,Lua::Vulkan::Texture&)>(&Lua::Material::Client::SetTexture));
	materialClassDef.def("GetTextureInfo",&Lua::Material::Client::GetTexture);
	materialClassDef.def("GetData",&Lua::Material::Client::GetData);
	materialClassDef.def("InitializeShaderDescriptorSet",static_cast<void(*)(lua_State*,::Material*,bool)>(&Lua::Material::Client::InitializeShaderData));
	materialClassDef.def("InitializeShaderDescriptorSet",static_cast<void(*)(lua_State*,::Material*)>(&Lua::Material::Client::InitializeShaderData));
	modGame[materialClassDef];

	 // prosper TODO
	auto &modShader = lua.RegisterLibrary("shader",{
		{"register",[](lua_State *l) {
			auto *className = Lua::CheckString(l,1);
			luaL_checkuserdata(l,2);
			auto o = luabind::object(luabind::from_stack(l,2));
			if(o)
			{
				auto &manager = c_game->GetLuaShaderManager();
				manager.RegisterShader(className,o);
			}
			return 0;
		}},
		{"get",[](lua_State *l) {
			auto *className = Lua::CheckString(l,1);
			auto whShader = c_engine->GetShader(className);
			if(whShader.expired())
				return 0;
			Lua::shader::push_shader(l,*whShader.get());
			return 1;
		}}
	});

	auto defShaderInfo = luabind::class_<util::ShaderInfo>("Info");
	//defShaderInfo.def("GetID",&Lua_ShaderInfo_GetID);
	defShaderInfo.def("GetName",&Lua_ShaderInfo_GetName);
	modShader[defShaderInfo];

	auto defShader = luabind::class_<prosper::Shader>("Shader");
	defShader.def("RecordBindDescriptorSet",&Lua::Shader::RecordBindDescriptorSet);
	defShader.def("RecordBindDescriptorSet",static_cast<void(*)(lua_State*,prosper::Shader&,Lua::Vulkan::DescriptorSet&,uint32_t)>([](lua_State *l,prosper::Shader &shader,Lua::Vulkan::DescriptorSet &ds,uint32_t firstSet) {
		Lua::Shader::RecordBindDescriptorSet(l,shader,ds,firstSet,{});
	}));
	defShader.def("RecordBindDescriptorSet",static_cast<void(*)(lua_State*,prosper::Shader&,Lua::Vulkan::DescriptorSet&)>([](lua_State *l,prosper::Shader &shader,Lua::Vulkan::DescriptorSet &ds) {
		Lua::Shader::RecordBindDescriptorSet(l,shader,ds,0u,{});
	}));
	defShader.def("RecordBindDescriptorSets",&Lua::Shader::RecordBindDescriptorSets);
	defShader.def("RecordBindDescriptorSets",static_cast<void(*)(lua_State*,prosper::Shader&,luabind::object,uint32_t)>([](lua_State *l,prosper::Shader &shader,luabind::object descSets,uint32_t firstSet) {
		Lua::Shader::RecordBindDescriptorSets(l,shader,descSets,firstSet,{});
	}));
	defShader.def("RecordBindDescriptorSets",static_cast<void(*)(lua_State*,prosper::Shader&,luabind::object)>([](lua_State *l,prosper::Shader &shader,luabind::object descSets) {
		Lua::Shader::RecordBindDescriptorSets(l,shader,descSets,0u,{});
	}));
	defShader.def("RecordPushConstants",&Lua::Shader::RecordPushConstants);
	defShader.def("RecordPushConstants",static_cast<void(*)(lua_State*,prosper::Shader&,::DataStream&)>([](lua_State *l,prosper::Shader &shader,::DataStream &ds) {
		Lua::Shader::RecordPushConstants(l,shader,ds,0u);
	}));
	defShader.def("GetEntrypointName",&Lua::Shader::GetEntrypointName);
	defShader.def("GetEntrypointName",static_cast<void(*)(lua_State*,prosper::Shader&,uint32_t)>([](lua_State *l,prosper::Shader &shader,uint32_t shaderStage) {
		Lua::Shader::GetEntrypointName(l,shader,shaderStage,0u);
	}));
	defShader.def("CreateDescriptorSet",&Lua::Shader::CreateDescriptorSetGroup);
	defShader.def("CreateDescriptorSet",static_cast<void(*)(lua_State*,prosper::Shader&,uint32_t)>([](lua_State *l,prosper::Shader &shader,uint32_t setIdx) {
		Lua::Shader::CreateDescriptorSetGroup(l,shader,setIdx,0u);
	}));
	defShader.def("GetPipelineInfo",&Lua::Shader::GetPipelineInfo);
	defShader.def("GetPipelineInfo",static_cast<void(*)(lua_State*,prosper::Shader&,uint32_t,uint32_t)>([](lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx) {
		Lua::Shader::GetPipelineInfo(l,shader,shaderStage,0u);
	}));
	defShader.def("GetGlslSourceCode",&Lua::Shader::GetGlslSourceCode);
	defShader.def("GetGlslSourceCode",static_cast<void(*)(lua_State*,prosper::Shader&,uint32_t,uint32_t)>([](lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx) {
		Lua::Shader::GetGlslSourceCode(l,shader,shaderStage,0u);
	}));
	defShader.def("GetSpirvBlob",&Lua::Shader::GetSpirvBlob);
	defShader.def("GetSpirvBlob",static_cast<void(*)(lua_State*,prosper::Shader&,uint32_t,uint32_t)>([](lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx) {
		Lua::Shader::GetSpirvBlob(l,shader,shaderStage,0u);
	}));
	defShader.def("GetStatistics",&Lua::Shader::GetStatistics);
	defShader.def("GetStatistics",static_cast<void(*)(lua_State*,prosper::Shader&,uint32_t,uint32_t)>([](lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx) {
		Lua::Shader::GetStatistics(l,shader,shaderStage,0u);
	}));

	defShader.def("IsGraphicsShader",&Lua::Shader::IsGraphicsShader);
	defShader.def("IsComputeShader",&Lua::Shader::IsComputeShader);
	defShader.def("GetPipelineBindPoint",&Lua::Shader::GetPipelineBindPoint);
	defShader.def("IsValid",&Lua::Shader::IsValid);
	defShader.def("GetIdentifier",&Lua::Shader::GetIdentifier);
	defShader.def("GetSourceFilePath",&Lua::Shader::GetSourceFilePath);
	defShader.def("GetSourceFilePaths",&Lua::Shader::GetSourceFilePaths);
	modShader[defShader];

	auto defShaderGraphics = luabind::class_<prosper::ShaderGraphics,prosper::Shader>("Graphics");
	defShaderGraphics.def("RecordBindVertexBuffer",&Lua::Shader::Graphics::RecordBindVertexBuffer);
	defShaderGraphics.def("RecordBindVertexBuffer",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &buffer,uint32_t startBinding) {
		Lua::Shader::Graphics::RecordBindVertexBuffer(l,shader,buffer,startBinding,0u);
	}));
	defShaderGraphics.def("RecordBindVertexBuffer",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,Lua::Vulkan::Buffer&)>([](lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &buffer) {
		Lua::Shader::Graphics::RecordBindVertexBuffer(l,shader,buffer,0u,0u);
	}));
	defShaderGraphics.def("RecordBindVertexBuffers",&Lua::Shader::Graphics::RecordBindVertexBuffers);
	defShaderGraphics.def("RecordBindVertexBuffers",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,luabind::object,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,luabind::object buffers,uint32_t startBinding) {
		Lua::Shader::Graphics::RecordBindVertexBuffers(l,shader,buffers,startBinding,{});
	}));
	defShaderGraphics.def("RecordBindVertexBuffers",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,luabind::object)>([](lua_State *l,prosper::ShaderGraphics &shader,luabind::object buffers) {
		Lua::Shader::Graphics::RecordBindVertexBuffers(l,shader,buffers,0u,{});
	}));
	defShaderGraphics.def("RecordBindIndexBuffer",&Lua::Shader::Graphics::RecordBindIndexBuffer);
	defShaderGraphics.def("RecordBindIndexBuffer",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &indexBuffer,uint32_t indexType) {
		Lua::Shader::Graphics::RecordBindIndexBuffer(l,shader,indexBuffer,indexType,0u);
	}));
	defShaderGraphics.def("RecordDraw",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t,uint32_t,uint32_t)>(&Lua::Shader::Graphics::RecordDraw));
	defShaderGraphics.def("RecordDraw",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t vertCount,uint32_t instanceCount,uint32_t firstVertex) {
		Lua::Shader::Graphics::RecordDraw(l,shader,vertCount,instanceCount,firstVertex,0u);
	}));
	defShaderGraphics.def("RecordDraw",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t vertCount,uint32_t instanceCount) {
		Lua::Shader::Graphics::RecordDraw(l,shader,vertCount,instanceCount,0u,0u);
	}));
	defShaderGraphics.def("RecordDraw",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t vertCount) {
		Lua::Shader::Graphics::RecordDraw(l,shader,vertCount,1u,0u,0u);
	}));
	defShaderGraphics.def("RecordDrawIndexed",&Lua::Shader::Graphics::RecordDrawIndexed);
	defShaderGraphics.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t,uint32_t,int32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t vertexOffset) {
		Lua::Shader::Graphics::RecordDrawIndexed(l,shader,indexCount,instanceCount,firstIndex,vertexOffset,0u);
	}));
	defShaderGraphics.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex) {
		Lua::Shader::Graphics::RecordDrawIndexed(l,shader,indexCount,instanceCount,firstIndex,0,0u);
	}));
	defShaderGraphics.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount) {
		Lua::Shader::Graphics::RecordDrawIndexed(l,shader,indexCount,instanceCount,0u,0,0u);
	}));
	defShaderGraphics.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount) {
		Lua::Shader::Graphics::RecordDrawIndexed(l,shader,indexCount,1u,0u,0,0u);
	}));
	defShaderGraphics.def("RecordBeginDraw",&Lua::Shader::Graphics::RecordBeginDraw);
	defShaderGraphics.def("RecordBeginDraw",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,Lua::Vulkan::CommandBuffer&)>([](lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::CommandBuffer &hCommandBuffer) {
		Lua::Shader::Graphics::RecordBeginDraw(l,shader,hCommandBuffer,0u);
	}));
	defShaderGraphics.def("RecordDraw",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&)>(&Lua::Shader::Graphics::RecordDraw));
	defShaderGraphics.def("RecordEndDraw",&Lua::Shader::Graphics::RecordEndDraw);
	defShaderGraphics.def("GetRenderPass",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t)>(&Lua::Shader::Graphics::GetRenderPass));
	defShaderGraphics.def("GetRenderPass",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&)>([](lua_State *l,prosper::ShaderGraphics &shader) {
		Lua::Shader::Graphics::GetRenderPass(l,shader,0u);
	}));
	defShaderGraphics.scope[luabind::def("GetRenderPass",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto &rp = prosper::ShaderGraphics::GetRenderPass<prosper::ShaderGraphics>(*c_engine);
		if(rp == nullptr)
			return;
		Lua::Push(l,rp);
	}))];
	modShader[defShaderGraphics];

	auto defShaderScene = luabind::class_<pragma::ShaderScene,luabind::bases<prosper::ShaderGraphics,prosper::Shader>>("Scene3D");
	defShaderScene.scope[luabind::def("GetRenderPass",&Lua::Shader::Scene3D::GetRenderPass)];
	defShaderScene.scope[luabind::def("GetRenderPass",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		Lua::Shader::Scene3D::GetRenderPass(l,0u);
	}))];
	defShaderScene.def("RecordBindSceneCamera",&Lua::Shader::Scene3D::BindSceneCamera);
	defShaderScene.def("RecordBindRenderSettings",&Lua::Shader::Scene3D::BindRenderSettings);
	defShaderScene.add_static_constant("RENDER_PASS_COLOR_FORMAT",umath::to_integral(pragma::ShaderScene::RENDER_PASS_FORMAT));
	defShaderScene.add_static_constant("RENDER_PASS_BLOOM_FORMAT",umath::to_integral(pragma::ShaderScene::RENDER_PASS_FORMAT));
	defShaderScene.add_static_constant("RENDER_PASS_DEPTH_FORMAT",umath::to_integral(pragma::ShaderScene::RENDER_PASS_DEPTH_FORMAT));
	modShader[defShaderScene];

	auto defShaderSceneLit = luabind::class_<pragma::ShaderSceneLit,luabind::bases<pragma::ShaderScene,prosper::ShaderGraphics,prosper::Shader>>("SceneLit3D");
	defShaderSceneLit.def("RecordBindLights",&Lua::Shader::SceneLit3D::BindLights);
	defShaderSceneLit.def("RecordBindScene",&Lua::Shader::SceneLit3D::BindScene);
	modShader[defShaderSceneLit];

	auto defShaderEntity = luabind::class_<pragma::ShaderEntity,luabind::bases<pragma::ShaderSceneLit,pragma::ShaderScene,prosper::ShaderGraphics,prosper::Shader>>("ShaderEntity");
	defShaderEntity.def("RecordBindInstanceDescriptorSet",&Lua::Shader::ShaderEntity::BindInstanceDescriptorSet);
	defShaderEntity.def("RecordBindEntity",&Lua::Shader::ShaderEntity::BindEntity);
	defShaderEntity.def("RecordBindVertexAnimationOffset",&Lua::Shader::ShaderEntity::BindVertexAnimationOffset);
	defShaderEntity.def("RecordDrawMesh",&Lua::Shader::ShaderEntity::Draw);
	defShaderEntity.def("BindVertexAnimationOffset",static_cast<void(*)(lua_State*,pragma::ShaderEntity&,uint32_t)>([](lua_State *l,pragma::ShaderEntity &shader,uint32_t offset) {
		Lua::PushBool(l,shader.BindVertexAnimationOffset(offset));
	}));
	defShaderEntity.def("GetBoundEntity",static_cast<void(*)(lua_State*,pragma::ShaderEntity&)>([](lua_State *l,pragma::ShaderEntity &shader) {
		auto *ent = shader.GetBoundEntity();
		if(ent == nullptr)
			return;
		ent->GetLuaObject()->push(l);
	}));
	modShader[defShaderEntity];

	auto defShaderTextured3D = luabind::class_<pragma::ShaderTextured3DBase,luabind::bases<pragma::ShaderEntity,pragma::ShaderSceneLit,pragma::ShaderScene,prosper::ShaderGraphics,prosper::Shader>>("TexturedLit3D");
	defShaderTextured3D.def("RecordBindMaterial",&Lua::Shader::TexturedLit3D::BindMaterial);
	defShaderTextured3D.def("RecordBindClipPlane",&Lua::Shader::TexturedLit3D::RecordBindClipPlane);
	defShaderTextured3D.add_static_constant("PUSH_CONSTANTS_SIZE",sizeof(pragma::ShaderTextured3DBase::PushConstants));
	defShaderTextured3D.add_static_constant("PUSH_CONSTANTS_USER_DATA_OFFSET",sizeof(pragma::ShaderTextured3DBase::PushConstants));
	modShader[defShaderTextured3D];

	auto defShaderCompute = luabind::class_<prosper::ShaderCompute,prosper::Shader>("Compute");
	defShaderCompute.def("RecordDispatch",&Lua::Shader::Compute::RecordDispatch);
	defShaderCompute.def("RecordDispatch",static_cast<void(*)(lua_State*,prosper::ShaderCompute&,uint32_t,uint32_t)>([](lua_State *l,prosper::ShaderCompute &shader,uint32_t x,uint32_t y) {
		Lua::Shader::Compute::RecordDispatch(l,shader,x,y,1u);
	}));
	defShaderCompute.def("RecordDispatch",static_cast<void(*)(lua_State*,prosper::ShaderCompute&,uint32_t)>([](lua_State *l,prosper::ShaderCompute &shader,uint32_t x) {
		Lua::Shader::Compute::RecordDispatch(l,shader,x,1u,1u);
	}));
	defShaderCompute.def("RecordDispatch",static_cast<void(*)(lua_State*,prosper::ShaderCompute&)>([](lua_State *l,prosper::ShaderCompute &shader) {
		Lua::Shader::Compute::RecordDispatch(l,shader,1u,1u,1u);
	}));
	defShaderCompute.def("RecordBeginCompute",&Lua::Shader::Compute::RecordBeginCompute);
	defShaderCompute.def("RecordBeginCompute",static_cast<void(*)(lua_State*,prosper::ShaderCompute&,Lua::Vulkan::CommandBuffer&)>([](lua_State *l,prosper::ShaderCompute &shader,Lua::Vulkan::CommandBuffer &hCommandBuffer) {
		Lua::Shader::Compute::RecordBeginCompute(l,shader,hCommandBuffer,0u);
	}));
	defShaderCompute.def("RecordCompute",&Lua::Shader::Compute::RecordCompute);
	defShaderCompute.def("RecordEndCompute",&Lua::Shader::Compute::RecordEndCompute);
	modShader[defShaderCompute];

	// Custom Shaders
	auto defVertexBinding = luabind::class_<pragma::LuaVertexBinding>("VertexBinding");
	defVertexBinding.def(luabind::constructor<>());
	defVertexBinding.def(luabind::constructor<uint32_t,uint32_t>());
	defVertexBinding.def(luabind::constructor<uint32_t>());
	defVertexBinding.def_readwrite("inputRate",reinterpret_cast<uint32_t pragma::LuaVertexBinding::*>(&pragma::LuaVertexBinding::inputRate));
	defVertexBinding.def_readwrite("stride",&pragma::LuaVertexBinding::stride);
	modShader[defVertexBinding];

	auto defVertexAttribute = luabind::class_<pragma::LuaVertexAttribute>("VertexAttribute");
	defVertexAttribute.def(luabind::constructor<>());
	defVertexAttribute.def(luabind::constructor<uint32_t,uint32_t,uint32_t>());
	defVertexAttribute.def(luabind::constructor<uint32_t,uint32_t>());
	defVertexAttribute.def(luabind::constructor<uint32_t>());
	defVertexAttribute.def_readwrite("format",reinterpret_cast<uint32_t pragma::LuaVertexAttribute::*>(&pragma::LuaVertexAttribute::format));
	defVertexAttribute.def_readwrite("offset",&pragma::LuaVertexAttribute::offset);
	defVertexAttribute.def_readwrite("location",&pragma::LuaVertexAttribute::location);
	modShader[defVertexAttribute];

	auto defDescriptorSetInfo = luabind::class_<pragma::LuaDescriptorSetInfo>("DescriptorSetInfo");
	defDescriptorSetInfo.def(luabind::constructor<>());
	defDescriptorSetInfo.def(luabind::constructor<luabind::object,uint32_t>());
	defDescriptorSetInfo.def(luabind::constructor<luabind::object>());
	defDescriptorSetInfo.def_readwrite("setIndex",&pragma::LuaDescriptorSetInfo::setIndex);
	modShader[defDescriptorSetInfo];

	auto defDescriptorSetBinding = luabind::class_<pragma::LuaDescriptorSetBinding>("DescriptorSetBinding");
	defDescriptorSetBinding.def(luabind::constructor<uint32_t,uint32_t,uint32_t,uint32_t>());
	defDescriptorSetBinding.def(luabind::constructor<uint32_t,uint32_t,uint32_t>());
	defDescriptorSetBinding.def(luabind::constructor<uint32_t,uint32_t>());
	defDescriptorSetBinding.def_readwrite("type",reinterpret_cast<uint32_t pragma::LuaDescriptorSetBinding::*>(&pragma::LuaDescriptorSetBinding::type));
	defDescriptorSetBinding.def_readwrite("shaderStages",reinterpret_cast<uint32_t pragma::LuaDescriptorSetBinding::*>(&pragma::LuaDescriptorSetBinding::shaderStages));
	defDescriptorSetBinding.def_readwrite("bindingIndex",&pragma::LuaDescriptorSetBinding::bindingIndex);
	defDescriptorSetBinding.def_readwrite("descriptorArraySize",&pragma::LuaDescriptorSetBinding::descriptorArraySize);
	modShader[defDescriptorSetBinding];

	auto defShaderBasePipelineCreateInfo = luabind::class_<Anvil::BasePipelineCreateInfo>("BasePipelineCreateInfo");
	defShaderBasePipelineCreateInfo.def("AttachPushConstantRange",&Lua::BasePipelineCreateInfo::AttachPushConstantRange);
	defShaderBasePipelineCreateInfo.def("AttachDescriptorSetInfo",&Lua::BasePipelineCreateInfo::AttachDescriptorSetInfo);
	modShader[defShaderBasePipelineCreateInfo];

	auto defShaderGraphicsPipelineCreateInfo = luabind::class_<Anvil::GraphicsPipelineCreateInfo,Anvil::BasePipelineCreateInfo>("GraphicsPipelineCreateInfo");
	defShaderGraphicsPipelineCreateInfo.def("SetBlendingProperties",&Lua::GraphicsPipelineCreateInfo::SetBlendingProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetCommonAlphaBlendProperties",&Lua::GraphicsPipelineCreateInfo::SetCommonAlphaBlendProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetColorBlendAttachmentProperties",&Lua::GraphicsPipelineCreateInfo::SetColorBlendAttachmentProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetMultisamplingProperties",&Lua::GraphicsPipelineCreateInfo::SetMultisamplingProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetSampleCount",&Lua::GraphicsPipelineCreateInfo::SetSampleCount);
	defShaderGraphicsPipelineCreateInfo.def("SetSampleShadingEnabled",&Lua::GraphicsPipelineCreateInfo::SetSampleShadingEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetSampleMaskEnabled",&Lua::GraphicsPipelineCreateInfo::SetSampleMaskEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDynamicScissorBoxesCount",&Lua::GraphicsPipelineCreateInfo::SetDynamicScissorBoxesCount);
	defShaderGraphicsPipelineCreateInfo.def("SetDynamicViewportsCount",&Lua::GraphicsPipelineCreateInfo::SetDynamicViewportsCount);
	defShaderGraphicsPipelineCreateInfo.def("SetPatchControlPointsCount",&Lua::GraphicsPipelineCreateInfo::SetPatchControlPointsCount);
	defShaderGraphicsPipelineCreateInfo.def("SetPrimitiveTopology",&Lua::GraphicsPipelineCreateInfo::SetPrimitiveTopology);
	defShaderGraphicsPipelineCreateInfo.def("SetRasterizationOrder",&Lua::GraphicsPipelineCreateInfo::SetRasterizationOrder);
	defShaderGraphicsPipelineCreateInfo.def("SetRasterizationProperties",&Lua::GraphicsPipelineCreateInfo::SetRasterizationProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetPolygonMode",&Lua::GraphicsPipelineCreateInfo::SetPolygonMode);
	defShaderGraphicsPipelineCreateInfo.def("SetCullMode",&Lua::GraphicsPipelineCreateInfo::SetCullMode);
	defShaderGraphicsPipelineCreateInfo.def("SetFrontFace",&Lua::GraphicsPipelineCreateInfo::SetFrontFace);
	defShaderGraphicsPipelineCreateInfo.def("SetLineWidth",&Lua::GraphicsPipelineCreateInfo::SetLineWidth);
	defShaderGraphicsPipelineCreateInfo.def("SetScissorBoxProperties",&Lua::GraphicsPipelineCreateInfo::SetScissorBoxProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetStencilTestProperties",&Lua::GraphicsPipelineCreateInfo::SetStencilTestProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetViewportProperties",&Lua::GraphicsPipelineCreateInfo::SetViewportProperties);
	defShaderGraphicsPipelineCreateInfo.def("AreDepthWritesEnabled",&Lua::GraphicsPipelineCreateInfo::AreDepthWritesEnabled);
	defShaderGraphicsPipelineCreateInfo.def("GetBlendingProperties",&Lua::GraphicsPipelineCreateInfo::GetBlendingProperties);
	defShaderGraphicsPipelineCreateInfo.def("GetColorBlendAttachmentProperties",&Lua::GraphicsPipelineCreateInfo::GetColorBlendAttachmentProperties);
	defShaderGraphicsPipelineCreateInfo.def("GetDepthBiasState",&Lua::GraphicsPipelineCreateInfo::GetDepthBiasState);
	defShaderGraphicsPipelineCreateInfo.def("GetDepthBiasConstantFactor",&Lua::GraphicsPipelineCreateInfo::GetDepthBiasConstantFactor);
	defShaderGraphicsPipelineCreateInfo.def("GetDepthBiasClamp",&Lua::GraphicsPipelineCreateInfo::GetDepthBiasClamp);
	defShaderGraphicsPipelineCreateInfo.def("GetDepthBiasSlopeFactor",&Lua::GraphicsPipelineCreateInfo::GetDepthBiasSlopeFactor);
	defShaderGraphicsPipelineCreateInfo.def("GetDepthBoundsState",&Lua::GraphicsPipelineCreateInfo::GetDepthBoundsState);
	defShaderGraphicsPipelineCreateInfo.def("GetMinDepthBounds",&Lua::GraphicsPipelineCreateInfo::GetMinDepthBounds);
	defShaderGraphicsPipelineCreateInfo.def("GetMaxDepthBounds",&Lua::GraphicsPipelineCreateInfo::GetMaxDepthBounds);
	defShaderGraphicsPipelineCreateInfo.def("GetDepthClamp",&Lua::GraphicsPipelineCreateInfo::GetDepthClamp);
	defShaderGraphicsPipelineCreateInfo.def("GetDepthTestState",&Lua::GraphicsPipelineCreateInfo::GetDepthTestState);
	defShaderGraphicsPipelineCreateInfo.def("GetDynamicStates",&Lua::GraphicsPipelineCreateInfo::GetDynamicStates);
	defShaderGraphicsPipelineCreateInfo.def("GetScissorCount",&Lua::GraphicsPipelineCreateInfo::GetScissorCount);
	defShaderGraphicsPipelineCreateInfo.def("GetViewportCount",&Lua::GraphicsPipelineCreateInfo::GetViewportCount);
	defShaderGraphicsPipelineCreateInfo.def("GetVertexAttributeCount",&Lua::GraphicsPipelineCreateInfo::GetVertexAttributeCount);
	defShaderGraphicsPipelineCreateInfo.def("GetLogicOpState",&Lua::GraphicsPipelineCreateInfo::GetLogicOpState);
	defShaderGraphicsPipelineCreateInfo.def("GetMultisamplingProperties",&Lua::GraphicsPipelineCreateInfo::GetMultisamplingProperties);
	defShaderGraphicsPipelineCreateInfo.def("GetSampleCount",&Lua::GraphicsPipelineCreateInfo::GetSampleCount);
	defShaderGraphicsPipelineCreateInfo.def("GetMinSampleShading",&Lua::GraphicsPipelineCreateInfo::GetMinSampleShading);
	defShaderGraphicsPipelineCreateInfo.def("GetSampleMask",&Lua::GraphicsPipelineCreateInfo::GetSampleMask);
	defShaderGraphicsPipelineCreateInfo.def("GetDynamicScissorBoxesCount",&Lua::GraphicsPipelineCreateInfo::GetDynamicScissorBoxesCount);
	defShaderGraphicsPipelineCreateInfo.def("GetDynamicViewportsCount",&Lua::GraphicsPipelineCreateInfo::GetDynamicViewportsCount);
	defShaderGraphicsPipelineCreateInfo.def("GetPatchControlPointsCount",&Lua::GraphicsPipelineCreateInfo::GetPatchControlPointsCount);
	defShaderGraphicsPipelineCreateInfo.def("GetScissorBoxesCount",&Lua::GraphicsPipelineCreateInfo::GetScissorBoxesCount);
	defShaderGraphicsPipelineCreateInfo.def("GetViewportsCount",&Lua::GraphicsPipelineCreateInfo::GetViewportsCount);
	defShaderGraphicsPipelineCreateInfo.def("GetPrimitiveTopology",&Lua::GraphicsPipelineCreateInfo::GetPrimitiveTopology);
	defShaderGraphicsPipelineCreateInfo.def("GetPushConstantRanges",&Lua::GraphicsPipelineCreateInfo::GetPushConstantRanges);
	defShaderGraphicsPipelineCreateInfo.def("GetRasterizationOrder",&Lua::GraphicsPipelineCreateInfo::GetRasterizationOrder);
	defShaderGraphicsPipelineCreateInfo.def("GetRasterizationProperties",&Lua::GraphicsPipelineCreateInfo::GetRasterizationProperties);
	defShaderGraphicsPipelineCreateInfo.def("GetPolygonMode",&Lua::GraphicsPipelineCreateInfo::GetPolygonMode);
	defShaderGraphicsPipelineCreateInfo.def("GetCullMode",&Lua::GraphicsPipelineCreateInfo::GetCullMode);
	defShaderGraphicsPipelineCreateInfo.def("GetFrontFace",&Lua::GraphicsPipelineCreateInfo::GetFrontFace);
	defShaderGraphicsPipelineCreateInfo.def("GetLineWidth",&Lua::GraphicsPipelineCreateInfo::GetLineWidth);
	defShaderGraphicsPipelineCreateInfo.def("GetSampleShadingState",&Lua::GraphicsPipelineCreateInfo::GetSampleShadingState);
	defShaderGraphicsPipelineCreateInfo.def("GetScissorBoxProperties",&Lua::GraphicsPipelineCreateInfo::GetScissorBoxProperties);
	defShaderGraphicsPipelineCreateInfo.def("GetStencilTestProperties",&Lua::GraphicsPipelineCreateInfo::GetStencilTestProperties);
	defShaderGraphicsPipelineCreateInfo.def("GetSubpassId",&Lua::GraphicsPipelineCreateInfo::GetSubpassId);
	defShaderGraphicsPipelineCreateInfo.def("GetVertexAttributeProperties",&Lua::GraphicsPipelineCreateInfo::GetVertexAttributeProperties);
	defShaderGraphicsPipelineCreateInfo.def("GetViewportProperties",&Lua::GraphicsPipelineCreateInfo::GetViewportProperties);
	defShaderGraphicsPipelineCreateInfo.def("IsAlphaToCoverageEnabled",&Lua::GraphicsPipelineCreateInfo::IsAlphaToCoverageEnabled);
	defShaderGraphicsPipelineCreateInfo.def("IsAlphaToOneEnabled",&Lua::GraphicsPipelineCreateInfo::IsAlphaToOneEnabled);
	defShaderGraphicsPipelineCreateInfo.def("IsDepthClampEnabled",&Lua::GraphicsPipelineCreateInfo::IsDepthClampEnabled);
	defShaderGraphicsPipelineCreateInfo.def("IsPrimitiveRestartEnabled",&Lua::GraphicsPipelineCreateInfo::IsPrimitiveRestartEnabled);
	defShaderGraphicsPipelineCreateInfo.def("IsRasterizerDiscardEnabled",&Lua::GraphicsPipelineCreateInfo::IsRasterizerDiscardEnabled);
	defShaderGraphicsPipelineCreateInfo.def("IsSampleMaskEnabled",&Lua::GraphicsPipelineCreateInfo::IsSampleMaskEnabled);
	defShaderGraphicsPipelineCreateInfo.def("AttachVertexAttribute",&Lua::GraphicsPipelineCreateInfo::AttachVertexAttribute);
	defShaderGraphicsPipelineCreateInfo.def("AddSpecializationConstant",&Lua::GraphicsPipelineCreateInfo::AddSpecializationConstant);
	defShaderGraphicsPipelineCreateInfo.def("SetAlphaToCoverageEnabled",&Lua::GraphicsPipelineCreateInfo::SetAlphaToCoverageEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetAlphaToOneEnabled",&Lua::GraphicsPipelineCreateInfo::SetAlphaToOneEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthBiasEnabled",&Lua::GraphicsPipelineCreateInfo::SetDepthBiasEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthBiasProperties",&Lua::GraphicsPipelineCreateInfo::SetDepthBiasProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthBiasConstantFactor",&Lua::GraphicsPipelineCreateInfo::SetDepthBiasConstantFactor);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthBiasClamp",&Lua::GraphicsPipelineCreateInfo::SetDepthBiasClamp);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthBiasSlopeFactor",&Lua::GraphicsPipelineCreateInfo::SetDepthBiasSlopeFactor);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthBoundsTestEnabled",&Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthBoundsTestProperties",&Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetMinDepthBounds",&Lua::GraphicsPipelineCreateInfo::SetMinDepthBounds);
	defShaderGraphicsPipelineCreateInfo.def("SetMaxDepthBounds",&Lua::GraphicsPipelineCreateInfo::SetMaxDepthBounds);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthClampEnabled",&Lua::GraphicsPipelineCreateInfo::SetDepthClampEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthTestProperties",&Lua::GraphicsPipelineCreateInfo::SetDepthTestProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthTestEnabled",&Lua::GraphicsPipelineCreateInfo::SetDepthTestEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDepthWritesEnabled",&Lua::GraphicsPipelineCreateInfo::SetDepthWritesEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDynamicStates",&Lua::GraphicsPipelineCreateInfo::SetDynamicStates);
	defShaderGraphicsPipelineCreateInfo.def("SetDynamicStateEnabled",&Lua::GraphicsPipelineCreateInfo::SetDynamicStateEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetLogicOpProperties",&Lua::GraphicsPipelineCreateInfo::SetLogicOpProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetLogicOpEnabled",&Lua::GraphicsPipelineCreateInfo::SetLogicOpEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetPrimitiveRestartEnabled",&Lua::GraphicsPipelineCreateInfo::SetPrimitiveRestartEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetRasterizerDiscardEnabled",&Lua::GraphicsPipelineCreateInfo::SetRasterizerDiscardEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetSampleMask",&Lua::GraphicsPipelineCreateInfo::SetSampleMask);
	defShaderGraphicsPipelineCreateInfo.def("SetMinSampleShading",&Lua::GraphicsPipelineCreateInfo::SetMinSampleShading);
	defShaderGraphicsPipelineCreateInfo.def("SetStencilTestEnabled",&Lua::GraphicsPipelineCreateInfo::SetStencilTestEnabled);
	modShader[defShaderGraphicsPipelineCreateInfo];

	auto defShaderComputePipelineCreateInfo = luabind::class_<Anvil::ComputePipelineCreateInfo,Anvil::BasePipelineCreateInfo>("ComputePipelineCreateInfo");
	defShaderComputePipelineCreateInfo.def("AddSpecializationConstant",&Lua::ComputePipelineCreateInfo::AddSpecializationConstant);
	modShader[defShaderComputePipelineCreateInfo];

	auto defShaderModule = luabind::class_<pragma::LuaShaderBase>("BaseModule");
	defShaderModule.def("SetShaderSource",&Lua::Shader::SetStageSourceFilePath);
	defShaderModule.def("SetPipelineCount",&Lua::Shader::SetPipelineCount);
	defShaderModule.def("GetCurrentCommandBuffer",&Lua::Shader::GetCurrentCommandBuffer);

	defShaderModule.def("InitializePipeline",&pragma::LuaShaderBase::Lua_InitializePipeline,&pragma::LuaShaderBase::Lua_default_InitializePipeline);
	defShaderModule.def("OnInitialized",&pragma::LuaShaderBase::Lua_OnInitialized,&pragma::LuaShaderBase::Lua_default_OnInitialized);
	defShaderModule.def("OnPipelinesInitialized",&pragma::LuaShaderBase::Lua_OnPipelinesInitialized,&pragma::LuaShaderBase::Lua_default_OnPipelinesInitialized);
	defShaderModule.def("OnPipelineInitialized",&pragma::LuaShaderBase::Lua_OnPipelineInitialized,&pragma::LuaShaderBase::Lua_default_OnPipelineInitialized);
	modShader[defShaderModule];

	auto defShaderGraphicsModule = luabind::class_<pragma::LuaShaderGraphicsBase,pragma::LuaShaderBase>("BaseGraphicsModule");
	modShader[defShaderGraphicsModule];

	auto defShaderComputeModule = luabind::class_<pragma::LuaShaderComputeBase,pragma::LuaShaderBase>("BaseComputeModule");
	// defShaderComputeModule.def("GetCurrentComputeCommandBuffer",&Lua::Shader::GetCurrentComputeCommandBuffer);
	modShader[defShaderComputeModule];

	auto defShaderGraphicsBase = luabind::class_<pragma::LuaShaderGraphics,luabind::bases<pragma::LuaShaderGraphicsBase,prosper::ShaderGraphics,prosper::Shader,pragma::LuaShaderBase>>("BaseGraphics");
	defShaderGraphicsBase.def(luabind::constructor<>());
	modShader[defShaderGraphicsBase];

	auto defShaderComputeBase = luabind::class_<pragma::LuaShaderCompute,luabind::bases<pragma::LuaShaderComputeBase,prosper::ShaderCompute,prosper::Shader,pragma::LuaShaderBase>>("BaseCompute");
	defShaderComputeBase.def(luabind::constructor<>());
	modShader[defShaderComputeBase];

	auto defShaderPostProcessingBase = luabind::class_<pragma::LuaShaderPostProcessing,luabind::bases<pragma::LuaShaderGraphicsBase,prosper::ShaderGraphics,prosper::Shader,pragma::LuaShaderBase>>("BasePostProcessing");
	defShaderPostProcessingBase.def(luabind::constructor<>());
	modShader[defShaderPostProcessingBase];

	auto defShaderImageProcessing = luabind::class_<pragma::LuaShaderImageProcessing,luabind::bases<pragma::LuaShaderGraphicsBase,prosper::ShaderGraphics,prosper::Shader,pragma::LuaShaderBase>>("BaseImageProcessing");
	defShaderImageProcessing.def(luabind::constructor<>());
	modShader[defShaderImageProcessing];

	auto defShaderTextured3DBase = luabind::class_<pragma::LuaShaderTextured3D,luabind::bases<pragma::LuaShaderGraphicsBase,pragma::ShaderTextured3DBase,prosper::ShaderGraphics,prosper::Shader,pragma::LuaShaderBase>>("BaseTexturedLit3D");
	defShaderTextured3DBase.def(luabind::constructor<>());
	defShaderTextured3DBase.def("BindMaterialParameters",&pragma::LuaShaderTextured3D::Lua_BindMaterialParameters,&pragma::LuaShaderTextured3D::Lua_default_BindMaterialParameters);
	defShaderTextured3DBase.def("InitializeGfxPipelineVertexAttributes",&pragma::LuaShaderTextured3D::Lua_InitializeGfxPipelineVertexAttributes,&pragma::LuaShaderTextured3D::Lua_default_InitializeGfxPipelineVertexAttributes);
	defShaderTextured3DBase.def("InitializeGfxPipelinePushConstantRanges",&pragma::LuaShaderTextured3D::Lua_InitializeGfxPipelinePushConstantRanges,&pragma::LuaShaderTextured3D::Lua_default_InitializeGfxPipelinePushConstantRanges);
	defShaderTextured3DBase.def("InitializeGfxPipelineDescriptorSets",&pragma::LuaShaderTextured3D::Lua_InitializeGfxPipelineDescriptorSets,&pragma::LuaShaderTextured3D::Lua_default_InitializeGfxPipelineDescriptorSets);

	defShaderTextured3DBase.def("OnBindMaterial",&pragma::LuaShaderTextured3D::Lua_OnBindMaterial,&pragma::LuaShaderTextured3D::Lua_default_OnBindMaterial);
	defShaderTextured3DBase.def("OnDraw",&pragma::LuaShaderTextured3D::Lua_OnDraw,&pragma::LuaShaderTextured3D::Lua_default_OnDraw);
	defShaderTextured3DBase.def("OnBindEntity",&pragma::LuaShaderTextured3D::Lua_OnBindEntity,&pragma::LuaShaderTextured3D::Lua_default_OnBindEntity);
	defShaderTextured3DBase.def("OnBindScene",&pragma::LuaShaderTextured3D::Lua_OnBindScene,&pragma::LuaShaderTextured3D::Lua_default_OnBindScene);
	defShaderTextured3DBase.def("OnBeginDraw",&pragma::LuaShaderTextured3D::Lua_OnBeginDraw,&pragma::LuaShaderTextured3D::Lua_default_OnBeginDraw);
	defShaderTextured3DBase.def("OnEndDraw",&pragma::LuaShaderTextured3D::Lua_OnEndDraw,&pragma::LuaShaderTextured3D::Lua_default_OnEndDraw);
	
	modShader[defShaderTextured3DBase];
}

void CGame::RegisterLuaClasses()
{
	Game::RegisterLuaClasses();
	ClientState::RegisterSharedLuaClasses(GetLuaInterface());

	auto debugMod = luabind::module(GetLuaState(),"debug");
	auto defDebugRendererObject = luabind::class_<DebugRenderer::BaseObject>("RendererObject");
	defDebugRendererObject.def("Remove",&Lua::DebugRenderer::Client::Object::Remove);
	defDebugRendererObject.def("IsValid",&Lua::DebugRenderer::Client::Object::IsValid);
	defDebugRendererObject.def("SetPos",&Lua::DebugRenderer::Client::Object::SetPos);
	defDebugRendererObject.def("GetPos",&Lua::DebugRenderer::Client::Object::GetPos);
	defDebugRendererObject.def("SetRotation",&Lua::DebugRenderer::Client::Object::SetRotation);
	defDebugRendererObject.def("GetRotation",&Lua::DebugRenderer::Client::Object::GetRotation);
	defDebugRendererObject.def("SetAngles",&Lua::DebugRenderer::Client::Object::SetAngles);
	defDebugRendererObject.def("GetAngles",&Lua::DebugRenderer::Client::Object::GetAngles);
	defDebugRendererObject.def("IsVisible",&Lua::DebugRenderer::Client::Object::IsVisible);
	defDebugRendererObject.def("SetVisible",&Lua::DebugRenderer::Client::Object::SetVisible);
	debugMod[defDebugRendererObject];

	auto &modGame = GetLuaInterface().RegisterLibrary("game");
	auto defDrawSceneInfo = luabind::class_<::util::DrawSceneInfo>("DrawSceneInfo");
	defDrawSceneInfo.def(luabind::constructor<>());
	defDrawSceneInfo.property("scene",static_cast<std::shared_ptr<::Scene>(*)(::util::DrawSceneInfo&)>([](::util::DrawSceneInfo &drawSceneInfo) -> std::shared_ptr<::Scene> {
		return drawSceneInfo.scene;
	}),static_cast<void(*)(::util::DrawSceneInfo&,const std::shared_ptr<::Scene>&)>([](::util::DrawSceneInfo &drawSceneInfo,const std::shared_ptr<::Scene> &scene) {
		drawSceneInfo.scene = scene;
	}));
	defDrawSceneInfo.property("commandBuffer",static_cast<std::shared_ptr<Lua::Vulkan::CommandBuffer>(*)(::util::DrawSceneInfo&)>([](::util::DrawSceneInfo &drawSceneInfo) -> std::shared_ptr<Lua::Vulkan::CommandBuffer> {
		return drawSceneInfo.commandBuffer;
	}),static_cast<void(*)(::util::DrawSceneInfo&,Lua::Vulkan::CommandBuffer&)>([](::util::DrawSceneInfo &drawSceneInfo,Lua::Vulkan::CommandBuffer &commandBuffer) {
		drawSceneInfo.commandBuffer = commandBuffer.shared_from_this();
	}));
	defDrawSceneInfo.def_readwrite("renderFlags",reinterpret_cast<uint32_t util::DrawSceneInfo::*>(&::util::DrawSceneInfo::renderFlags));
	defDrawSceneInfo.property("clearColor",static_cast<Color(*)(::util::DrawSceneInfo&)>([](::util::DrawSceneInfo &drawSceneInfo) -> Color {
		return *drawSceneInfo.clearColor;
	}),static_cast<void(*)(::util::DrawSceneInfo&,const Color&)>([](::util::DrawSceneInfo &drawSceneInfo,const Color &color) {
		drawSceneInfo.clearColor = color;
	}));
	modGame[defDrawSceneInfo];

	auto modelMeshClassDef = luabind::class_<ModelMesh>("Mesh");
	Lua::ModelMesh::register_class(modelMeshClassDef);
	modelMeshClassDef.scope[luabind::def("Create",&Lua::ModelMesh::Client::Create)];

	auto subModelMeshClassDef = luabind::class_<ModelSubMesh>("Sub");
	Lua::ModelSubMesh::register_class(subModelMeshClassDef);
	subModelMeshClassDef.def("GetTangents",&Lua::ModelSubMesh::Client::GetTangents);
	subModelMeshClassDef.def("GetBiTangents",&Lua::ModelSubMesh::Client::GetBiTangents);
	subModelMeshClassDef.def("GetVertexBuffer",&Lua::ModelSubMesh::Client::GetVertexBuffer);
	subModelMeshClassDef.def("GetVertexWeightBuffer",&Lua::ModelSubMesh::Client::GetVertexWeightBuffer);
	subModelMeshClassDef.def("GetAlphaBuffer",&Lua::ModelSubMesh::Client::GetAlphaBuffer);
	subModelMeshClassDef.def("GetIndexBuffer",&Lua::ModelSubMesh::Client::GetIndexBuffer);
	subModelMeshClassDef.def("GetVkMesh",&Lua::ModelSubMesh::Client::GetVkMesh);
	subModelMeshClassDef.scope[luabind::def("Create",&Lua::ModelSubMesh::Client::Create)];
	subModelMeshClassDef.scope[luabind::def("CreateQuad",&Lua::ModelSubMesh::Client::CreateQuad)];
	subModelMeshClassDef.scope[luabind::def("CreateBox",&Lua::ModelSubMesh::Client::CreateBox)];
	subModelMeshClassDef.scope[luabind::def("CreateSphere",static_cast<void(*)(lua_State*,const Vector3&,float,uint32_t)>(&Lua::ModelSubMesh::Client::CreateSphere))];
	subModelMeshClassDef.scope[luabind::def("CreateSphere",static_cast<void(*)(lua_State*,const Vector3&,float)>(&Lua::ModelSubMesh::Client::CreateSphere))];
	subModelMeshClassDef.scope[luabind::def("CreateCylinder",static_cast<void(*)(lua_State*,float,float,uint32_t)>(&Lua::ModelSubMesh::Client::CreateCylinder))];
	subModelMeshClassDef.scope[luabind::def("CreateCylinder",static_cast<void(*)(lua_State*,float,float)>([](lua_State *l,float startRadius,float length) {
		Lua::ModelSubMesh::Client::CreateCylinder(l,startRadius,length,12);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCone",static_cast<void(*)(lua_State*,float,float,float,uint32_t)>(&Lua::ModelSubMesh::Client::CreateCone))];
	subModelMeshClassDef.scope[luabind::def("CreateCone",static_cast<void(*)(lua_State*,float,float,float)>([](lua_State *l,float startRadius,float length,float endRadius) {
		Lua::ModelSubMesh::Client::CreateCone(l,startRadius,length,endRadius,12);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float,bool,uint32_t)>(&Lua::ModelSubMesh::Client::CreateCircle))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float,bool)>([](lua_State *l,float radius,bool doubleSided) {
		Lua::ModelSubMesh::Client::CreateCircle(l,radius,doubleSided,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float)>([](lua_State *l,float radius) {
		Lua::ModelSubMesh::Client::CreateCircle(l,radius,true,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float,bool,uint32_t)>(&Lua::ModelSubMesh::Client::CreateRing))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float,bool)>([](lua_State *l,float innerRadius,float outerRadius,bool doubleSided) {
		Lua::ModelSubMesh::Client::CreateRing(l,innerRadius,outerRadius,doubleSided,36);
		}))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float)>([](lua_State *l,float innerRadius,float outerRadius) {
		Lua::ModelSubMesh::Client::CreateRing(l,innerRadius,outerRadius,true,36);
	}))];

	auto modelClassDef = luabind::class_<Model>("Model");
	Lua::Model::register_class(GetLuaState(),modelClassDef,modelMeshClassDef,subModelMeshClassDef);
	modelClassDef.def("AddMaterial",&Lua::Model::Client::AddMaterial);
	modelClassDef.def("SetMaterial",&Lua::Model::Client::SetMaterial);
	modelClassDef.def("GetVertexAnimationBuffer",&Lua::Model::Client::GetVertexAnimationBuffer);
	modGame[modelClassDef];
	auto _G = luabind::globals(GetLuaState());
	_G["Model"] = _G["game"]["Model"];
	_G["Animation"] = _G["game"]["Model"]["Animation"];

	// COMPONENT TODO
	/*auto vhcWheelClassDef = luabind::class_<VHCWheelHandle COMMA EntityHandle>("Wheel")
	LUA_CLASS_VHCWHEEL_SHARED;
	vehicleClassDef.scope[vhcWheelClassDef];
	modGame[vehicleClassDef];*/

	// Custom Classes
	// COMPONENT TODO
	/*auto classDefBase = luabind::class_<CLuaEntityHandle COMMA CLuaEntityWrapper COMMA luabind::bases<EntityHandle>>("BaseEntity");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBase,CLuaEntityWrapper);
	classDefBase.def("ReceiveData",&CLuaEntityWrapper::ReceiveData,static_cast<void(*)(lua_State*,EntityHandle&,NetPacket&)>(&CLuaBaseEntityWrapper::default_ReceiveData));
	classDefBase.def("OnRender",&CLuaEntityWrapper::OnRender,&CLuaBaseEntityWrapper::default_OnRender);
	classDefBase.def("OnPostRender",&CLuaEntityWrapper::OnPostRender,&CLuaBaseEntityWrapper::default_OnPostRender);
	classDefBase.def("ReceiveNetEvent",&CLuaEntityWrapper::ReceiveNetEvent,&CLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefBase];*/

	/*auto classDefWeapon = luabind::class_<CLuaWeaponHandle COMMA CLuaWeaponWrapper COMMA luabind::bases<CLuaEntityHandle COMMA WeaponHandle>>("BaseWeapon");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefWeapon,CLuaWeaponWrapper);
	LUA_CUSTOM_CLASS_WEAPON_SHARED(classDefWeapon,CLuaWeaponWrapper);
	classDefWeapon.def("HandleViewModelAnimationEvent",&CLuaWeaponWrapper::HandleViewModelAnimationEvent,&CLuaWeaponWrapper::default_HandleViewModelAnimationEvent);
	classDefWeapon.def("ReceiveData",&CLuaWeaponWrapper::ReceiveData,&CLuaBaseEntityWrapper::default_ReceiveData);
	classDefWeapon.def("OnRender",&CLuaWeaponWrapper::OnRender,&CLuaBaseEntityWrapper::default_OnRender);
	classDefWeapon.def("OnPostRender",&CLuaWeaponWrapper::OnPostRender,&CLuaBaseEntityWrapper::default_OnPostRender);
	classDefWeapon.def("ReceiveNetEvent",&CLuaWeaponWrapper::ReceiveNetEvent,&CLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefWeapon];*/

	// COMPONENT TODO
	/*auto classDefBaseVehicle = luabind::class_<CLuaVehicleHandle COMMA CLuaVehicleWrapper COMMA luabind::bases<CLuaEntityHandle COMMA VehicleHandle>>("BaseVehicle");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBaseVehicle,CLuaVehicleWrapper);
	LUA_CUSTOM_CLASS_VEHICLE_SHARED(classDefBaseVehicle,CLuaVehicleWrapper);
	classDefBaseVehicle.def("ReceiveData",&CLuaVehicleWrapper::ReceiveData,&CLuaBaseEntityWrapper::default_ReceiveData);
	classDefBaseVehicle.def("OnRender",&CLuaVehicleWrapper::OnRender,&CLuaBaseEntityWrapper::default_OnRender);
	classDefBaseVehicle.def("OnPostRender",&CLuaVehicleWrapper::OnPostRender,&CLuaBaseEntityWrapper::default_OnPostRender);
	classDefBaseVehicle.def("ReceiveNetEvent",&CLuaVehicleWrapper::ReceiveNetEvent,&CLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefBaseVehicle];

	auto classDefBaseNPC = luabind::class_<CLuaNPCHandle COMMA CLuaNPCWrapper COMMA luabind::bases<CLuaEntityHandle COMMA NPCHandle>>("BaseNPC");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBaseNPC,CLuaNPCWrapper);
	LUA_CUSTOM_CLASS_NPC_SHARED(classDefBaseNPC,CLuaNPCWrapper);
	classDefBaseNPC.def("ReceiveData",&CLuaNPCWrapper::ReceiveData,&CLuaBaseEntityWrapper::default_ReceiveData);
	classDefBaseNPC.def("OnRender",&CLuaNPCWrapper::OnRender,&CLuaBaseEntityWrapper::default_OnRender);
	classDefBaseNPC.def("OnPostRender",&CLuaNPCWrapper::OnPostRender,&CLuaBaseEntityWrapper::default_OnPostRender);
	classDefBaseNPC.def("ReceiveNetEvent",&CLuaNPCWrapper::ReceiveNetEvent,&CLuaBaseEntityWrapper::default_ReceiveNetEvent);
	classDefBaseNPC.def("PlayFootStepSound",&CLuaNPCWrapper::PlayFootStepSound,&CLuaNPCWrapper::default_PlayFootStepSound);
	classDefBaseNPC.def("CalcMovementSpeed",&CLuaNPCWrapper::CalcMovementSpeed,&CLuaNPCWrapper::default_CalcMovementSpeed);
	modGame[classDefBaseNPC];*/
	//
	
	// COMPONENT TODO
	/*auto defListener = luabind::class_<ListenerHandle COMMA EntityHandle>("Listener");
	defListener.def("GetGain",&Lua_Listener_GetGain);
	defListener.def("SetGain",&Lua_Listener_SetGain);
	modGame[defListener];
	
	auto defEnvParticleSystem = luabind::class_<EnvParticleSystemHandle COMMA EntityHandle>("EnvParticleSystem");
	modGame[defEnvParticleSystem];
	
	auto envLightClassDef = luabind::class_<EnvLightHandle COMMA EntityHandle>("EnvLight");
	Lua::EnvLight::register_class(envLightClassDef);
	modGame[envLightClassDef];

	auto envLightSpotClassDef = luabind::class_<EnvLightSpotHandle COMMA EnvLightHandle COMMA EntityHandle>("EnvLightSpot");
	Lua::EnvLightSpot::register_class(envLightSpotClassDef);
	modGame[envLightSpotClassDef];

	auto envLightPointClassDef = luabind::class_<EnvLightPointHandle COMMA EnvLightHandle COMMA EntityHandle>("EnvLightPoint");
	Lua::EnvLightPoint::register_class(envLightPointClassDef);
	modGame[envLightPointClassDef];

	auto envLightDirectionalClassDef = luabind::class_<EnvLightDirectionalHandle COMMA EnvLightHandle COMMA EntityHandle>("EnvLightDirectional");
	Lua::EnvLightDirectional::register_class(envLightDirectionalClassDef);
	modGame[envLightDirectionalClassDef];

	auto funcWaterClassDef = luabind::class_<FuncWaterHandle COMMA EntityHandle>("FuncWater");
	Lua::FuncWater::register_class(funcWaterClassDef);
	modGame[funcWaterClassDef];

	auto defRenderTarget = luabind::class_<PointRenderTargetHandle COMMA EntityHandle>("RenderTarget");
	defRenderTarget.def("GetTextureBuffer",&Lua_PointRenderTarget_GetTextureBuffer);
	defRenderTarget.def("SetRenderSize",&Lua_PointRenderTarget_SetRenderSize);
	defRenderTarget.def("GetRenderSize",&Lua_PointRenderTarget_GetRenderSize);
	defRenderTarget.def("SetRenderMaterial",static_cast<void(*)(lua_State*,PointRenderTargetHandle&,Material*)) &Lua_PointRenderTarget_SetRenderMaterial);
	defRenderTarget.def("SetRenderMaterial",static_cast<void(*)(lua_State*,PointRenderTargetHandle&,std::string)) &Lua_PointRenderTarget_SetRenderMaterial);
	defRenderTarget.def("SetRenderMaterial",static_cast<void(*)(lua_State*,PointRenderTargetHandle&)) &Lua_PointRenderTarget_SetRenderMaterial);
	defRenderTarget.def("GetRenderMaterial",&Lua_PointRenderTarget_GetRenderMaterial);
	defRenderTarget.def("SetRefreshRate",&Lua_PointRenderTarget_SetRefreshRate);
	defRenderTarget.def("GetRefreshRate",&Lua_PointRenderTarget_GetRefreshRate);
	defRenderTarget.def("SetRenderDepth",&Lua_PointRenderTarget_SetRenderDepth);
	defRenderTarget.def("GetRenderDepth",&Lua_PointRenderTarget_GetRenderDepth);
	defRenderTarget.def("SetRenderFOV",&Lua_PointRenderTarget_SetRenderFOV);
	defRenderTarget.def("GetRenderFOV",&Lua_PointRenderTarget_GetRenderFOV);
	modGame[defRenderTarget];*/

	_G["Entity"] = _G["ents"]["Entity"];
	_G["BaseEntity"] = _G["ents"]["BaseEntity"];

	auto worldEnvClassDef = luabind::class_<::WorldEnvironment>("WorldEnvironment");
	Lua::WorldEnvironment::register_class(worldEnvClassDef);
	modGame[worldEnvClassDef];
}
