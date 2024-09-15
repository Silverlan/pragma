#ifndef F_SH_MATERIAL_GLS
#define F_SH_MATERIAL_GLS

#include "/common/alpha_mode.glsl"

bool use_normal_map() { return is_mat_flag_set(FMAT_FLAGS_HAS_NORMAL_MAP); }
bool use_parallax_map() { return is_mat_flag_set(FMAT_FLAGS_HAS_PARALLAX_MAP); }
bool use_glow_map() { return is_mat_flag_set(FMAT_FLAGS_HAS_EMISSION_MAP); }
bool use_wrinkle_maps() { return is_mat_flag_set(FMAT_FLAGS_HAS_WRINKLE_MAPS); }
bool use_rma_map() { return is_mat_flag_set(FMAT_FLAGS_HAS_RMA_MAP); }
bool is_material_translucent() { return is_mat_flag_set(FMAT_FLAGS_TRANSLUCENT); }
bool is_material_debug_flag_set() { return is_mat_flag_set(FMAT_FLAGS_DEBUG); }

#endif
