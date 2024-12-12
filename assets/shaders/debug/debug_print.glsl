#ifndef F_SH_DEBUG_PRINT_GLS
#define F_SH_DEBUG_PRINT_GLS

// These have to match the enums in pragma/rendering/global_render_settings_buffer_data.hpp
#define GLSL_TYPE_INT32 0
#define GLSL_TYPE_UINT32 1
#define GLSL_TYPE_FLOAT 2
#define GLSL_TYPE_BOOLEAN 3
#define GLSL_TYPE_VECTOR2 4
#define GLSL_TYPE_VECTOR3 5
#define GLSL_TYPE_VECTOR4 6
#define GLSL_TYPE_MAT4 7
#define GLSL_TYPE_VECTOR2I 8
#define GLSL_TYPE_VECTOR3I 9
#define GLSL_TYPE_VECTOR4I 10
#define GLSL_TYPE_NOT_SET 11
#define GLSL_TYPE_COUNT 12
#define GLSL_TYPE_LAST 11

layout(std140, LAYOUT_ID(RENDER_SETTINGS, DEBUG_PRINT)) buffer DebugPrintData {
	mat4 value;
	uint type;
} u_debugPrintData;

void print(int value) {
    u_debugPrintData.value[0][0] = value;
    u_debugPrintData.type = GLSL_TYPE_INT32;
}

void print(uint value) {
    u_debugPrintData.value[0][0] = value;
    u_debugPrintData.type = GLSL_TYPE_UINT32;
}

void print(float value) {
    u_debugPrintData.value[0][0] = value;
    u_debugPrintData.type = GLSL_TYPE_FLOAT;
}

void print(bool value) {
    u_debugPrintData.value[0][0] = value ? 1.0 : 0.0;
    u_debugPrintData.type = GLSL_TYPE_BOOLEAN;
}

void print(vec2 value) {
    u_debugPrintData.value[0] = vec4(value, 0.0, 0.0);
    u_debugPrintData.type = GLSL_TYPE_VECTOR2;
}

void print(vec3 value) {
    u_debugPrintData.value[0] = vec4(value, 0.0);
    u_debugPrintData.type = GLSL_TYPE_VECTOR3;
}

void print(vec4 value) {
    u_debugPrintData.value[0] = value;
    u_debugPrintData.type = GLSL_TYPE_VECTOR4;
}

void print(mat4 value) {
    u_debugPrintData.value = value;
    u_debugPrintData.type = GLSL_TYPE_MAT4;
}

void print(ivec2 value) {
    u_debugPrintData.value[0] = vec4(value, 0.0, 0.0);
    u_debugPrintData.type = GLSL_TYPE_VECTOR2I;
}

void print(ivec3 value) {
    u_debugPrintData.value[0] = vec4(value, 0.0);
    u_debugPrintData.type = GLSL_TYPE_VECTOR3I;
}

void print(ivec4 value) {
    u_debugPrintData.value[0] = vec4(value);
    u_debugPrintData.type = GLSL_TYPE_VECTOR4I;
}

#endif
