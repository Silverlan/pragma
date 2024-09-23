#version 440

#include "shadow.glsl"
#include "/functions/fs_linearize_depth.glsl"

float get_linearized_depth(float depth)
{
	float n = 1.0;
	float f = fs_in.light_pos_ws.w; //u_pushConstants.lightPos.w;
	float z = depth;
	return (2.0 * n) / (f + n - z * (f - n));
}

void main()
{
	gl_FragDepth = get_linearized_depth(gl_FragCoord.z);
	//gl_FragDepth = length(fs_in.vert_pos_ws -fs_in.light_pos_ws.xyz) /fs_in.light_pos_ws.w;
	//gl_FragDepth = get_depth_distance(gl_FragCoord.z,1.0,u_pushConstants.lightPos.w);
}
