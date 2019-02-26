#ifndef __FBXFILE_H__
#define __FBXFILE_H__

#include <fbxsdk.h>
#include <fsys/filesystem.h>

class FBXFile
	: public fbxsdk::FbxStream
{
private:
	VFilePtr m_file;
public:
	FBXFile(const VFilePtr &f);
	virtual EState GetState() override;
	virtual bool Open(void* pStreamData) override;
	virtual bool Close() override;
	virtual bool Flush() override;
	virtual int Write(const void* /*pData*/, int /*pSize*/) override;
	virtual int Read(void* /*pData*/, int /*pSize*/) const override;
	virtual int GetReaderID() const override;
	virtual int GetWriterID() const override;
	virtual void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos) override;
	virtual long GetPosition() const override;
	virtual void SetPosition(long pPosition) override;
	virtual int GetError() const override;
	virtual void ClearError() override;
};

#endif
