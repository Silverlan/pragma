#ifndef F_SH_PARTICLE_MODEL_GLS
#define F_SH_PARTICLE_MODEL_GLS

#include "../../modules/sh_instance_flags.gls"

#define USE_INSTANCE_UNIFORM_BUFFER 1

#define PUSH_USER_CONSTANTS                                                                                                                                                                                                                                                                      \
	uint renderFlags;                                                                                                                                                                                                                                                                            \
	uint alphaMode;

#include "../../modules/sh_entity_info.gls"

//mat4 get_model_matrix() {return mat4(1.0);}
//bool is_weighted() {return false;}
//vec4 get_instance_color() {return vec4(1,1,1,1);}

#endif
