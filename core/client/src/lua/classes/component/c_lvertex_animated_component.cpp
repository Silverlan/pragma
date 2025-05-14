/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include "pragma/model/c_modelmesh.h"
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_swap_buffer.hpp>

void Lua::VertexAnimated::register_class(lua_State *l, luabind::module_ &entsMod)
{
	auto defCVertexAnimated = pragma::lua::create_entity_component_class<pragma::CVertexAnimatedComponent, pragma::BaseEntityComponent>("VertexAnimatedComponent");
	defCVertexAnimated.def("UpdateVertexAnimationBuffer", static_cast<void (*)(lua_State *, pragma::CVertexAnimatedComponent &, std::shared_ptr<prosper::ICommandBuffer> &)>([](lua_State *l, pragma::CVertexAnimatedComponent &hAnim, std::shared_ptr<prosper::ICommandBuffer> &drawCmd) {
		if(drawCmd->IsPrimary() == false)
			return;
		hAnim.UpdateVertexAnimationBuffer(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd));
	}));
	defCVertexAnimated.def("GetVertexAnimationBuffer", &pragma::CVertexAnimatedComponent::GetVertexAnimationBuffer);
	defCVertexAnimated.def("GetVertexAnimationBufferMeshOffset",
	  static_cast<Lua::opt<luabind::mult<uint32_t, uint32_t>> (*)(lua_State *, pragma::CVertexAnimatedComponent &, std::shared_ptr<::ModelSubMesh> &)>(
	    [](lua_State *l, pragma::CVertexAnimatedComponent &hAnim, std::shared_ptr<::ModelSubMesh> &subMesh) -> Lua::opt<luabind::mult<uint32_t, uint32_t>> {
		    uint32_t offset;
		    uint32_t animCount;
		    auto b = hAnim.GetVertexAnimationBufferMeshOffset(static_cast<CModelSubMesh &>(*subMesh), offset, animCount);
		    if(b == false)
			    return nil;
		    return luabind::mult<uint32_t, uint32_t> {l, offset, animCount};
	    }));
	defCVertexAnimated.def("GetLocalVertexPosition",
	  static_cast<Lua::opt<luabind::mult<Vector3, Vector3>> (*)(lua_State *, pragma::CVertexAnimatedComponent &, std::shared_ptr<::ModelSubMesh> &, uint32_t)>(
	    [](lua_State *l, pragma::CVertexAnimatedComponent &hAnim, std::shared_ptr<::ModelSubMesh> &subMesh, uint32_t vertexId) -> Lua::opt<luabind::mult<Vector3, Vector3>> {
		    Vector3 pos, n;
		    auto b = hAnim.GetLocalVertexPosition(static_cast<CModelSubMesh &>(*subMesh), vertexId, pos, &n);
		    if(b == false)
			    return nil;
		    return luabind::mult<Vector3, Vector3> {l, pos, n};
	    }));
	entsMod[defCVertexAnimated];
}
