#ifndef F_SH_MATERIAL_FLAGS_GLS
#define F_SH_MATERIAL_FLAGS_GLS

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

bool use_normal_map(uint flags) { return ((flags & FMAT_NORMAL) != 0) ? true : false; }
bool use_parallax_map(uint flags) { return ((flags & FMAT_PARALLAX) != 0) ? true : false; }
bool use_glow_map(uint flags) { return ((flags & FMAT_GLOW_MODE) != 0) ? true : false; }
bool use_wrinkle_maps(uint flags) { return ((flags & FMAT_WRINKLE_MAPS) != 0) ? true : false; }
bool use_rma_map(uint flags) { return ((flags & FMAT_RMA_MAP) != 0) ? true : false; }
bool is_material_translucent(uint flags) { return ((flags & FMAT_TRANSLUCENT) != 0) ? true : false; }
bool is_diffuse_srgb(uint flags) { return ((flags & FMAT_DIFFUSE_SRGB) != 0) ? true : false; }
bool is_glow_srgb(uint flags) { return ((flags & FMAT_GLOW_SRGB) != 0) ? true : false; }
bool is_material_debug_flag_set(uint flags) { return ((flags & FMAT_DEBUG) != 0) ? true : false; }

#endif
