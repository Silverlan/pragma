-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local Component = util.register_class("ents.RenderTargetComponent", BaseEntityComponent)
ents.COMPONENT_RENDER_TARGET =
	ents.register_component("render_target", Component, ents.EntityComponent.FREGISTER_BIT_NETWORKED)
