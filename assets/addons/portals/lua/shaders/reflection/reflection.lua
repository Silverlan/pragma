--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Shader = util.register_class("shader.Reflection",shader.BasePbr)

Shader.FragmentShader = "reflection/fs_reflection"
Shader.VertexShader = "world/vs_textured"
shader.register("reflection",Shader)
