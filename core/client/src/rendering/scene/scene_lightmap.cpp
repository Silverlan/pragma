#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

void Scene::SetLightMap(const std::shared_ptr<prosper::Texture> &lightMapTexture)
{
	m_lightMapInfo.lightMapTexture = lightMapTexture;
	auto &descSetCam = *(*m_camDescSetGroupGraphics)->get_descriptor_set(0u);
	auto &descSetCamView = *(*m_camViewDescSetGroup)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(descSetCam,*lightMapTexture,umath::to_integral(pragma::ShaderScene::CameraBinding::LightMap));
	prosper::util::set_descriptor_set_binding_texture(descSetCamView,*lightMapTexture,umath::to_integral(pragma::ShaderScene::CameraBinding::LightMap));
}
const std::shared_ptr<prosper::Texture> &Scene::GetLightMap() const {return m_lightMapInfo.lightMapTexture;}
