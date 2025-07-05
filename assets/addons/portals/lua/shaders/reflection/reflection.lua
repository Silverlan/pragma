-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local Shader = util.register_class("shader.Reflection",shader.BasePbr)

Shader.FragmentShader = "reflection/fs_reflection"
Shader.VertexShader = "world/vs_textured"
shader.register("reflection",Shader)
