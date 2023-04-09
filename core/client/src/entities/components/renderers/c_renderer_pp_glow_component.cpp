/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_renderer_pp_glow_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_bloom_blur.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <prosper_command_buffer.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CRendererPpGlowComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CRendererPpGlowComponent;
}

CRendererPpGlowComponent::CRendererPpGlowComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent)
{
}
void CRendererPpGlowComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	
}
void CRendererPpGlowComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
