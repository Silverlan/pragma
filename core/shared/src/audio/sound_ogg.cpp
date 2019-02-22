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
