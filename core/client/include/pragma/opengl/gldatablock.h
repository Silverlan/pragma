#ifndef __GLDATABLOCK_H__
#define __GLDATABLOCK_H__

#include "pragma/clientdefinitions.h"

class DLLCLIENT GLDataBlock
{
protected:
	unsigned int m_type;
	unsigned int m_size;
	void Initialize();
	void Initialize(void **data,unsigned int size);
	template<class T>
		void SetValue(unsigned int idx,T val);
	template<class T>
		void Clear(T t);
public:
	GLDataBlock(unsigned int type,unsigned int size);
	~GLDataBlock();
	void *data;
	void Release();
	void Clear(float f);
	void ClearInt(int i);
	void ClearUInt(unsigned int i);
	void ClearChar(char c);
	void ClearUChar(unsigned char c);
	void ClearFloat(float f);
	unsigned int GetType();
	unsigned int GetSize();
	void SetValue(unsigned int idx,float f);
	void SetInt(unsigned int idx,int i);
	void SetUInt(unsigned int idx,unsigned int i);
	void SetChar(unsigned int idx,char c);
	void SetUChar(unsigned int idx,unsigned char c);
	void SetFloat(unsigned int idx,float f);
	void Resize(unsigned int size);
	float GetValue(unsigned int idx);
	int GetInt(unsigned int idx);
	unsigned int GetUInt(unsigned int idx);
	char GetChar(unsigned int idx);
	unsigned char GetUChar(unsigned int idx);
	float GetFloat(unsigned int idx);
	char GetElementSize();
	unsigned long long GetByteSize();
};

template<class T>
	void GLDataBlock::SetValue(unsigned int idx,T val)
{
	if(data == nullptr)
		return;
	static_cast<T*>(data)[idx] = val;
}

template<class T>
	void GLDataBlock::Clear(T t)
{
	if(data == nullptr)
		return;
	T *tData = static_cast<T*>(data);
	for(unsigned int idx=0;idx<m_size;idx++)
		tData[idx] = t;
}

#endif