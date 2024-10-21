/*
DoF with bokeh GLSL shader v2.4
by Martins Upitis (martinsh) (devlog-martinsh.blogspot.com)

----------------------
The shader is Blender Game Engine ready, but it should be quite simple to adapt for your engine.

This work is licensed under a Creative Commons Attribution 3.0 Unported License.
So you are free to share, modify and adapt it for your needs, and even use it for commercial use.
I would also love to hear about a project you are using it.

Have fun,
Martins
----------------------

changelog:
    
2.4:
- physically accurate DoF simulation calculated from "focalDepth" ,"focalLength", "f-stop" and "CoC" parameters. 
- option for artist controlled DoF simulation calculated only from "focalDepth" and individual controls for near and far blur 
- added "circe of confusion" (CoC) parameter in mm to accurately simulate DoF with different camera sensor or film sizes
- cleaned up the code
- some optimization

2.3:
- new and physically little more accurate DoF
- two extra input variables - focal length and aperture iris diameter
- added a debug visualization of focus point and focal range

2.1:
- added an option for pentagonal bokeh shape
- minor fixes

2.0:
- variable sample count to increase quality/performance
- option to blur depth buffer to reduce hard edges
- option to dither the samples with noise or pattern
- bokeh chromatic aberration/fringing
- bokeh bias to bring out bokeh edges
- image thresholding to bring out highlights when image is out of focus

*/

#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// Source: https://github.com/NatronGitHub/openfx-misc/blob/master/Shadertoy/presets/default/DoF_bokeh_2.4.frag.glsl
// http://artmartinsh.blogspot.com/2010/02/glsl-lens-blur-filter-with-bokeh.html

#include "/math/math.glsl"
#include "/functions/fs_linearize_depth.glsl"

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D bgl_RenderedTexture;
layout(LAYOUT_ID(DEPTH_BUFFER, MAP)) uniform sampler2D bgl_DepthTexture;

layout(location = 0) in vec2 vs_vert_uv;

#define FDOF_NONE 0
//show debug focus point and focal range (red = focal point, green = focal range)
#define FDOF_DEBUG_SHOW_FOCUS 1
#define FDOF_ENABLE_VIGNETTE (FDOF_DEBUG_SHOW_FOCUS<<1)
#define FDOF_PENTAGON_BOKEH_SHAPE (FDOF_ENABLE_VIGNETTE<<1)
#define FDOF_DEBUG_SHOW_DEPTH (FDOF_PENTAGON_BOKEH_SHAPE<<1)

layout(LAYOUT_PUSH_CONSTANTS()) uniform RenderSettings {
	mat4 MVP;
	uint width;
	uint height;

	float focalDepth;  //focal distance value in meters, but you may use autofocus option below
	float focalLength; //focal length in mm
	float fstop; //f-stop value

	float zNear;
	float zFar;

	uint flags;
	int rings;
	int ringSamples; //samples on the first ring
	float CoC; //circle of confusion size in mm (35mm film = 0.03mm)
	float maxBlur; //clamp value of max blur (0.0 = no blur,1.0 default)
	float dither; //dither amount
	float vignIn; //vignetting inner border
	float vignOut; //vignetting outer border
	float pentagonShapeFeather; //pentagon shape feather
} u_renderSettings;

//uniform variables from external script

/* 
make sure that these two values are the same for your camera, otherwise distances will be wrong.
*/

//------------------------------------------
//user variables

bool manualdof = false; //manual dof calculation
float ndofstart = 1.0; //near dof blur start
float ndofdist = 2.0; //near dof blur falloff distance
float fdofstart = 1.0; //far dof blur start
float fdofdist = 3.0; //far dof blur falloff distance

float vignfade = 22.0; //f-stops till vignete fades

bool autofocus = false; //use autofocus in shader? disable if you use external focalDepth value
vec2 focus = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)

float threshold = 0.5; //highlight threshold;
float gain = 2.0; //highlight gain;

float bias = 0.5; //bokeh edge bias
float fringe = 0.7; //bokeh chromatic aberration/fringing

bool noise = true; //use noise instead of pattern for sample dithering

bool depthblur = false; //blur the depth buffer?
float dbsize = 1.25; //depthblursize

/*
next part is experimental
not looking good with small sample and ring count
looks okay starting from samples = 4, rings = 4
*/

//------------------------------------------


