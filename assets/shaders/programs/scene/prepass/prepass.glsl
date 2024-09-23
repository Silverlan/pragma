#ifndef F_SH_PREPASS_GLS
#define F_SH_PREPASS_GLS

#define SPECIALIZATION_CONSTANT_ENABLE_ALPHA_TEST 0
#define SPECIALIZATION_CONSTANT_ENABLE_NORMAL_OUTPUT (SPECIALIZATION_CONSTANT_ENABLE_ALPHA_TEST + 1)
#define SPECIALIZATION_CONSTANT_ENABLE_ANIMATION (SPECIALIZATION_CONSTANT_ENABLE_NORMAL_OUTPUT + 1)
#define SPECIALIZATION_CONSTANT_ENABLE_MORPH_TARGET_ANIMATION (SPECIALIZATION_CONSTANT_ENABLE_ANIMATION + 1)
#define SPECIALIZATION_CONSTANT_ENABLE_EXTENDED_VERTEX_WEIGHTS (SPECIALIZATION_CONSTANT_ENABLE_MORPH_TARGET_ANIMATION + 1)

#include "/common/export.glsl"
#include "/common/scene_draw_info.glsl"
#include "/common/inputs/camera.glsl"
#include "/common/inputs/entity.glsl"
#include "/math/math.glsl"

#ifndef PUSH_USER_CONSTANTS
#define PUSH_USER_CONSTANTS
#endif

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	SceneDrawInfo scene;

	float alphaCutoff;
	PUSH_USER_CONSTANTS
}
u_pushConstants;

bool is_weighted_ext() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_USE_EXTENDED_VERTEX_WEIGHTS) != 0; }
bool is_3d_sky() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_RENDER_AS_3D_SKY) != 0; }
bool is_alpha_test_enabled() { return (u_pushConstants.scene.flags & SCENE_FLAG_BIT_ALPHA_TEST) != 0; }

#endif
