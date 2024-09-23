#ifndef F_FS_TEXTURE_GLS
#define F_FS_TEXTURE_GLS

float texture_cube(samplerCube depthTexture, int cubeSide, vec2 uv)
{
	float uvx = uv.x * 2.0;
	float uvy = uv.y * 2.0;
	float r;
	switch(cubeSide) {
	case 0:
		{
			float z = uvx - 1.0;
			float y = uvy - 1.0;
			r = texture(depthTexture, vec3(1.0, -y, z)).x;
			break;
		}
	case 1:
		{
			float z = 1.0 - uvx;
			float y = uvy - 1.0;
			r = texture(depthTexture, vec3(-1.0, -y, z)).x;
			break;
		}
	case 2:
		{
			float x = uvx - 1.0;
			float z = 1.0 - uvy;
			r = texture(depthTexture, vec3(x, 1.0, z)).x;
			break;
		}
	case 3:
		{
			float x = uvx - 1.0;
			float z = uvy - 1.0;
			r = texture(depthTexture, vec3(x, -1.0, z)).x;
			break;
		}
	case 4:
		{
			float x = uvx - 1.0;
			float y = uvy - 1.0;
			r = texture(depthTexture, vec3(x, -y, -1.0)).x;
			break;
		}
	case 5:
		{
			float x = 1.0 - (uv.x / 4.0) * 8.0;
			float y = (uv.y / 3.0) * 6.0 - 1.0;
			r = texture(depthTexture, vec3(x, -y, 1.0)).x;
			break;
		}
	}
	return r;
}

#endif
