#ifndef F_SH_EXPORT_GLS
#define F_SH_EXPORT_GLS

#ifdef GLS_VERTEX_SHADER
#define EXPORT_VS out
#define EXPORT_VS_PREFIX vs_out_
#else
#define EXPORT_VS in
#define EXPORT_VS_PREFIX fs_in_
#endif

#ifdef GLS_GEOMETRY_SHADER
#define EXPORT_GS out
#else
#define EXPORT_GS in
#endif

#define CONCAT(a, b) a ## b

#endif