float penta(vec2 coords) //pentagonal shape
{
	float scale = float(u_renderSettings.rings) - 1.3;
	vec4  HS0 = vec4( 1.0,         0.0,         0.0,  1.0);
	vec4  HS1 = vec4( 0.309016994, 0.951056516, 0.0,  1.0);
	vec4  HS2 = vec4(-0.809016994, 0.587785252, 0.0,  1.0);
	vec4  HS3 = vec4(-0.809016994,-0.587785252, 0.0,  1.0);
	vec4  HS4 = vec4( 0.309016994,-0.951056516, 0.0,  1.0);
	vec4  HS5 = vec4( 0.0        ,0.0         , 1.0,  1.0);
	
	vec4  one = vec4( 1.0 );
	
	vec4 P = vec4((coords),vec2(scale, scale)); 
	
	vec4 dist = vec4(0.0);
	float inorout = -4.0;
	
	dist.x = dot( P, HS0 );
	dist.y = dot( P, HS1 );
	dist.z = dot( P, HS2 );
	dist.w = dot( P, HS3 );
	
	dist = smoothstep( -u_renderSettings.pentagonShapeFeather, u_renderSettings.pentagonShapeFeather, dist );
	
	inorout += dot( dist, one );
	
	dist.x = dot( P, HS4 );
	dist.y = HS5.w - abs( P.z );
	
	dist = smoothstep( -u_renderSettings.pentagonShapeFeather, u_renderSettings.pentagonShapeFeather, dist );
	inorout += dist.x;
	
	return clamp( inorout, 0.0, 1.0 );
}

vec2 get_texel()
{
	return vec2(1.0/float(u_renderSettings.width),1.0/float(u_renderSettings.height));
}

float bdepth(vec2 coords) //blurring depth
{
	float d = 0.0;
	float kernel[9];
	vec2 offset[9];
	
	vec2 wh = get_texel() * dbsize;
	
	offset[0] = vec2(-wh.x,-wh.y);
	offset[1] = vec2( 0.0, -wh.y);
	offset[2] = vec2( wh.x -wh.y);
	
	offset[3] = vec2(-wh.x,  0.0);
	offset[4] = vec2( 0.0,   0.0);
	offset[5] = vec2( wh.x,  0.0);
	
	offset[6] = vec2(-wh.x, wh.y);
	offset[7] = vec2( 0.0,  wh.y);
	offset[8] = vec2( wh.x, wh.y);
	
	kernel[0] = 1.0/16.0;   kernel[1] = 2.0/16.0;   kernel[2] = 1.0/16.0;
	kernel[3] = 2.0/16.0;   kernel[4] = 4.0/16.0;   kernel[5] = 2.0/16.0;
	kernel[6] = 1.0/16.0;   kernel[7] = 2.0/16.0;   kernel[8] = 1.0/16.0;
	
	
	for( int i=0; i<9; i++ )
	{
		float tmp = texture(bgl_DepthTexture, coords + offset[i]).r;
		d += tmp * kernel[i];
	}
	
	return d;
}


vec3 color(vec2 coords,float blur) //processing the sample
{
	vec3 col = vec3(0.0);
	
	vec2 texel = get_texel();
	col.r = texture(bgl_RenderedTexture,coords + vec2(0.0,1.0)*texel*fringe*blur).r;
	col.g = texture(bgl_RenderedTexture,coords + vec2(-0.866,-0.5)*texel*fringe*blur).g;
	col.b = texture(bgl_RenderedTexture,coords + vec2(0.866,-0.5)*texel*fringe*blur).b;
	
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-threshold)*gain, 0.0);
	return col+mix(vec3(0.0),col,thresh*blur);
}

