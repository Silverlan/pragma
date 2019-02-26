#include "fbxfile.h"
#include <assert.h>

FBXFile::FBXFile(const VFilePtr &f)
	: m_file(f)
{}
fbxsdk::FbxStream::EState FBXFile::GetState()
{
	if(m_file == nullptr)
		return fbxsdk::FbxStream::EState::eClosed;
	if(m_file->GetSize() == 0)
		return fbxsdk::FbxStream::EState::eEmpty;
	return fbxsdk::FbxStream::EState::eOpen;
}
bool FBXFile::Open(void *pStreamData)
{
	m_file->Seek(0);
	return true;
}
bool FBXFile::Close()
{
	m_file->Seek(0);
	return true;
}
bool FBXFile::Flush()
{
	return false;
}
int FBXFile::Write(const void* /*pData*/, int /*pSize*/)
{
	return 0;
}
int FBXFile::Read(void* pData, int pSize) const
{
	return m_file->Read(pData,pSize);
}
int FBXFile::GetReaderID() const
{
	return 0;//-1;
}
int FBXFile::GetWriterID() const
{
	return 0;//-1;
}
void FBXFile::Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos)
{
	m_file->Seek(pOffset,pSeekPos);
}
long FBXFile::GetPosition() const
{
	return m_file->Tell();
}
void FBXFile::SetPosition(long pPosition)
{
	m_file->Seek(pPosition);
}
int FBXFile::GetError() const
{
	return 0;
}
void FBXFile::ClearError()
{}
