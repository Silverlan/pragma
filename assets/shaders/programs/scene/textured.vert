#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "vs_config.glsl"
#include "vs_world.glsl"
#include "/common/inputs/material.glsl"

void main() { export_world_fragment_data(use_normal_map(), use_parallax_map()); }
