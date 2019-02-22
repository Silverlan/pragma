#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include <fsys/filesystem.h>
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/scene/camera.h"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

bool WriteTGA(const char *name,int w,int h,unsigned char *pixels,int size)
{
	auto f = FileManager::OpenFile<VFilePtrReal>(name,"wb");
	if(f == NULL)
		return false;
	f->Write<char>((char)(0)); // Length of image ID field
	f->Write<char>((char)(0)); // No color map included
	f->Write<char>(2); // Uncompressed true-color image
	
	// Color map
	f->Write<short>((short)(0));
	f->Write<short>((short)(0));
	f->Write<char>((char)(0));

	f->Write<short>((short)(0)); // x-origin (origin = lower-left corner)
	f->Write<short>((short)(0)); // y-origin
	
	f->Write<char>(w %256);
	f->Write<char>(CInt8(w /256));

	f->Write<char>(h %256);
	f->Write<char>(CInt8(h /256));

	f->Write<char>(24); // Bits per pixel
	f->Write<char>((char)(0)); // Image descriptor

	f->Write(pixels,size);
	return true;
}

bool ClientState::SaveFrameBufferAsTGA(const char *name,int x,int y,int w,int h)
{
#if 0
	int format = OpenGL::GetTexParameter(GL_TEXTURE_INTERNAL_FORMAT);
	return SaveFrameBufferAsTGA(name,x,y,w,h,format);
#endif
	return false;
}

static void SwapRedBlue(unsigned char *pixels,unsigned int w,unsigned int h)
{
	unsigned int size = w *h *3;
	unsigned char tmp;
	for(unsigned int i=0;i<size;i+=3)
	{
		tmp = pixels[i];
		pixels[i] = pixels[i +2];
		pixels[i +2] = tmp;
	}
}

bool ClientState::SaveFrameBufferAsTGA(const char *name,int,int,int w,int h,unsigned int format)
{
	/*auto &scene = c_game->GetScene();
	auto &cam = scene->camera;
	float n = cam->GetZNear();
	float f = cam->GetZFar();

	unsigned int size = w *h *3;
	unsigned char *pixels = new unsigned char[size];
	switch(format)
	{
	case GL_RGB:
		{
			glReadPixels(0,0,w,h,format,GL_UNSIGNED_BYTE,&pixels[0]);
			//glGetTexImage(GL_TEXTURE_2D,0,format,GL_UNSIGNED_BYTE,&pixels[0]);
			SwapRedBlue(pixels,w,h);
			break;
		}
	case GL_DEPTH_COMPONENT:
		{
			float *depths = new float[w *h];
			glReadPixels(0,0,w,h,format,GL_FLOAT,&depths[0]);
			for(int i=0;i<(w *h);i++)
			{
				float v = depths[i];
				v = (2.f *n) /(f +n -v *(f -n));
				v *= 255.f;
				pixels[i *3] = CUInt8(v);
				pixels[i *3 +1] = CUInt8(v);
				pixels[i *3 +2] = CUInt8(v);
			}
			delete[] depths;
			break;
		}
	default:
		{
			delete[] pixels;
			return false;
		}
	}
	bool b = WriteTGA(name,w,h,pixels,size);
	delete[] pixels;
	return b;*/ // prosper TODO
	return false; // prosper TODO
}

