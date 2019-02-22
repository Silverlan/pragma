#include "stdafx_engine.h"
#ifdef _WIN32
#include "pragma/debug/mdump.h"
#include <tchar.h>
#include <fsys/filesystem.h>
#include <util_zip.h>
#include <sharedutils/util_debug.h>
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include "pragma/engine_info.hpp"

extern DLLENGINE Engine *engine;

LPCSTR MiniDumper::m_szAppName;

static std::string s_exceptionMessage = {};
MiniDumper::MiniDumper( LPCSTR szAppName )
{
	// if this assert fires then you have two instances of MiniDumper
	// which is not allowed
	assert( m_szAppName==NULL );

	m_szAppName = szAppName ? strdup(szAppName) : "Application";

	::SetUnhandledExceptionFilter( TopLevelFilter );
	// Note: set_terminate handler is called before SetUnhandledExceptionFilter.
	// set_terminate allows us to retrieve the underlying message from the exception (if there was one)
	/*set_terminate([]() {
		auto eptr = std::current_exception();
		if(!eptr)
		{
			s_exceptionMessage = {};
			return;
		}
		try
		{
			 std::rethrow_exception(eptr);
		}
		catch(const std::exception &e)
		{
			s_exceptionMessage = std::string{typeid(e).name()} +": " +e.what();
		}
		catch(...)
		{
			s_exceptionMessage = "Unknown Exception";
		}
		// Relay exception to SetUnhandledExceptionFilter
		std::rethrow_exception(eptr);
	});*/
}

LONG MiniDumper::TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	// MessageBox(0,s_exceptionMessage.c_str(),"Exception",MB_OK);
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;						// find a better value for your app

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	char szDbgHelpPath[_MAX_PATH];

	if (GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ))
	{
		char *pSlash = _tcsrchr( szDbgHelpPath, '\\' );
		if (pSlash)
		{
			_tcscpy( pSlash+1, "DBGHELP.DLL" );
			hDll = ::LoadLibrary( szDbgHelpPath );
		}
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary( "DBGHELP.DLL" );
	}

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			FileManager::CreateDirectory("crashdumps");
			auto programPath = util::get_program_path();
			auto szDumpPath = programPath +"/crashdumps/";
			szDumpPath += std::string(m_szAppName) +std::string(".dmp");
			char szScratch [_MAX_PATH];

			// ask the user if they want to save a dump file
			if (::MessageBox( NULL, "A terminal error has occurred in the program. Would you like to save a diagnostic file?", m_szAppName, MB_YESNO )==IDYES)
			{
				// create the file
				HANDLE hFile = ::CreateFile( szDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL, NULL );

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					// write the dump
					//MiniDumpWithFullMemory
					BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
					::CloseHandle(hFile);
					if (bOK)
					{
						auto zipName = programPath +std::string("/") +util::get_date_time("crashdumps/crashdump_%Y-%m-%d_%H-%M-%S.zip");
						auto zipFile = ZIPFile::Open(zipName,ZIPFile::OpenFlags::CreateIfNotExist);
						if(zipFile != nullptr)
						{
							// Write Exception
							if(s_exceptionMessage.empty() == false)
								zipFile->AddFile("exception.txt",s_exceptionMessage);

							// Write Stack Backtrace
							zipFile->AddFile("stack_backtrace.txt",util::get_formatted_stack_backtrace_string());
							
							// Write Info
							if(engine != nullptr)
								engine->DumpDebugInformation(*zipFile.get());

							// Write Minidump
							VFilePtrReal f = nullptr;
							auto t = std::chrono::high_resolution_clock::now();
							while(f == nullptr) // Wait until dump has been written
							{
								auto tNow = std::chrono::high_resolution_clock::now();
								auto tDelta = std::chrono::duration_cast<std::chrono::seconds>(tNow -t).count();
								if(tDelta >= 4) // Don't wait more than 4 seconds
									break;
								std::this_thread::sleep_for(std::chrono::milliseconds(250));
								f = FileManager::OpenSystemFile(szDumpPath.c_str(),"rb");
							}
							if(f != nullptr)
							{
								auto size = f->GetSize();
								std::vector<uint8_t> dumpData(size);
								f->Read(dumpData.data(),size);
								f = nullptr;
								zipFile->AddFile(ufile::get_file_from_filename(szDumpPath),dumpData.data(),size);
								std::remove(szDumpPath.c_str());
							}
							zipFile = nullptr;
							sprintf(szScratch,"Saved dump file to '%s'. Please send it to %s, along with a description of what you did to trigger the error.",zipName.c_str(),engine_info::get_author_mail_address().c_str());
							szResult = szScratch;
							retval = EXCEPTION_EXECUTE_HANDLER;
						}
						else
						{
							sprintf( szScratch, "Failed to create dump file '%s'", zipName.c_str() );
							szResult = szScratch;
						}
					}
					else
					{
						sprintf( szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath.c_str(), GetLastError() );
						szResult = szScratch;
					}
				}
				else
				{
					sprintf( szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath.c_str(), GetLastError() );
					szResult = szScratch;
				}
			}
		}
		else
		{
			szResult = "DBGHELP.DLL too old";
		}
	}
	else
	{
		szResult = "DBGHELP.DLL not found";
	}

	if (szResult)
		::MessageBox( NULL, szResult, m_szAppName, MB_OK );

	return retval;
}

#endif