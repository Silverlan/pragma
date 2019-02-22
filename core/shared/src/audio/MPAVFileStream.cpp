#include "stdafx_shared.h"
#ifdef _WIN32
#ifdef WEAVE_MP3_SUPPORT_ENABLED
#include <Windows.h>
#include <atlstr.h>
#include "pragma/audio/MPAVFileStream.h"
#include "mpaexception.h"
#include "mpaendoffileexception.h"
#include <fsys/filesystem.h>

// 1KB is inital buffersize
const DWORD CMPAVFileStream::INIT_BUFFERSIZE = 1024;	

CMPAVFileStream::CMPAVFileStream(VFilePtr f) :
	CMPAStream(""),m_dwOffset(0),m_hFile(f),m_bMustReleaseFile(true)
{
	Init();
}


void CMPAVFileStream::Init() 
{
	m_dwBufferSize = INIT_BUFFERSIZE;
	// fill buffer for first time
	m_pBuffer = new BYTE[m_dwBufferSize];
	FillBuffer(m_dwOffset, m_dwBufferSize, false);
}

CMPAVFileStream::~CMPAVFileStream(void)
{
	if (m_pBuffer)
		delete[] m_pBuffer;
	
	// close file
	if (m_bMustReleaseFile)
		m_hFile.reset();
}

// VC++6 doesn't contain this definition
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

// set file position
void CMPAVFileStream::SetPosition(DWORD dwOffset) const
{
	// convert from unsigned DWORD to signed 64bit long
	m_hFile->Seek(dwOffset);
}


BYTE* CMPAVFileStream::ReadBytes(DWORD dwSize, DWORD& dwOffset, bool bMoveOffset, bool bReverse) const
{
	// enough bytes in buffer, otherwise read from file
	if (m_dwOffset > dwOffset || ( ((int)((m_dwOffset + m_dwBufferSize) - dwOffset)) < (int)dwSize))
	{
		if (!FillBuffer(dwOffset, dwSize, bReverse)) 
		{
			throw CMPAEndOfFileException(m_szFile);
		}
	}

	BYTE* pBuffer = m_pBuffer + (dwOffset-m_dwOffset);
	if (bMoveOffset)
		dwOffset += dwSize;
	
	return pBuffer;
}

DWORD CMPAVFileStream::GetSize() const
{
	return static_cast<DWORD>(m_hFile->GetSize());
}

// fills internal buffer, returns false if EOF is reached, otherwise true. Throws exceptions
bool CMPAVFileStream::FillBuffer(DWORD dwOffset, DWORD dwSize, bool bReverse) const
{
	// calc new buffer size
	if (dwSize > m_dwBufferSize)
	{
        m_dwBufferSize = dwSize;
		
		// release old buffer 
		delete[] m_pBuffer;

		// reserve new buffer
		m_pBuffer = new BYTE[m_dwBufferSize];
	}	

	if (bReverse)
	{
		if (dwOffset + dwSize < m_dwBufferSize)
			dwOffset = 0;
		else
			dwOffset = dwOffset + dwSize - m_dwBufferSize;
	}

	// read <m_dwBufferSize> bytes from offset <dwOffset>
	m_dwBufferSize = Read(m_pBuffer, dwOffset, m_dwBufferSize);

	// set new offset
	m_dwOffset = dwOffset;

	if (m_dwBufferSize < dwSize)
		return false;

	return true;
}

// read from file, return number of bytes read
DWORD CMPAVFileStream::Read(LPVOID pData, DWORD dwOffset, DWORD dwSize) const
{
	DWORD dwBytesRead = 0;
	
	// set position first
	SetPosition(dwOffset);

	dwBytesRead = static_cast<DWORD>(m_hFile->Read(pData,dwSize));
	
	return dwBytesRead;
}
#endif
#endif