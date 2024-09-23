#ifndef F_FS_SCENE_GLS
#define F_FS_SCENE_GLS

layout(location = 0) out vec4 fs_color;
layout(location = 1) out vec4 fs_brightColor; // Render bright colors into additional color attachment; Used for bloom effects

void extract_bright_color(vec4 col, float threshold)
{
	float brightness = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > threshold) {
		fs_brightColor = vec4(col.rgb, 1.0);
		float brightenRatio = 1.0 / max(max(fs_brightColor.r, fs_brightColor.g), fs_brightColor.b);
		fs_brightColor.r *= brightenRatio;
		fs_brightColor.g *= brightenRatio;
		fs_brightColor.b *= brightenRatio;
		fs_brightColor.a = 1.0;
	}
	else
		fs_brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
void extract_bright_color(vec4 col) { extract_bright_color(col, 1.0); }

#endif
