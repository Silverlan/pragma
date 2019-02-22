#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/post_processing/c_shader_postprocessing.h"
#include "cmaterialmanager.h"
 // prosper TODO
#if 0
#include "pragma/model/c_side.h"
#include <texturemanager/texturemanager.h>
#include "pragma/console/c_cvar.h"
#include "textureinfo.h"


using namespace Shader;

LINK_SHADER_TO_CLASS(PostProcessing,postprocessing);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static CVar cvNightvision = GetClientConVar("render_pp_nightvision");
static CVar cvLuminanceThreshold = GetClientConVar("render_pp_nightvision_luminance_threshold");
static CVar cvColorAmplification = GetClientConVar("render_pp_nightvision_color_amplification");
static CVar cvNoiseTexture = GetClientConVar("render_pp_nightvision_noise_texture");

PostProcessing::PostProcessing()
	: Base("postprocessing","screen/vs_screen_uv","screen/fs_postprocessing"),
	m_matNoise()
{
	/*m_tmpFrameBuffer = OpenGL::GenerateFrameBuffer();
	m_tmpRenderTexture = OpenGL::GenerateTexture();

	unsigned int frameBuffer = m_tmpFrameBuffer;
	unsigned int texture = m_tmpRenderTexture;

	OpenGL::BindFrameBuffer(frameBuffer);
	OpenGL::BindTexture(texture);
	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	OpenGL::Set2DTextureImage(
		GL_TEXTURE_2D,0,
		GL_RGB16F,w,h, // GL_RGB16F Required for HDR
		0,GL_RGB,
		GL_FLOAT,// GL_UNSIGNED_BYTE Required for HDR (Do we need it for post-processing though?)
		0
	);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    OpenGL::SetTextureParameter(GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	OpenGL::AttachFrameBufferTexture2D(GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
	OpenGL::AttachFrameBufferTexture2D(GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,0,0);

	OpenGL::BindTexture(0);
	OpenGL::BindFrameBuffer(0);

	m_vao = GLVertexArray::Create();
	if(m_vao != nullptr)
	{
		m_vao->Bind();
		auto vertexbuffer = c_game->GetScreenVertexBuffer();
		OpenGL::EnableVertexAttribArray(SHADER_VERTEX_BUFFER_LOCATION);
		OpenGL::BindBuffer(vertexbuffer);
		OpenGL::SetVertexAttribData(
			SHADER_VERTEX_BUFFER_LOCATION,
			3,
			GL_FLOAT,
			GL_FALSE,
			(void*)0
		);
		m_vao->Unbind();
	}*/ // Vulkan TODO
}

void PostProcessing::InitializeShader()
{
	/*Base::InitializeShader();
	unsigned int locRenderTex = OpenGL::GetUniformLocation(m_shader,"u_texture");
	OpenGL::SetUniform1i(locRenderTex,0);

	unsigned int texNoise = OpenGL::GetUniformLocation(m_shader,"u_noiseTexture");
	OpenGL::SetUniform1i(texNoise,1);

	m_locLuminanceThreshold = GLUniformValue<float>(m_shader,"u_luminanceThreshold",0.f);
	m_locColorAmplification = GLUniformValue<float>(m_shader,"u_colorAmplification",0.f);
	m_locNightvisionEnabled = GLUniformValue<int>(m_shader,"u_nightvisionEnabled",0);
	
	SetNightVisionEnabled(cvNightvision->GetBool());
	SetNightVisionLuminanceThreshold(cvLuminanceThreshold->GetFloat());
	SetNightVisionColorAmplification(cvColorAmplification->GetFloat());
	SetNightVisionNoiseTexture(cvNoiseTexture->GetString());*/ // Vulkan TODO
}

