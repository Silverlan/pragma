#ifndef F_FS_PARTICLE_TEXTURE_GLS
#define F_FS_PARTICLE_TEXTURE_GLS

#ifdef GLS_FRAGMENT_SHADER
#include "particle_render_flags.glsl"
#include "particle_vertex_data.glsl"
#ifndef ANIMATION_RENDER_FLAGS
#define ANIMATION_RENDER_FLAGS u_instance.renderFlags
#endif
bool is_animated() { return is_animated(ANIMATION_RENDER_FLAGS); }
bool is_particle_unlit() { return is_particle_unlit(ANIMATION_RENDER_FLAGS); }
#if 0
	vec2 get_particle_uv(vec2 uv,float tStart)
	{
		if(is_animated() == false)
			return uv;
		//return get_animated_texture_uv(uv,tStart);
		return get_animation_frame_uv_coordinates(uv);
	}
	vec2 get_particle_uv(vec2 uv,float curTime,float tStart)
	{
		if(is_animated() == false)
			return uv;
		return get_animation_frame_uv_coordinates(uv);//get_animated_texture_uv(uv,curTime,tStart);
	}
#endif
float get_particle_alpha(vec4 col) { return col.a; }

#ifndef ENABLE_PARTICLE_TEXTURE
#define ENABLE_PARTICLE_TEXTURE 1
#endif
#if ENABLE_PARTICLE_TEXTURE == 1
#include "/common/inputs/fs_animated_texture.glsl"
layout(LAYOUT_ID(TEXTURE,TEXTURE)) uniform sampler2D u_smpParticle;

vec4 get_animated_texture_color(vec2 uv)
{
	if(is_animated() == false)
		return texture(u_smpParticle, uv);
	uint frameIndices = fs_in.animationFrameIndices;
	uint frameIndex1 = frameIndices >> 16;
	uint frameIndex0 = (frameIndices << 16) >> 16;
	float interpFactor = fs_in.animationFrameInterpFactor;

	TextureAnimationFrame frame0 = u_animation.frames[frameIndex0];
	vec2 uv0 = frame0.uvStart + (frame0.uvEnd - frame0.uvStart) * uv;
	vec4 col0 = texture(u_smpParticle, uv0);

	TextureAnimationFrame frame1 = u_animation.frames[frameIndex1];
	vec2 uv1 = frame1.uvStart + (frame1.uvEnd - frame1.uvStart) * uv;
	vec4 col1 = texture(u_smpParticle, uv1);

	return mix(col0, col1, interpFactor);
}
#if 0
		vec4 get_particle_color(float tStart)
		{
			vec2 uv = fs_in.vert_uv;
			if(is_animated() == false)
				return texture(u_smpParticle,get_particle_uv(uv,tStart));
			float remainder;
			uint frameIndex;
			vec2 uv0 = get_animation_frame_uv_coordinates(uv);//get_animated_texture_uv(uv,cur_time(),tStart,remainder,frameIndex);
			vec2 uv1 = get_animation_frame_uv_coordinates(uv);//get_animation_frame_uv_coordinates(uv,frameIndex +1);
			vec4 col0 = texture(u_smpParticle,uv0);
			vec4 col1 = texture(u_smpParticle,uv1);
			vec4 col = mix(col0,col1,remainder);
			//if(remainder > 0.5)
				return vec4(remainder,0,0,1);
			return col;
		}
		vec4 get_particle_color(vec2 uv) {return texture(u_smpParticle,uv);}
		vec2 get_particle_uv() {return vec2(0,0);}//get_particle_uv(fs_in.vert_uv,fs_in.particle_start);}
#endif
#endif
#endif

#endif
