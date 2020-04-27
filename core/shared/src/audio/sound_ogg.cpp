/*
Copyright (c) 2002-2018 Xiph.org Foundation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx_shared.h"
#include <fsys/filesystem.h>
#include <vorbis/vorbisfile.h>
#include "pragma/audio/sound_ogg.h"

static size_t ogg_read_func(void *ptr,size_t size,size_t nmemb,void *dataSource)
{
	auto &f = *static_cast<VFilePtr*>(dataSource);
	return f->Read(ptr,size *nmemb);
}

static int ogg_seek_func(void *dataSource,ogg_int64_t offset,int whence)
{
	auto &f = *static_cast<VFilePtr*>(dataSource);
	f->Seek(offset,whence);
	return 0;
}

static int ogg_close_func(void *dataSource)
{
	auto &f = *static_cast<VFilePtr*>(dataSource);
	f.reset();
	return 0;
}

static long ogg_tell_func(void *dataSource)
{
	auto f = *static_cast<VFilePtr*>(dataSource);
	return static_cast<long>(f->Tell());
}

DLLNETWORK void GetOGGFileData(VFilePtr f,vorbis_info **pInfo,OggVorbis_File &oggFile)
{
	ov_callbacks cb;
	cb.close_func = &ogg_close_func;
	cb.read_func = &ogg_read_func;
	cb.seek_func = &ogg_seek_func;
	cb.tell_func = &ogg_tell_func;
	ov_open_callbacks(&f,&oggFile,NULL,0,cb);
	*pInfo = ov_info(&oggFile,-1);
}

DLLNETWORK float GetOGGSoundDuration(VFilePtr f)
{
	vorbis_info *pInfo;
	OggVorbis_File oggFile;
	GetOGGFileData(f,&pInfo,oggFile);
	long long bitrate = ov_bitrate(&oggFile,-1);
	if(bitrate == 0) // This shouldn't happen, but it does for some files. Reason unknown
		bitrate = pInfo->bitrate_nominal;
	ogg_int64_t numBytes = ov_raw_total(&oggFile,-1);
	ov_clear(&oggFile);
	float d = (bitrate /8.f);
	if(d == 0.f)
		return 0.f;
	return float(numBytes) /d;
}
