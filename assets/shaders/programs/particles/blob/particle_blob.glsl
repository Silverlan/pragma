#ifndef F_SH_PARTICLE_BLOB_GLS
#define F_SH_PARTICLE_BLOB_GLS

#define VERTEX_USER_DATA flat uvec4 blobNeighbors;

#define SHADER_STORAGE_PARTICLE_SET 4
#define SHADER_STORAGE_PARTICLE_BINDING 0

#define SHADER_UNIFORM_CAMERA_SET 0
#define SHADER_UNIFORM_CAMERA_BINDING 0

#define ENABLE_PARTICLE_TEXTURE 0
#define SHADER_VERTEX_USER_DATA_LOCATION_COUNT 1

#include "../particle_mode.glsl"
#include "../particle_generic.glsl"

#endif
