#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/c_shader.h"
//#include "shader.h" // prosper TODO
#include <pragma/console/convars.h>
#include "pragma/rendering/uniformbinding.h"
//#include "shader_screen.h" // prosper TODO
#include "pragma/rendering/shaders/world/c_shader_wireframe.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured_alpha_transition.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug_text.hpp"
#include "pragma/rendering/shaders/c_shader_cubemap.h"
#include "pragma/rendering/shaders/world/c_shader_flat.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/c_shader_depth_to_rgb.h"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_model.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_blob.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_indexing.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/shaders/image/c_shader_calc_image_color.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/image/c_shader_clear_color.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_water.hpp"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/shaders/world/c_shader_loading.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_splash.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_integrate.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_solve_edges.hpp"
#include "pragma/rendering/shaders/world/c_shader_light_cone.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/shaders/c_shader_equirectangular_to_cubemap.hpp"
#include "pragma/rendering/shaders/c_shader_cubemap_to_equirectangular.hpp"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/world/c_shader_unlit.hpp"
#include "pragma/rendering/shaders/c_shader_convolute_cubemap_lighting.hpp"
#include "pragma/rendering/shaders/c_shader_compute_irradiance_map_roughness.hpp"
#include "pragma/rendering/shaders/c_shader_brdf_convolution.hpp"
#include "pragma/rendering/shaders/util/c_shader_specular_to_roughness.hpp"
#include "pragma/rendering/shaders/util/c_shader_extract_diffuse_ambient_occlusion.hpp"
#include <pragma/console/convars.h>
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/world_environment.hpp"
#include <buffers/prosper_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

REGISTER_CONVAR_CALLBACK_CL(cl_render_shader_quality,[](NetworkState*,ConVar*,int,int val) {
	if(c_game == nullptr)
		return;
	c_game->GetWorldEnvironment().SetShaderQuality(val);
});

static void CVAR_CALLBACK_render_unlit(NetworkState*,ConVar*,bool,bool val)
{
	if(c_game == nullptr)
		return;
	c_game->GetWorldEnvironment().SetUnlit(val);
}
REGISTER_CONVAR_CALLBACK_CL(render_unlit,CVAR_CALLBACK_render_unlit);

static void CVAR_CALLBACK_cl_render_shadow_resolution(NetworkState*,ConVar*,int,int val)
{
	if(c_game == nullptr)
		return;
	c_game->GetWorldEnvironment().SetShadowResolution(val);
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_resolution,CVAR_CALLBACK_cl_render_shadow_resolution);

