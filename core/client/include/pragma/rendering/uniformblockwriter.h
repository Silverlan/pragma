#ifndef __UNIFORMBLOCKWRITER_H__
#define __UNIFORMBLOCKWRITER_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>

// Used for std140-type uniform blocks
class DLLCLIENT UniformBlockWriter
{
protected:
	std::vector<unsigned int> m_offsets;
	unsigned int GetOffset();
public:
	UniformBlockWriter();
	unsigned int WriteFloat(float f);
	unsigned int WriteInt(int i);
	unsigned int WriteVector(const Vector3 &v);
	unsigned int WriteVector(const Vector4 &v);
	unsigned int WriteMatrix(const Mat4 &m);

	void WriteFloat(unsigned int location,float f);
	void WriteInt(unsigned int location,int i);
	void WriteVector(unsigned int location,const Vector3 &v);
	void WriteVector(unsigned int location,const Vector4 &v);
	void WriteMatrix(unsigned int location,const Mat4 &m);

	unsigned int GetOffset(unsigned int location);
};

#endif