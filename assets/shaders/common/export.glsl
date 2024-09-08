#ifndef F_SH_EXPORT_GLS
#define F_SH_EXPORT_GLS

#ifdef GLS_VERTEX_SHADER
#define EXPORT_VS out
#else
#define EXPORT_VS in
#endif

#ifdef GLS_GEOMETRY_SHADER
#define EXPORT_GS out
#else
#define EXPORT_GS in
#endif

#endif
