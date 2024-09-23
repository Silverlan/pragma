#ifndef F_SH_MATERIAL_FLAGS_GLS
#define F_SH_MATERIAL_FLAGS_GLS

bool use_normal_map(uint flags) { return ((flags & FMAT_FLAGS_HAS_NORMAL_MAP) != 0) ? true : false; }
bool use_parallax_map(uint flags) { return ((flags & FMAT_FLAGS_HAS_PARALLAX_MAP) != 0) ? true : false; }
bool use_glow_map(uint flags) { return ((flags & FMAT_FLAGS_HAS_EMISSION_MAP) != 0) ? true : false; }
bool use_wrinkle_maps(uint flags) { return ((flags & FMAT_FLAGS_HAS_WRINKLE_MAPS) != 0) ? true : false; }
bool use_rma_map(uint flags) { return ((flags & FMAT_FLAGS_HAS_RMA_MAP) != 0) ? true : false; }
bool is_material_translucent(uint flags) { return ((flags & FMAT_FLAGS_TRANSLUCENT) != 0) ? true : false; }
bool is_material_debug_flag_set(uint flags) { return ((flags & FMAT_FLAGS_DEBUG) != 0) ? true : false; }

#endif
