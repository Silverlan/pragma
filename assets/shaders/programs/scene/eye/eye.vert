#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/programs/scene/vs_world.glsl"
#include "/common/inputs/material.glsl"

vec3 world_to_tangent(vec3 vWorld, vec3 nWorld, vec3 tWorld, vec3 biWorld) { return vec3(dot(vWorld.xyz, tWorld.xyz), dot(vWorld.xyz, biWorld.xyz), dot(vWorld.xyz, nWorld.xyz)); }

vec3 world_to_tangent_normalized(vec3 vWorld, vec3 nWorld, vec3 tWorld, vec3 biWorld) { return normalize(world_to_tangent(vWorld, nWorld, tWorld, biWorld)); }

void main() { export_world_fragment_data(use_normal_map(), use_parallax_map()); }
