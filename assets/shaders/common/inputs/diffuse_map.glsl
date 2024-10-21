#ifndef F_FS_DIFFUSEMAP_GLS
#define F_FS_DIFFUSEMAP_GLS

#include "sh_uniform_locations.gls"
#include "sh_instance_info.gls"

#ifndef USE_DIFFUSE_MAP
#define USE_DIFFUSE_MAP 1
#endif
#if USE_DIFFUSE_MAP == 1
layout(LAYOUT_ID(MATERIAL, ALBEDO_MAP)) uniform sampler2D u_diffuseMap;
vec4 get_diffuse_map_color(vec2 texCoords) { return texture(u_diffuseMap, texCoords).rgba; }
vec4 get_diffuse_map_color() { return get_diffuse_map_color(get_vertex_uv()); }
vec4 get_diffuse_map_color(sampler2D diffMap) { return texture(diffMap, get_vertex_uv()).rgba; }
#endif

vec4 get_color_modifier() { return get_instance_color(); }
/*layout(std140,LAYOUT_ID(SHADER_UNIFORM_DIFFUSE_SET,0)) uniform Diffuse
{
	vec4 color;
} u_diffuse;
vec4 get_color_modifier() {return u_diffuse.color;}
*/ // Moved to push constants

#endif
