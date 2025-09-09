// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_descriptor_set_group.hpp>

module pragma.client.rendering.shaders;

import :world_unlit;

import pragma.client.client_state;

extern CGame *c_game;
extern ClientState *client;
extern CEngine *c_engine;

using namespace pragma;

ShaderUnlit::ShaderUnlit(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass {context, identifier, "programs/scene/textured", "programs/scene/unlit"}
{
	m_shaderMaterialName = "albedo";
	// SetPipelineCount(umath::to_integral(Pipeline::Count));
}