void PostProcessing::SetNightVisionEnabled(bool)
{
	//Use();
	//m_locNightvisionEnabled.SetValue((b == true) ? 1 : 0); // Vulkan TODO
}
void PostProcessing::SetNightVisionLuminanceThreshold(float)
{
	//Use();
	//m_locLuminanceThreshold.SetValue(threshold); // Vulkan TODO
}
void PostProcessing::SetNightVisionColorAmplification(float)
{
	//Use();
	//m_locColorAmplification.SetValue(amplification); // Vulkan TODO
}
void PostProcessing::SetNightVisionNoiseTexture(const std::string&)
{
	//Use(); // Vulkan TODO
	//m_texNoise = tex.c_str();
	//ReloadNightVisionNoiseTexture();
}
void PostProcessing::ReloadNightVisionNoiseTexture()
{
	/*auto *mat = CMaterialSystem::Load(m_texNoise.c_str());
	if(mat == nullptr)
	{
		m_matNoise = MaterialHandle();
		SetNightVisionEnabled(false);
	}
	else
	{
		m_matNoise = mat->GetHandle();
		SetNightVisionEnabled(cvNightvision->GetBool());
	}*/ // Vulkan TODO
}

void PostProcessing::Render(unsigned int,unsigned int)
{
	/*if(m_vao == nullptr)
		return;
	OpenGL::BindFrameBuffer(m_tmpFrameBuffer);
	Use();
	if(m_matNoise != nullptr)
	{
		auto *diffuse = m_matNoise->GetDiffuseMap();
		if(diffuse != nullptr && diffuse->texture != nullptr)
		{
			auto *texture = static_cast<Texture*>(diffuse->texture);
			OpenGL::SetActiveTexture(GL_TEXTURE1);
			OpenGL::BindTexture(texture->GetTextureID());
		}
	}
	OpenGL::SetActiveTexture(GL_TEXTURE0);
	OpenGL::BindTexture(screenTexture);

	m_vao->Bind();
	OpenGL::DrawArrays(GL_TRIANGLES,0,6);
	m_vao->Unbind();
	OpenGL::BindTexture(0);
	OpenGL::BindFrameBuffer(frameBuffer,GL_DRAW_FRAMEBUFFER);

	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	OpenGL::BlitFrameBuffer(w,h);*/ // Vulkan TODO
}

////////////////////////////////////////

static void CVAR_CALLBACK_render_pp_nightvision(NetworkState*,ConVar*,bool,bool)
{
	/*if(c_game == NULL)
		return;
	ShaderBase *shaderPP = c_game->GetShader("postprocessing");
	ShaderPostProcessing *pp = static_cast<ShaderPostProcessing*>(shaderPP);
	pp->SetNightVisionEnabled(val);*/ // Vulkan TODO
}
REGISTER_CONVAR_CALLBACK_CL(render_pp_nightvision,CVAR_CALLBACK_render_pp_nightvision);

static void CVAR_CALLBACK_render_pp_nightvision_luminance_threshold(NetworkState*,ConVar*,float,float)
{
	/*if(c_game == NULL)
		return;
	ShaderBase *shaderPP = c_game->GetShader("postprocessing");
	ShaderPostProcessing *pp = static_cast<ShaderPostProcessing*>(shaderPP);
	pp->SetNightVisionLuminanceThreshold(val);*/ // Vulkan TODO
}
REGISTER_CONVAR_CALLBACK_CL(render_pp_nightvision_luminance_threshold,CVAR_CALLBACK_render_pp_nightvision_luminance_threshold);

static void CVAR_CALLBACK_render_pp_nightvision_color_amplification(NetworkState*,ConVar*,float,float)
{
	/*if(c_game == NULL)
		return;
	ShaderBase *shaderPP = c_game->GetShader("postprocessing");
	ShaderPostProcessing *pp = static_cast<ShaderPostProcessing*>(shaderPP);
	pp->SetNightVisionColorAmplification(val);*/ // Vulkan TODO
}
REGISTER_CONVAR_CALLBACK_CL(render_pp_nightvision_color_amplification,CVAR_CALLBACK_render_pp_nightvision_color_amplification);

static void CVAR_CALLBACK_render_pp_nightvision_noise_texture(NetworkState*,ConVar*,std::string,std::string)
{
	/*if(c_game == NULL)
		return;
	ShaderBase *shaderPP = c_game->GetShader("postprocessing");
	ShaderPostProcessing *pp = static_cast<ShaderPostProcessing*>(shaderPP);
	pp->SetNightVisionNoiseTexture(val);*/ // Vulkan TODO
}
REGISTER_CONVAR_CALLBACK_CL(render_pp_nightvision_noise_texture,CVAR_CALLBACK_render_pp_nightvision_noise_texture);
#endif