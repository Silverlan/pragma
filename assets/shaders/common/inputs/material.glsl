#ifndef F_SH_MATERIAL_GLS
#define F_SH_MATERIAL_GLS

#include "/common/alpha_mode.glsl"

#define FMAT_DIFFUSE 0 // Diffuse is always enabled
#define FMAT_NORMAL 1
#define FMAT_PARALLAX (FMAT_NORMAL << 1)
#define FMAT_GLOW (FMAT_PARALLAX << 1)
#define FMAT_TRANSLUCENT (FMAT_GLOW << 1)
#define FMAT_BLACK_TO_ALPHA (FMAT_TRANSLUCENT << 1)

#define FMAT_GLOW_MODE_1 (FMAT_BLACK_TO_ALPHA << 1)
#define FMAT_GLOW_MODE_2 (FMAT_GLOW_MODE_1 << 1)
#define FMAT_GLOW_MODE_3 (FMAT_GLOW_MODE_2 << 1)
#define FMAT_GLOW_MODE_4 (FMAT_GLOW_MODE_3 << 1)
#define FMAT_GLOW_MODE (FMAT_GLOW_MODE_1 | FMAT_GLOW_MODE_2 | FMAT_GLOW_MODE_3 | FMAT_GLOW_MODE_4)

#define FMAT_DIFFUSE_SRGB (FMAT_GLOW_MODE_4 << 1)
#define FMAT_GLOW_SRGB (FMAT_DIFFUSE_SRGB << 1)
#define FMAT_DEBUG (FMAT_GLOW_SRGB << 1)

#define FMAT_WRINKLE_MAPS (FMAT_DEBUG << 1)
#define FMAT_RMA_MAP (FMAT_WRINKLE_MAPS << 1)

struct MaterialData {
	vec4 color;
	vec4 emissionFactor;
	uint flags;
	float glowScale;
	uint parallax; // (parallaxSteps << 16) | parallaxHeightScale;
	float alphaDiscardThreshold;
	float phongIntensity;
	float metalnessFactor;
	float roughnessFactor;
	float aoFactor;
	uint alphaMode;
	float alphaCutoff;
};

layout(std140, LAYOUT_ID(MATERIAL, SETTINGS)) uniform Material { MaterialData material; }
u_material;

bool use_normal_map() { return ((u_material.material.flags & FMAT_NORMAL) != 0) ? true : false; }
bool use_parallax_map() { return ((u_material.material.flags & FMAT_PARALLAX) != 0) ? true : false; }
bool use_glow_map() { return ((u_material.material.flags & FMAT_GLOW_MODE) != 0) ? true : false; }
bool use_wrinkle_maps() { return ((u_material.material.flags & FMAT_WRINKLE_MAPS) != 0) ? true : false; }
bool use_rma_map() { return ((u_material.material.flags & FMAT_RMA_MAP) != 0) ? true : false; }
float get_glow_scale() { return u_material.material.glowScale; }
bool is_material_translucent() { return ((u_material.material.flags & FMAT_TRANSLUCENT) != 0) ? true : false; }
bool is_diffuse_srgb() { return ((u_material.material.flags & FMAT_DIFFUSE_SRGB) != 0) ? true : false; }
bool is_glow_srgb() { return ((u_material.material.flags & FMAT_GLOW_SRGB) != 0) ? true : false; }
bool is_material_debug_flag_set() { return ((u_material.material.flags & FMAT_DEBUG) != 0) ? true : false; }

float get_parallax_height_scale()
{
	uint parallaxHeightScaleBits = u_material.material.parallax & 0xFFFF;
	return unpackHalf2x16(parallaxHeightScaleBits).x;
}
float get_parallax_steps() { return (u_material.material.parallax >> 16) & 0xFFFF; }

#endif