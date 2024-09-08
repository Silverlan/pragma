#version 440

#include "shadow.glsl"
#include "/functions/fs_linearize_depth.glsl"

void main() { gl_FragDepth = length(fs_in.vert_pos_ws - fs_in.light_pos_ws.xyz) / fs_in.light_pos_ws.w; }
