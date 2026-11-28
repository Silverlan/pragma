#ifndef F_SH_WGUI_BASE_PUSH_CONSTANTS_GLS
#define F_SH_WGUI_BASE_PUSH_CONSTANTS_GLS

#define GUI_BASE_PUSH_CONSTANTS \
	mat3x4 modelMatrix;         \
	vec4 _padding;              \
	vec4 color;                 \
	uint viewportSize;

#endif
