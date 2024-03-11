/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/curl_query_handler.hpp"
#include <sharedutils/util_library.hpp>

decltype(pragma::CurlQueryHandler::s_bInitialized) pragma::CurlQueryHandler::s_bInitialized = false;
decltype(pragma::CurlQueryHandler::s_fCreate) pragma::CurlQueryHandler::s_fCreate = nullptr;
decltype(pragma::CurlQueryHandler::s_fRelease) pragma::CurlQueryHandler::s_fRelease = nullptr;
decltype(pragma::CurlQueryHandler::s_fAddResource) pragma::CurlQueryHandler::s_fAddResource = nullptr;
decltype(pragma::CurlQueryHandler::s_fStartDownload) pragma::CurlQueryHandler::s_fStartDownload = nullptr;
decltype(pragma::CurlQueryHandler::s_fCancelDownload) pragma::CurlQueryHandler::s_fCancelDownload = nullptr;
decltype(pragma::CurlQueryHandler::s_fIsComplete) pragma::CurlQueryHandler::s_fIsComplete = nullptr;
decltype(pragma::CurlQueryHandler::s_fSetErrorHandler) pragma::CurlQueryHandler::s_fSetErrorHandler = nullptr;
decltype(pragma::CurlQueryHandler::s_fSendRequest) pragma::CurlQueryHandler::s_fSendRequest = nullptr;
decltype(pragma::CurlQueryHandler::s_fCodeToString) pragma::CurlQueryHandler::s_fCodeToString = nullptr;
decltype(pragma::CurlQueryHandler::s_fIsErrorCode) pragma::CurlQueryHandler::s_fIsErrorCode = nullptr;

pragma::CurlQueryHandler::CurlQueryHandler()
{
	Initialize();
	if(s_fCreate == nullptr) {
		throw std::runtime_error("Unable to load 'curl' libary!");
		return;
	}
	m_curl = s_fCreate();
}

pragma::CurlQueryHandler::~CurlQueryHandler()
{
	if(IsComplete() == false) {
		CancelDownload();
		s_fRelease(m_curl);
		for(auto &f : m_files) // TODO: Only remove files which haven't been fully downloaded yet
		{
			auto path = f->GetPath();
			f = nullptr;
			FileManager::RemoveFile(path.c_str());
		}
	}
	else
		s_fRelease(m_curl);
}
void pragma::CurlQueryHandler::AddResource(const std::string &url, const std::string &fname, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &progressCallback, const std::function<void(int32_t)> &onComplete)
{
	auto f = FileManager::OpenFile<VFilePtrReal>(fname.c_str(), "wb");
	if(f == nullptr)
		return;
	m_files.push_back(f);
	struct FileData {
		FileData(const VFilePtrReal &f) : file {f} {}
		VFilePtrReal file;
	};
	auto fd = std::make_shared<FileData>(f);
	auto *fptr = f.get();
	s_fAddResource(
	  m_curl, url,
	  [fptr](void *data, size_t size, size_t nmemb) -> size_t {
		  fptr->Write(data, size * nmemb);
		  return size * nmemb;
	  },
	  nullptr, progressCallback,
	  [this, onComplete, fd](int32_t code) {
		  auto fptr = fd->file.get();
		  auto it = std::find_if(m_files.begin(), m_files.end(), [fptr](const std::shared_ptr<VFilePtrInternalReal> &fOther) { return (fOther.get() == fptr) ? true : false; });
		  if(it != m_files.end())
			  m_files.erase(it);
		  fd->file = nullptr;
		  if(onComplete != nullptr)
			  onComplete(code);
	  });
}
void pragma::CurlQueryHandler::AddRequest(const std::string &url, const std::unordered_map<std::string, std::string> &post, const std::function<void(int32_t, const std::string &)> &onComplete, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &progressCallback)
{
	s_fSendRequest(m_curl, url, post, onComplete, progressCallback);
}
void pragma::CurlQueryHandler::StartDownload() { s_fStartDownload(m_curl); }
void pragma::CurlQueryHandler::CancelDownload() { s_fCancelDownload(m_curl); }
bool pragma::CurlQueryHandler::IsComplete() const { return s_fIsComplete(m_curl); }
void pragma::CurlQueryHandler::SetErrorHandler(const std::function<void(uint32_t)> &f) { s_fSetErrorHandler(m_curl, f); }
std::string pragma::CurlQueryHandler::CodeToString(int32_t code) const {
    std::string str;
    s_fCodeToString(code, str);
    return str;
}
bool pragma::CurlQueryHandler::IsErrorCode(int32_t code) const { return s_fIsErrorCode(code); }

void pragma::CurlQueryHandler::Initialize()
{
	if(s_bInitialized == true)
		return;
	s_bInitialized = true;
	std::string err;
#ifdef _WIN32
	const std::string curlPath = "curl/pr_curl.dll";
#else
	const std::string curlPath = "curl/libpr_curl.so";
#endif
	auto *nw = static_cast<NetworkState *>(pragma::get_engine()->GetServerNetworkState());
	if(!nw)
		nw = pragma::get_engine()->GetClientState();

	if(!nw || nw->InitializeLibrary(curlPath, &err) == nullptr) {
		Con::cerr << "Unable to load 'curl' library: " << err << Con::endl;
		return;
	}
	auto dllHandle = nw->GetLibraryModule(curlPath);
	assert(dllHandle != nullptr);
	if(dllHandle == nullptr)
		return;
	s_fCreate = dllHandle->FindSymbolAddress<decltype(s_fCreate)>("mcd_create");
	s_fRelease = dllHandle->FindSymbolAddress<decltype(s_fRelease)>("mcd_release");
	s_fAddResource = dllHandle->FindSymbolAddress<decltype(s_fAddResource)>("mcd_add_resource");
	s_fStartDownload = dllHandle->FindSymbolAddress<decltype(s_fStartDownload)>("mcd_start_download");
	s_fCancelDownload = dllHandle->FindSymbolAddress<decltype(s_fCancelDownload)>("mcd_cancel_download");
	s_fIsComplete = dllHandle->FindSymbolAddress<decltype(s_fIsComplete)>("mcd_is_complete");
	s_fSetErrorHandler = dllHandle->FindSymbolAddress<decltype(s_fSetErrorHandler)>("mcd_set_error_handler");
	s_fSendRequest = dllHandle->FindSymbolAddress<decltype(s_fSendRequest)>("mcd_send_request");
	s_fCodeToString = dllHandle->FindSymbolAddress<decltype(s_fCodeToString)>("mcd_code_to_string");
	s_fIsErrorCode = dllHandle->FindSymbolAddress<decltype(s_fIsErrorCode)>("mcd_is_error_code");
}
