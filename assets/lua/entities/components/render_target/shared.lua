--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Component = util.register_class("ents.RenderTargetComponent",BaseEntityComponent)
ents.COMPONENT_RENDER_TARGET = ents.register_component("render_target",Component,ents.EntityComponent.FREGISTER_BIT_NETWORKED)
