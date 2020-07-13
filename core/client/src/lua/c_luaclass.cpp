/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/classes/c_lworldenvironment.hpp"
#include "pragma/asset/c_util_model.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include <pragma/lua/lua_entity_component.hpp>
#include <shader/prosper_pipeline_create_info.hpp>
#include <wgui/fontmanager.h>
#include <wgui/shaders/wishader_textured.hpp>
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <pragma/entities/func/basefuncwater.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_render_pass.hpp>
#include <pragma/lua/lua_call.hpp>
#include <luainterface.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void ClientState::RegisterSharedLuaClasses(Lua::Interface &lua,bool bGUI)
{
	auto &modEngine = lua.RegisterLibrary("engine");
	auto defFontInfo = luabind::class_<FontInfo>("FontInfo");
	modEngine[defFontInfo];

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

	auto spriteSheetDef = luabind::class_<SpriteSheetAnimation>("SpriteSheetAnimation");

	auto sequenceDef = luabind::class_<SpriteSheetAnimation::Sequence>("Sequence");
	sequenceDef.def("GetDuration",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence&)>([](lua_State *l,SpriteSheetAnimation::Sequence &seq) {
		Lua::PushNumber(l,seq.GetDuration());
	}));
	sequenceDef.def("GetFrameOffset",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence&)>([](lua_State *l,SpriteSheetAnimation::Sequence &seq) {
		Lua::PushNumber(l,seq.GetFrameOffset());
	}));

	auto frameDef = luabind::class_<SpriteSheetAnimation::Sequence::Frame>("Frame");
	frameDef.def("GetUVBounds",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence::Frame&)>([](lua_State *l,SpriteSheetAnimation::Sequence::Frame &frame) {
		Lua::Push<Vector2>(l,frame.uvStart);
		Lua::Push<Vector2>(l,frame.uvEnd);
	}));
	frameDef.def("GetDuration",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence::Frame&)>([](lua_State *l,SpriteSheetAnimation::Sequence::Frame &frame) {
		Lua::PushNumber(l,frame.duration);
	}));
	sequenceDef.scope[frameDef];

	sequenceDef.def("GetFrameCount",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence&)>([](lua_State *l,SpriteSheetAnimation::Sequence &sequence) {
		Lua::PushInt(l,sequence.frames.size());
	}));
	sequenceDef.def("GetFrame",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence&,uint32_t)>([](lua_State *l,SpriteSheetAnimation::Sequence &sequence,uint32_t frameIdx) {
		if(frameIdx >= sequence.frames.size())
			return;
		auto &frame = sequence.frames.at(frameIdx);
		Lua::Push<SpriteSheetAnimation::Sequence::Frame*>(l,&frame);
	}));
	sequenceDef.def("GetFrames",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence&)>([](lua_State *l,SpriteSheetAnimation::Sequence &sequence) {
		auto &frames = sequence.frames;
		auto t = Lua::CreateTable(l);
		uint32_t frameIndex = 1;
		for(auto &frame : frames)
		{
			Lua::PushInt(l,frameIndex++);
			Lua::Push<SpriteSheetAnimation::Sequence::Frame*>(l,&frame);
			Lua::SetTableValue(l,t);
		}
	}));
	sequenceDef.def("IsLooping",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence&)>([](lua_State *l,SpriteSheetAnimation::Sequence &sequence) {
		Lua::PushBool(l,sequence.loop);
	}));
	sequenceDef.def("GetInterpolatedFrameData",static_cast<void(*)(lua_State*,SpriteSheetAnimation::Sequence&,float)>([](lua_State *l,SpriteSheetAnimation::Sequence &sequence,float ptTime) {
		uint32_t frameIndex0,frameIndex1;
		float interpFactor;
		if(sequence.GetInterpolatedFrameData(ptTime,frameIndex0,frameIndex1,interpFactor) == false)
			return;
		Lua::PushInt(l,frameIndex0);
		Lua::PushInt(l,frameIndex1);
		Lua::PushNumber(l,interpFactor);
	}));

	spriteSheetDef.scope[sequenceDef];
	
	spriteSheetDef.def("GetSequenceCount",static_cast<void(*)(lua_State*,SpriteSheetAnimation&)>([](lua_State *l,SpriteSheetAnimation &spriteSheetAnim) {
		Lua::PushInt(l,spriteSheetAnim.sequences.size());
	}));
	spriteSheetDef.def("GetSequence",static_cast<void(*)(lua_State*,SpriteSheetAnimation&,uint32_t)>([](lua_State *l,SpriteSheetAnimation &spriteSheetAnim,uint32_t seqIdx) {
		if(seqIdx >= spriteSheetAnim.sequences.size())
			return;
		auto &seq = spriteSheetAnim.sequences.at(seqIdx);
		Lua::Push<SpriteSheetAnimation::Sequence*>(l,&seq);
	}));
	spriteSheetDef.def("GetSequences",static_cast<void(*)(lua_State*,SpriteSheetAnimation&)>([](lua_State *l,SpriteSheetAnimation &spriteSheetAnim) {
		auto &sequences = spriteSheetAnim.sequences;
		auto t = Lua::CreateTable(l);
		uint32_t seqIdx = 1;
		for(auto &seq : sequences)
		{
			Lua::PushInt(l,seqIdx++);
			Lua::Push<SpriteSheetAnimation::Sequence*>(l,&seq);
			Lua::SetTableValue(l,t);
		}
	}));
	materialClassDef.scope[spriteSheetDef];

	Lua::Material::register_class(materialClassDef);
	materialClassDef.def("SetTexture",static_cast<void(*)(lua_State*,Material*,const std::string&,const std::string&)>(&Lua::Material::Client::SetTexture));
	materialClassDef.def("SetTexture",static_cast<void(*)(lua_State*,Material*,const std::string&,Texture&)>(&Lua::Material::Client::SetTexture));
	materialClassDef.def("SetTexture",static_cast<void(*)(lua_State*,Material*,const std::string&,Lua::Vulkan::Texture&)>(&Lua::Material::Client::SetTexture));
	materialClassDef.def("GetTextureInfo",&Lua::Material::Client::GetTexture);
	materialClassDef.def("GetData",&Lua::Material::Client::GetData);
	materialClassDef.def("InitializeShaderDescriptorSet",static_cast<void(*)(lua_State*,::Material*,bool)>(&Lua::Material::Client::InitializeShaderData));
	materialClassDef.def("InitializeShaderDescriptorSet",static_cast<void(*)(lua_State*,::Material*)>(&Lua::Material::Client::InitializeShaderData));
	materialClassDef.def("ClearSpriteSheetAnimation",static_cast<void(*)(lua_State*,::Material&)>([](lua_State *l,::Material &mat) {
		static_cast<CMaterial&>(mat).ClearSpriteSheetAnimation();
	}));
	materialClassDef.def("GetSpriteSheetAnimation",static_cast<void(*)(lua_State*,::Material&)>([](lua_State *l,::Material &mat) {
		auto *spriteSheetAnim = static_cast<CMaterial&>(mat).GetSpriteSheetAnimation();
		if(spriteSheetAnim == nullptr)
			return;
		Lua::Push<SpriteSheetAnimation*>(l,spriteSheetAnim);
	}));
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

	Lua::RegisterLibraryEnums(lua.GetState(),"shader",{
		// These have to match shaders/modules/fs_tonemapping.gls!
		{"TONE_MAPPING_GAMMA_CORRECTION",0},
		{"TONE_MAPPING_REINHARD",1},
		{"TONE_MAPPING_HEJIL_RICHARD",2},
		{"TONE_MAPPING_UNCHARTED",3},
		{"TONE_MAPPING_ACES",4},
		{"TONE_MAPPING_GRAN_TURISMO",5},
		{"TONE_MAPPING_COUNT",6}
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
	defShaderGraphics.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex) {
		Lua::Shader::Graphics::RecordDrawIndexed(l,shader,indexCount,instanceCount,firstIndex,0u);
	}));
	defShaderGraphics.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount) {
		Lua::Shader::Graphics::RecordDrawIndexed(l,shader,indexCount,instanceCount,0u,0);
	}));
	defShaderGraphics.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,prosper::ShaderGraphics&,uint32_t)>([](lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount) {
		Lua::Shader::Graphics::RecordDrawIndexed(l,shader,indexCount,1u,0u,0);
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
	defShaderGraphics.scope[luabind::def("get_render_pass",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto &rp = prosper::ShaderGraphics::GetRenderPass<prosper::ShaderGraphics>(c_engine->GetRenderContext());
		if(rp == nullptr)
			return;
		Lua::Push(l,rp);
	}))];
	modShader[defShaderGraphics];

	auto defShaderGUITextured = luabind::class_<wgui::ShaderTextured,luabind::bases<prosper::ShaderGraphics,prosper::Shader>>("GUITextured");
	modShader[defShaderGUITextured];

	auto defShaderScene = luabind::class_<pragma::ShaderScene,luabind::bases<prosper::ShaderGraphics,prosper::Shader>>("Scene3D");
	defShaderScene.scope[luabind::def("get_render_pass",&Lua::Shader::Scene3D::GetRenderPass)];
	defShaderScene.scope[luabind::def("get_render_pass",static_cast<void(*)(lua_State*)>([](lua_State *l) {
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

	auto defShaderGlow = luabind::class_<pragma::ShaderGlow,luabind::bases<pragma::ShaderTextured3DBase,pragma::ShaderEntity,pragma::ShaderSceneLit,pragma::ShaderScene,prosper::ShaderGraphics,prosper::Shader>>("Glow");
	defShaderGlow.add_static_constant("RENDER_PASS_COLOR_FORMAT",umath::to_integral(pragma::ShaderGlow::RENDER_PASS_FORMAT));
	modShader[defShaderGlow];

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

	// Utility shaders
	auto defShaderComposeRMA = luabind::class_<pragma::ShaderComposeRMA,luabind::bases<prosper::ShaderGraphics,prosper::Shader>>("ComposeRMA");
	defShaderComposeRMA.add_static_constant("FLAG_NONE",umath::to_integral(pragma::ShaderComposeRMA::Flags::None));
	defShaderComposeRMA.add_static_constant("FLAG_USE_SPECULAR_WORKFLOW_BIT",umath::to_integral(pragma::ShaderComposeRMA::Flags::UseSpecularWorkflow));
	defShaderComposeRMA.def("ComposeRMA",static_cast<void(*)(lua_State*,pragma::ShaderComposeRMA&,prosper::Texture*,prosper::Texture*,prosper::Texture*,uint32_t)>(
		[](lua_State *l,pragma::ShaderComposeRMA &shader,prosper::Texture *roughnessMap,prosper::Texture *metalnessMap,prosper::Texture *aoMap,uint32_t flags) {
		auto rma = shader.ComposeRMA(c_engine->GetRenderContext(),roughnessMap,metalnessMap,aoMap,static_cast<pragma::ShaderComposeRMA::Flags>(flags));
		if(rma == nullptr)
			return;
		Lua::Push(l,rma);
	}));
	modShader[defShaderComposeRMA];

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

	auto defShaderBasePipelineCreateInfo = luabind::class_<prosper::BasePipelineCreateInfo>("BasePipelineCreateInfo");
	defShaderBasePipelineCreateInfo.def("AttachPushConstantRange",&Lua::BasePipelineCreateInfo::AttachPushConstantRange);
	defShaderBasePipelineCreateInfo.def("AttachDescriptorSetInfo",&Lua::BasePipelineCreateInfo::AttachDescriptorSetInfo);
	modShader[defShaderBasePipelineCreateInfo];

	auto defShaderGraphicsPipelineCreateInfo = luabind::class_<prosper::GraphicsPipelineCreateInfo,prosper::BasePipelineCreateInfo>("GraphicsPipelineCreateInfo");
	defShaderGraphicsPipelineCreateInfo.def("SetBlendingProperties",&Lua::GraphicsPipelineCreateInfo::SetBlendingProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetCommonAlphaBlendProperties",&Lua::GraphicsPipelineCreateInfo::SetCommonAlphaBlendProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetColorBlendAttachmentProperties",&Lua::GraphicsPipelineCreateInfo::SetColorBlendAttachmentProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetMultisamplingProperties",&Lua::GraphicsPipelineCreateInfo::SetMultisamplingProperties);
	defShaderGraphicsPipelineCreateInfo.def("SetSampleCount",&Lua::GraphicsPipelineCreateInfo::SetSampleCount);
	defShaderGraphicsPipelineCreateInfo.def("SetSampleShadingEnabled",&Lua::GraphicsPipelineCreateInfo::SetSampleShadingEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetSampleMaskEnabled",&Lua::GraphicsPipelineCreateInfo::SetSampleMaskEnabled);
	defShaderGraphicsPipelineCreateInfo.def("SetDynamicScissorBoxesCount",&Lua::GraphicsPipelineCreateInfo::SetDynamicScissorBoxesCount);
	defShaderGraphicsPipelineCreateInfo.def("SetDynamicViewportsCount",&Lua::GraphicsPipelineCreateInfo::SetDynamicViewportsCount);
	defShaderGraphicsPipelineCreateInfo.def("SetPrimitiveTopology",&Lua::GraphicsPipelineCreateInfo::SetPrimitiveTopology);
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
	defShaderGraphicsPipelineCreateInfo.def("GetScissorBoxesCount",&Lua::GraphicsPipelineCreateInfo::GetScissorBoxesCount);
	defShaderGraphicsPipelineCreateInfo.def("GetViewportsCount",&Lua::GraphicsPipelineCreateInfo::GetViewportsCount);
	defShaderGraphicsPipelineCreateInfo.def("GetPrimitiveTopology",&Lua::GraphicsPipelineCreateInfo::GetPrimitiveTopology);
	defShaderGraphicsPipelineCreateInfo.def("GetPushConstantRanges",&Lua::GraphicsPipelineCreateInfo::GetPushConstantRanges);
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

	auto defShaderComputePipelineCreateInfo = luabind::class_<prosper::ComputePipelineCreateInfo,prosper::BasePipelineCreateInfo>("ComputePipelineCreateInfo");
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

	auto defShaderGUITexturedBase = luabind::class_<pragma::LuaShaderGUITextured,luabind::bases<pragma::LuaShaderGraphicsBase,wgui::ShaderTextured,prosper::ShaderGraphics,prosper::Shader,pragma::LuaShaderBase>>("BaseGUITextured");
	defShaderGUITexturedBase.def(luabind::constructor<>());
	modShader[defShaderGUITexturedBase];

	auto defShaderParticleBase = luabind::class_<pragma::LuaShaderGUIParticle2D,luabind::bases<pragma::LuaShaderGraphicsBase,pragma::ShaderSceneLit,pragma::ShaderScene,prosper::ShaderGraphics,prosper::Shader,pragma::LuaShaderBase>>("BaseParticle2D");
	defShaderParticleBase.scope[luabind::def("get_depth_pipeline_render_pass",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto &rp = prosper::ShaderGraphics::GetRenderPass<pragma::ShaderParticle2DBase>(c_engine->GetRenderContext(),pragma::ShaderParticle2DBase::GetDepthPipelineIndex());
		if(rp == nullptr)
			return;
		Lua::Push(l,rp);
	}))];
	defShaderParticleBase.def(luabind::constructor<>());
	defShaderParticleBase.add_static_constant("PUSH_CONSTANTS_SIZE",sizeof(pragma::ShaderParticle2DBase::PushConstants));
	defShaderParticleBase.add_static_constant("PUSH_CONSTANTS_USER_DATA_OFFSET",sizeof(pragma::ShaderParticle2DBase::PushConstants));
	defShaderParticleBase.def("RecordDraw",static_cast<void(*)(lua_State*,pragma::LuaShaderGUIParticle2D&,pragma::rendering::RasterizationRenderer&,CParticleSystemHandle&,uint32_t)>([](lua_State *l,pragma::LuaShaderGUIParticle2D &shader,pragma::rendering::RasterizationRenderer &renderer,CParticleSystemHandle &ps,uint32_t renderFlags) {
		pragma::Lua::check_component(l,ps);
		Lua::PushBool(l,shader.Draw(renderer,*ps,ps->GetOrientationType(),static_cast<pragma::ParticleRenderFlags>(renderFlags)));
	}));
	defShaderParticleBase.def("RecordBeginDraw",static_cast<void(*)(lua_State*,pragma::LuaShaderGUIParticle2D&,Lua::Vulkan::CommandBuffer&,CParticleSystemHandle&,uint32_t)>([](lua_State *l,pragma::LuaShaderGUIParticle2D &shader,Lua::Vulkan::CommandBuffer &drawCmd,CParticleSystemHandle &ps,uint32_t renderFlags) {
		pragma::Lua::check_component(l,ps);
		if(drawCmd.IsPrimary() == false)
		{
			Lua::PushBool(l,false);
			return;
		}
		Lua::PushBool(l,shader.BeginDraw(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd.shared_from_this()),*ps,static_cast<pragma::ParticleRenderFlags>(renderFlags)));
	}));

	modShader[defShaderParticleBase];

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
		if(commandBuffer.IsPrimary() == false)
			return;
		drawSceneInfo.commandBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(commandBuffer.shared_from_this());
	}));
	defDrawSceneInfo.def_readwrite("renderFlags",reinterpret_cast<uint32_t util::DrawSceneInfo::*>(&::util::DrawSceneInfo::renderFlags));
	defDrawSceneInfo.def_readwrite("renderTarget",&::util::DrawSceneInfo::renderTarget);
	defDrawSceneInfo.def_readwrite("outputImage",&::util::DrawSceneInfo::outputImage);
	defDrawSceneInfo.def_readwrite("outputLayerId",reinterpret_cast<uint32_t util::DrawSceneInfo::*>(&::util::DrawSceneInfo::outputLayerId));
	defDrawSceneInfo.property("clearColor",static_cast<Color(*)(::util::DrawSceneInfo&)>([](::util::DrawSceneInfo &drawSceneInfo) -> Color {
		return *drawSceneInfo.clearColor;
	}),static_cast<void(*)(::util::DrawSceneInfo&,const Color&)>([](::util::DrawSceneInfo &drawSceneInfo,const Color &color) {
		drawSceneInfo.clearColor = color;
	}));
	defDrawSceneInfo.def("SetEntityRenderFilter",static_cast<void(*)(lua_State*,util::DrawSceneInfo&,luabind::object)>([](lua_State *l,util::DrawSceneInfo &drawSceneInfo,luabind::object f) {
		Lua::CheckFunction(l,2);
		drawSceneInfo.renderFilter = [f,l](CBaseEntity &ent) -> bool {
			auto r = Lua::CallFunction(l,[&f,&ent](lua_State *l) {
				f.push(l);
				ent.GetLuaObject()->push(l);
				return Lua::StatusCode::Ok;
				},1);
			if(r == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return false;
				return Lua::CheckBool(l,-1);
			}
			return true;
		};
	}));
	defDrawSceneInfo.def("SetEntityPrepassFilter",static_cast<void(*)(lua_State*,util::DrawSceneInfo&,luabind::object)>([](lua_State *l,util::DrawSceneInfo &drawSceneInfo,luabind::object f) {
		Lua::CheckFunction(l,2);
		drawSceneInfo.prepassFilter = [f,l](CBaseEntity &ent) -> bool {
			auto r = Lua::CallFunction(l,[&f,&ent](lua_State *l) {
				f.push(l);
				ent.GetLuaObject()->push(l);
				return Lua::StatusCode::Ok;
				},1);
			if(r == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return false;
				return Lua::CheckBool(l,-1);
			}
			return true;
		};
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

	auto defMdlExportInfo = luabind::class_<pragma::asset::ModelExportInfo>("ExportInfo");
	defMdlExportInfo.add_static_constant("IMAGE_FORMAT_PNG",umath::to_integral(pragma::asset::ModelExportInfo::ImageFormat::PNG));
	defMdlExportInfo.add_static_constant("IMAGE_FORMAT_BMP",umath::to_integral(pragma::asset::ModelExportInfo::ImageFormat::BMP));
	defMdlExportInfo.add_static_constant("IMAGE_FORMAT_TGA",umath::to_integral(pragma::asset::ModelExportInfo::ImageFormat::TGA));
	defMdlExportInfo.add_static_constant("IMAGE_FORMAT_JPG",umath::to_integral(pragma::asset::ModelExportInfo::ImageFormat::JPG));
	defMdlExportInfo.add_static_constant("IMAGE_FORMAT_HDR",umath::to_integral(pragma::asset::ModelExportInfo::ImageFormat::HDR));
	defMdlExportInfo.add_static_constant("IMAGE_FORMAT_DDS",umath::to_integral(pragma::asset::ModelExportInfo::ImageFormat::DDS));
	defMdlExportInfo.add_static_constant("IMAGE_FORMAT_KTX",umath::to_integral(pragma::asset::ModelExportInfo::ImageFormat::KTX));

	defMdlExportInfo.add_static_constant("DEVICE_CPU",umath::to_integral(pragma::rendering::cycles::SceneInfo::DeviceType::CPU));
	defMdlExportInfo.add_static_constant("DEVICE_GPU",umath::to_integral(pragma::rendering::cycles::SceneInfo::DeviceType::GPU));
	defMdlExportInfo.def(luabind::constructor<>());
	defMdlExportInfo.def_readwrite("exportAnimations",&pragma::asset::ModelExportInfo::exportAnimations);
	defMdlExportInfo.def_readwrite("exportSkinnedMeshData",&pragma::asset::ModelExportInfo::exportSkinnedMeshData);
	defMdlExportInfo.def_readwrite("exportMorphTargets",&pragma::asset::ModelExportInfo::exportMorphTargets);
	defMdlExportInfo.def_readwrite("exportImages",&pragma::asset::ModelExportInfo::exportImages);
	defMdlExportInfo.def_readwrite("embedAnimations",&pragma::asset::ModelExportInfo::embedAnimations);
	defMdlExportInfo.def_readwrite("fullExport",&pragma::asset::ModelExportInfo::fullExport);
	defMdlExportInfo.def_readwrite("normalizeTextureNames",&pragma::asset::ModelExportInfo::normalizeTextureNames);
	defMdlExportInfo.def_readwrite("enableExtendedDDS",&pragma::asset::ModelExportInfo::enableExtendedDDS);
	defMdlExportInfo.def_readwrite("saveAsBinary",&pragma::asset::ModelExportInfo::saveAsBinary);
	defMdlExportInfo.def_readwrite("verbose",&pragma::asset::ModelExportInfo::verbose);
	defMdlExportInfo.def_readwrite("generateAo",&pragma::asset::ModelExportInfo::generateAo);
	defMdlExportInfo.def_readwrite("aoSamples",&pragma::asset::ModelExportInfo::aoSamples);
	defMdlExportInfo.def_readwrite("aoResolution",&pragma::asset::ModelExportInfo::aoResolution);
	defMdlExportInfo.def_readwrite("scale",&pragma::asset::ModelExportInfo::scale);
	defMdlExportInfo.def_readwrite("mergeMeshesByMaterial",&pragma::asset::ModelExportInfo::mergeMeshesByMaterial);
	defMdlExportInfo.def_readwrite("imageFormat",reinterpret_cast<std::underlying_type_t<decltype(pragma::asset::ModelExportInfo::imageFormat)> pragma::asset::ModelExportInfo::*>(&pragma::asset::ModelExportInfo::imageFormat));
	defMdlExportInfo.def_readwrite("aoDevice",reinterpret_cast<std::underlying_type_t<decltype(pragma::asset::ModelExportInfo::aoDevice)> pragma::asset::ModelExportInfo::*>(&pragma::asset::ModelExportInfo::aoDevice));
	defMdlExportInfo.def("SetAnimationList",static_cast<void(*)(lua_State*,pragma::asset::ModelExportInfo&,luabind::object)>([](lua_State *l,pragma::asset::ModelExportInfo &exportInfo,luabind::object oTable) {
		int32_t t = 2;
		auto n = Lua::GetObjectLength(l,t);
		std::vector<std::string> anims {};
		anims.reserve(n);
		for(auto i=decltype(n){0u};i<n;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,t);
			anims.push_back(Lua::CheckString(l,-1));

			Lua::Pop(l,1);
		}
		exportInfo.SetAnimationList(anims);
	}));
	modelClassDef.scope[defMdlExportInfo];

	Lua::Model::register_class(GetLuaState(),modelClassDef,modelMeshClassDef,subModelMeshClassDef);
	modelClassDef.def("AddMaterial",&Lua::Model::Client::AddMaterial);
	modelClassDef.def("SetMaterial",&Lua::Model::Client::SetMaterial);
	modelClassDef.def("GetVertexAnimationBuffer",&Lua::Model::Client::GetVertexAnimationBuffer);
	modelClassDef.def("Export",&Lua::Model::Client::Export);
	modelClassDef.def("ExportAnimation",&Lua::Model::Client::ExportAnimation);
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
