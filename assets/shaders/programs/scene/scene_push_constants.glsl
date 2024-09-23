#ifndef F_SH_INSTANCE_INFO_GLS
#define F_SH_INSTANCE_INFO_GLS

#include "/common/scene_draw_info.glsl"

#ifndef PUSH_USER_CONSTANTS
#define PUSH_USER_CONSTANTS
#endif

#define ENABLE_MATERIAL_PARAMETERS 1

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	SceneDrawInfo scene;

	uint debugMode;
	float reflectionProbeIntensity;
	vec2 padding;
	PUSH_USER_CONSTANTS
}
u_pushConstants;

bool is_light_map_enabled() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_LIGHTMAPS_ENABLED) != 0; }
bool is_directional_light_map_enabled() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_DIRECTIONAL_LIGHTMAPS_ENABLED) != 0; }
bool is_indirect_light_map_enabled() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_INDIRECT_LIGHTMAPS_ENABLED) != 0; }
bool is_ibl_enabled() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_NO_IBL) == 0; }
bool is_weighted_ext() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_USE_EXTENDED_VERTEX_WEIGHTS) != 0; }
bool is_3d_sky() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_RENDER_AS_3D_SKY) != 0; }
bool are_shadows_enabled() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_DISABLE_SHADOWS) == 0; }
float get_reflection_probe_intensity() { return u_pushConstants.reflectionProbeIntensity; }

#endif
