#ifndef F_FS_PARTICLE_DATA_GLS
#define F_FS_PARTICLE_DATA_GLS

// #include "../modules/sh_uniform_locations.gls"

//#ifndef SHADER_STORAGE_PARTICLE_SET
//	#define SHADER_STORAGE_PARTICLE_SET SHADER_UNIFORM_USER_SET1
//	#define SHADER_STORAGE_PARTICLE_BINDING 0
//#endif

// Storage buffer
struct ParticleData {
	vec4 position; // position.w = Particle size
	vec4 prevPos;  // prevPos.w = age
	uint color_rg; // red, green, each = 2 bytes
	uint color_ba; // blue, alpha, each = 2 bytes
	float rotation;
	uint lengthRotationYaw; // length, rotationYaw, each = 2 bytes
};

layout(std140, LAYOUT_ID(PARTICLES, DATA)) buffer FS_PARTICLE_DATA { ParticleData particles[]; }
u_particleData;

vec4 get_particle_blob_color(uint id)
{
	ParticleData pd = u_particleData.particles[id];
	uint twoByteMask = (1 << 16) - 1;
	uint r = pd.color_rg & twoByteMask;
	uint g = (pd.color_rg >> 16) & twoByteMask;
	uint b = pd.color_ba & twoByteMask;
	uint a = (pd.color_ba >> 16) & twoByteMask;
	return vec4(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

#endif