bool ClientState::SaveTextureAsTGA(const char *name) {return SaveTextureAsTGA(name,GL_TEXTURE_2D,0);}
bool ClientState::SaveTextureAsTGA(const char *name,unsigned int target,int level)
{
#if 0
	unsigned int tex = OpenGL::GetInt(GL_TEXTURE_BINDING_2D);
	if(!glIsTexture(tex))
		return false;
	int err = OpenGL::GetError();
	int format;
	glGetTexLevelParameteriv(target,level,GL_TEXTURE_INTERNAL_FORMAT,&format);
	int w;
	int h;
	glGetTexLevelParameteriv(target,level,GL_TEXTURE_WIDTH,&w);
	glGetTexLevelParameteriv(target,level,GL_TEXTURE_HEIGHT,&h);
	if(w <= 0 || h <= 0)
		return false;
	err = OpenGL::GetError();
	if(err != 0)
		return false;
	unsigned int size = w *h *3;
	unsigned int sizeIn = w *h;
	if(format == GL_RGBA)
		sizeIn *= 4;
	else if(format != GL_ALPHA && format != GL_DEPTH_COMPONENT)
	{
		format = GL_RGB;
		sizeIn *= 3;
	}
	else if(target == GL_TEXTURE_CUBE_MAP)
	{
		size *= 12;
		w *= 4;
		h *= 3;
	}
	unsigned char *pixelsIn = new unsigned char[sizeIn];
	unsigned char *pixels = new unsigned char[size];
	OpenGL::SetPixelStorageMode(GL_PACK_ALIGNMENT,1);
	if(format == GL_RGBA)
	{
		glGetTexImage(target,level,format,GL_UNSIGNED_BYTE,&pixelsIn[0]);
		unsigned long long px = 0;
		for(unsigned int i=0;i<sizeIn;i+=4)
		{
			pixels[px] = pixelsIn[i];
			pixels[px +1] = pixelsIn[i +1];
			pixels[px +2] = pixelsIn[i +2];
			px += 3;
		}
	}
	else if(format == GL_ALPHA || format == GL_DEPTH_COMPONENT)
	{
		if(target == GL_TEXTURE_CUBE_MAP)
		{
			int wSrc = w /4;
			int hSrc = h /3;
			for(int i=0;i<6;i++)
			{
				for(unsigned int j=0;j<sizeIn;j++)
					pixelsIn[j] = UInt16(32);
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i,level,format,GL_UNSIGNED_BYTE,&pixelsIn[0]);
				unsigned long long offset;
				if(i == 0)
					offset = w *hSrc +wSrc *2;
				else if(i == 1)
					offset = w *hSrc;
				else if(i == 2)
					offset = wSrc;
				else if(i == 3)
					offset = w *hSrc *2 +wSrc;
				else if(i == 4)
					offset = w *hSrc +wSrc;
				else
					offset = w *hSrc +wSrc *3;
				offset *= 3;
				for(unsigned long long i=0;i<sizeIn;i++)
				{
					unsigned long long px = offset +i *3;
					px += (i /wSrc) *(w -wSrc) *3;
					pixels[px] = pixelsIn[i];
					pixels[px +1] = pixelsIn[i];
					pixels[px +2] = pixelsIn[i];
				}
			}
		}
		else
		{
			glGetTexImage(target,level,format,GL_UNSIGNED_BYTE,&pixelsIn[0]);
			for(unsigned int i=0;i<sizeIn;i++)
			{
				unsigned long long px = i *3;
				pixels[px] = pixelsIn[i];
				pixels[px +1] = pixelsIn[i];
				pixels[px +2] = pixelsIn[i];
			}
		}
	}
	else
	{
		glGetTexImage(target,level,format,GL_UNSIGNED_BYTE,&pixelsIn[0]);
		memcpy(&pixels[0],&pixelsIn[0],size);
	}
	SwapRedBlue(pixels,w,h);
	bool b = WriteTGA(name,w,h,pixels,size);
	delete[] pixelsIn;
	delete[] pixels;
	return b;
#endif
	return false;
}
bool ClientState::SavePixelsAsTGA(const char *name,unsigned char *pixels,unsigned int w,unsigned int h)
{
#if 0
	unsigned char *out = new unsigned char[w *h *3];
	memcpy(&out[0],&pixels[0],w *h *3);
	SwapRedBlue(out,w,h);
	bool b = WriteTGA(name,w,h,out,w *h *3);
	delete[] out;
	return b;
#endif
	return false;
}