vec2 rand(vec2 coord) //generating noise/pattern texture for dithering
{
	float noiseX = ((fract(1.0-coord.s*(float(u_renderSettings.width)/2.0))*0.25)+(fract(coord.t*(float(u_renderSettings.height)/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(float(u_renderSettings.width)/2.0))*0.75)+(fract(coord.t*(float(u_renderSettings.height)/2.0))*0.25))*2.0-1.0;
	
	if (noise)
	{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	}
	return vec2(noiseX,noiseY);
}

vec3 debugFocus(vec3 col, float blur, float depth)
{
	float edge = 0.002*depth; //distance based edge smoothing
	float m = clamp(smoothstep(0.0,edge,blur),0.0,1.0);
	float e = clamp(smoothstep(1.0-edge,1.0,blur),0.0,1.0);
	
	col = mix(col,vec3(1.0,0.5,0.0),(1.0-m)*0.6);
	col = mix(col,vec3(0.0,0.5,1.0),((1.0-e)-(1.0-m))*0.2);

	return col;
}

float linearize(float depth)
{
	return get_depth_distance(depth,u_renderSettings.zNear,u_renderSettings.zFar);
}

float vignette()
{
	float dist = distance(vs_vert_uv, vec2(0.5,0.5));
	dist = smoothstep(u_renderSettings.vignOut+(u_renderSettings.fstop/vignfade), u_renderSettings.vignIn+(u_renderSettings.fstop/vignfade), dist);
	return clamp(dist,0.0,1.0);
}

layout(location = 0) out vec4 fs_color;

void main() 
{
	float width = u_renderSettings.width; //texture width
	float height = u_renderSettings.height; //texture height

	//scene depth calculation
	
	float depth = linearize(texture(bgl_DepthTexture,vs_vert_uv).x);
	
	if (depthblur)
		depth = linearize(bdepth(vs_vert_uv));

	depth = units_to_meters(depth);
	
	//focal plane calculation
	
	float fDepth = units_to_meters(u_renderSettings.focalDepth);
	
	if (autofocus)
		fDepth = linearize(texture(bgl_DepthTexture,focus).x);
	
	//dof blur factor calculation
	
	float blur = 0.0;
	
	if (manualdof)
	{    
		float a = depth-fDepth; //focal plane
		float b = (a-fdofstart)/fdofdist; //far DoF
		float c = (-a-ndofstart)/ndofdist; //near Dof
		blur = (a>0.0)?b:c;
	}
	
	else
	{
		float f = u_renderSettings.focalLength; //focal length in mm
		float d = fDepth*1000.0; //focal plane in mm
		float o = depth*1000.0; //depth in mm
		
		float a = (o*f)/(o-f); 
		float b = (d*f)/(d-f); 
		float c = (d-f)/(d*u_renderSettings.fstop*u_renderSettings.CoC); 
		
		blur = abs(a-b)*c;
	}
	
	blur = clamp(blur,0.0,1.0);
	
	// calculation of pattern for ditering
	
	vec2 noise = rand(vs_vert_uv)*u_renderSettings.dither*blur;
	
	// getting blur x and y step factor
	
	// The original code was using fw = width and fh = height, however that doesn't seem to be correct and would cause
	// the intensity of the effect to drastically change depending on the resolution. The reference width of 1920 below is
	// subjectively chosen and creates a consistent effect independent of the resolution.
	float fw = 1920;
	float fh = fw *(height /width);
	float w = (1.0/fw)*blur*u_renderSettings.maxBlur+noise.x;
	float h = (1.0/fh)*blur*u_renderSettings.maxBlur+noise.y;
	
	// calculation of final color
	
	vec3 col = vec3(0.0);
	
	if(blur < 0.05) //some optimization thingy
		col = texture(bgl_RenderedTexture, vs_vert_uv).rgb;
	
	else
	{
		col = texture(bgl_RenderedTexture, vs_vert_uv).rgb;
		float s = 1.0;
		int ringsamples;
		
		for (int i = 1; i <= u_renderSettings.rings; i += 1)
		{   
			ringsamples = i * u_renderSettings.ringSamples;
			
			for (int j = 0 ; j < ringsamples ; j += 1)   
			{
				float step = M_PI*2.0 / float(ringsamples);
				float pw = (cos(float(j)*step)*float(i));
				float ph = (sin(float(j)*step)*float(i));
				float p = 1.0;
				if ((u_renderSettings.flags &FDOF_PENTAGON_BOKEH_SHAPE) != 0)
					p = penta(vec2(pw,ph));
				col += color(vs_vert_uv + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(u_renderSettings.rings)),bias)*p;  
				s += 1.0*mix(1.0,(float(i))/(float(u_renderSettings.rings)),bias)*p;   
			}
		}
		col /= s; //divide by sample count
	}
	
	if ((u_renderSettings.flags &FDOF_DEBUG_SHOW_FOCUS) != 0)
		col = debugFocus(col, blur, depth);
	
	if ((u_renderSettings.flags &FDOF_ENABLE_VIGNETTE) != 0)
		col *= vignette();
	
	fs_color.rgb = col;
	fs_color.a = 1.0;

	//fs_color = texture(bgl_RenderedTexture, vs_vert_uv);

	if ((u_renderSettings.flags &FDOF_DEBUG_SHOW_DEPTH) != 0)
	{
		float d = get_linearized_depth(texture(bgl_DepthTexture,vs_vert_uv).x,u_renderSettings.zNear,u_renderSettings.zFar);
		fs_color.rgb = vec3(d,d,d);
	}

	//if(abs(fDepth -depth) < 0.1)
	//	fs_color = vec4(1,0,0,1);

	//float d = linearize(texture(bgl_DepthTexture,vs_vert_uv).x);
	//if(d < 400 && d > 200)
	//	fs_color = vec4(1,0,0,1);
}
