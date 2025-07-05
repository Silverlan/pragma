-- SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local Shader = util.register_class("shader.ExampleSimpleGameShader", shader.BaseTexturedLit3D)

Shader.FragmentShader = "examples/fs_example_simple_game_shader"
Shader.VertexShader = "world/vs_textured"
shader.register("example_simple_game_shader", Shader)