void CGame::InitShaders()
{
	Con::cout<<"Loading shaders..."<<Con::endl;
	
	auto &shaderManager = c_engine->GetShaderManager();
	pragma::ShaderScene::SetRenderPassSampleCount(static_cast<Anvil::SampleCountFlagBits>(GetMSAASampleCount()));

	shaderManager.RegisterShader("clear_color",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderClearColor(context,identifier);});

	shaderManager.RegisterShader("textured",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderTextured3D(context,identifier);});
	shaderManager.RegisterShader("prepass",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderPrepass(context,identifier);});
	shaderManager.RegisterShader("prepass_depth",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderPrepassDepth(context,identifier);});

	shaderManager.RegisterShader("forwardp_light_indexing",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderForwardPLightIndexing(context,identifier);});
	shaderManager.RegisterShader("forwardp_light_culling",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderForwardPLightCulling(context,identifier);});

	shaderManager.RegisterShader("raytracing",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderRayTracing(context,identifier);});
	shaderManager.RegisterShader("pbr",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderPBR(context,identifier);});

	shaderManager.RegisterShader("flat",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderFlat(context,identifier);});
	shaderManager.RegisterShader("unlit",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderUnlit(context,identifier);});
	shaderManager.RegisterShader("wireframe",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderWireframe(context,identifier);});
	shaderManager.RegisterShader("texturedalphatransition",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderTexturedAlphaTransition(context,identifier);});
	shaderManager.RegisterShader("skybox",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderSkybox(context,identifier);});
	shaderManager.RegisterShader("skybox_equirect",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderSkyboxEquirect(context,identifier);});
	shaderManager.RegisterShader("loading",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderLoading(context,identifier);});
	shaderManager.RegisterShader("light_cone",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderLightCone(context,identifier);});
	shaderManager.RegisterShader("water",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderWater(context,identifier);});

	m_gameShaders.at(umath::to_integral(GameShader::Debug)) = shaderManager.RegisterShader("debug",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderDebug(context,identifier);});
	m_gameShaders.at(umath::to_integral(GameShader::DebugTexture)) = shaderManager.RegisterShader("debug_texture",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderDebugTexture(context,identifier);});
	m_gameShaders.at(umath::to_integral(GameShader::DebugVertex)) = shaderManager.RegisterShader("debug_vertex",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderDebugVertexColor(context,identifier);});
	shaderManager.RegisterShader("debug_depth_to_rgb",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderDepthToRGB(context,identifier);});
	shaderManager.RegisterShader("debug_cube_depth_to_rgb",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderCubeDepthToRGB(context,identifier);});
	shaderManager.RegisterShader("debug_csm_depth_to_rgb",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderCSMDepthToRGB(context,identifier);});

	shaderManager.RegisterShader("particle",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderParticle(context,identifier);});
	shaderManager.RegisterShader("particle_rotational",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderParticleRotational(context,identifier);});
	//shaderManager.RegisterShader("particlemodel",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderParticleModel(context,identifier);});
	shaderManager.RegisterShader("particlepolyboard",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderParticlePolyboard(context,identifier);});

	m_gameShaders.at(umath::to_integral(GameShader::Shadow)) = shaderManager.RegisterShader("shadow",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderShadow(context,identifier);});
	m_gameShaders.at(umath::to_integral(GameShader::ShadowCSM)) = shaderManager.RegisterShader("shadowcsm",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderShadowCSM(context,identifier);});
	m_gameShaders.at(umath::to_integral(GameShader::ShadowSpot)) = shaderManager.RegisterShader("shadow_spot",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderShadowSpot(context,identifier);});
	// TODO: Transparent shaders
	//shaderManager.RegisterShader("hdr",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderHDR(context,identifier);});
	
	shaderManager.RegisterShader("equirectangular_to_cubemap",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderEquirectangularToCubemap(context,identifier);});
	shaderManager.RegisterShader("cubemap_to_equirectangular",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderCubemapToEquirectangular(context,identifier);});
	shaderManager.RegisterShader("convolute_cubemap_lighting",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderConvoluteCubemapLighting(context,identifier);});
	shaderManager.RegisterShader("compute_irradiance_map_roughness",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderComputeIrradianceMapRoughness(context,identifier);});
	shaderManager.RegisterShader("brdf_convolution",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderBRDFConvolution(context,identifier);});
	shaderManager.RegisterShader("specular_to_roughness",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderSpecularToRoughness(context,identifier);});
	shaderManager.RegisterShader("extract_diffuse_ambient_occlusion",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderExtractDiffuseAmbientOcclusion(context,identifier);});

	shaderManager.RegisterShader("calcimagecolor",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderCalcImageColor(context,identifier);});
	shaderManager.RegisterShader("watersplash",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderWaterSplash(context,identifier);});
	shaderManager.RegisterShader("watersurface",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderWaterSurface(context,identifier);});
	shaderManager.RegisterShader("watersurfaceintegrate",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderWaterSurfaceIntegrate(context,identifier);});
	shaderManager.RegisterShader("watersurfacesumedges",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderWaterSurfaceSumEdges(context,identifier);});
	shaderManager.RegisterShader("watersurfacesolveedges",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderWaterSurfaceSolveEdges(context,identifier);});

	shaderManager.RegisterShader("ssao",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderSSAO(context,identifier);});
	shaderManager.RegisterShader("ssao_blur",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderSSAOBlur(context,identifier);});
	shaderManager.RegisterShader("glow",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderGlow(context,identifier);});
	m_gameShaders.at(umath::to_integral(GameShader::PPTonemapping)) = shaderManager.RegisterShader("pp_hdr",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderPPHDR(context,identifier);});
	m_gameShaders.at(umath::to_integral(GameShader::PPFog)) = shaderManager.RegisterShader("pp_fog",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderPPFog(context,identifier);});
	shaderManager.RegisterShader("pp_water",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderPPWater(context,identifier);});
	m_gameShaders.at(umath::to_integral(GameShader::PPFXAA)) = shaderManager.RegisterShader("pp_fxaa",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderPPFXAA(context,identifier);});


	//shaderManager.RegisterShader("particleblob",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderParticleBlob(context,identifier);}); // prosper TODO
	// shaderManager.RegisterShader("debug_text",[](prosper::Context &context,const std::string &identifier) {return new pragma::ShaderDebugText(context,identifier);}); // Deprecated?
	/*c_engine->LoadShader("screen");
	c_engine->LoadShader("screen_clip");
	c_engine->LoadShader("pp_water");
	c_engine->LoadShader("pp_fog");
	c_engine->LoadShader("pp_hdr");
	c_engine->LoadShader("textured");
	c_engine->LoadShader("hdr");
	c_engine->LoadShader("additive");
	c_engine->LoadShader("prepass");
	c_engine->LoadShader("prepass_depth");
	c_engine->LoadShader("forwardp_light_culling");
	c_engine->LoadShader("forwardp_light_indexing");
	c_engine->LoadShader("ssao");
	c_engine->LoadShader("ssao_blur");
	c_engine->LoadShader("refraction");
	///c_engine->LoadShader("depthcombine");
	c_engine->LoadShader("particle");
	c_engine->LoadShader("particleshadow");
	c_engine->LoadShader("particleunlit");
	c_engine->LoadShader("particlepolyboard");
	c_engine->LoadShader("particlepolyboardshadow");
	c_engine->LoadShader("particleblob");
	c_engine->LoadShader("particleblobshadow");
	c_engine->LoadShader("particlemodel");
	c_engine->LoadShader("particlemodelshadow");
	c_engine->LoadShader("skybox");
	c_engine->LoadShader("shadow");
	c_engine->LoadShader("shadow_spot");
	//c_engine->LoadShader("shadowtransparent");
	//c_engine->LoadShader("shadowtransparent_spot");
	c_engine->LoadShader("shadowcsm");
	c_engine->LoadShader("shadowcsmtransparent");
	c_engine->LoadShader("shadowcsmstatic");
	c_engine->LoadShader("calcimagecolor");
	c_engine->LoadShader("watersurface");
	c_engine->LoadShader("watersplash");
	c_engine->LoadShader("watersurfaceintegrate");
	c_engine->LoadShader("watersurfacesolveedges");
	c_engine->LoadShader("watersurfacesumedges");
	c_engine->LoadShader("water_surface_refraction");
	c_engine->LoadShader("depthtorgb");
	c_engine->LoadShader("cubedepthtorgb");
	c_engine->LoadShader("csmdepthtorgb");
	c_engine->LoadShader("texturedalphatransition");
	c_engine->LoadShader("wireframe");
	c_engine->LoadShader("debug");
	c_engine->LoadShader("debugline");
	c_engine->LoadShader("debugtext");
	c_engine->LoadShader("debugtriangleline");
	c_engine->LoadShader("debuglinestrip");
	c_engine->LoadShader("debugpoint");
	c_engine->LoadShader("debugpointvertex");
	c_engine->LoadShader("debugnormals");
	c_engine->LoadShader("debug_multisample_image");
	c_engine->LoadShader("debug_multisample_depth");
	c_engine->LoadShader("glow");
	c_engine->LoadShader("loading");
	c_engine->LoadShader("light_cone");
	c_engine->LoadShader("water");
	///c_engine->LoadShader("occlusion");*/ // prosper TODO
}

/*const Vulkan::Buffer *CGame::GetUniformBlockBuffer(UniformBinding id) const
{
	auto &context = c_engine->GetRenderContext();
	return c_engine->GetUniformBlockManager().GetBuffer(umath::to_integral(id),context.GetFrameSwapIndex());
}
const Vulkan::SwapBuffer *CGame::GetUniformBlockSwapBuffer(UniformBinding id) const {return c_engine->GetUniformBlockManager().GetSwapBuffer(umath::to_integral(id));}
const Vulkan::Std140LayoutBlockData *CGame::GetUniformBlockLayout(UniformBinding id) const {return c_engine->GetUniformBlockManager().GetBufferLayout(umath::to_integral(id));}
*/ // prosper TODO
void CGame::UpdateShaderTimeData()
{
	c_engine->ScheduleRecordUpdateBuffer(m_globalRenderSettingsBufferData->timeBuffer,0ull,pragma::ShaderTextured3DBase::TimeData{
		static_cast<float>(CurTime()),static_cast<float>(DeltaTime()),
		static_cast<float>(RealTime()),static_cast<float>(DeltaRealTime())
	});
}
