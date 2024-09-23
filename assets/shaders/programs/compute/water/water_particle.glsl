#ifndef F_CS_WATER_PARTICLE_GLS
#define F_CS_WATER_PARTICLE_GLS

struct WaterParticle {
	uvec4 neighbors;
	float height;
	float oldHeight;
	float targetHeight;
	float velocity;
};

layout(std140, LAYOUT_ID(WATER, PARTICLE_DATA)) buffer CS_PARTICLE_DATA { WaterParticle particles[]; }
u_particles;

#endif
