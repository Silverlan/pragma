#ifndef F_SH_WGUI_ELEMENT_BUFFER_GLS
#define F_SH_WGUI_ELEMENT_BUFFER_GLS

layout(std140, LAYOUT_ID(0, 0)) uniform shader
{
	vec4 color;
	mat4 MVP;
}
u_shader;

#endif
