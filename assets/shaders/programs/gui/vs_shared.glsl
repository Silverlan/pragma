#ifndef F_VS_WGUI_SHARED_GLS
#define F_VS_WGUI_SHARED_GLS

uint get_viewport_width() { return u_pushConstants.viewportSize >> 16; }

uint get_viewport_height() { return (u_pushConstants.viewportSize << 16) >> 16; }

vec4 get_vertex_position(vec2 vertPos)
{
	vec4 pos = u_pushConstants.modelMatrix * vec4(vertPos.x + 1.0, vertPos.y + 1.0, 0.0, 1.0);
	pos.x /= float(get_viewport_width());
	pos.y /= float(get_viewport_height());
	pos.x -= 1;
	pos.y -= 1;
	return pos;
}

vec4 get_vertex_position() { return get_vertex_position(in_vert_pos); }

#endif
