/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CURL_DOWNLOAD_BATCH_HPP__
#define __CURL_DOWNLOAD_BATCH_HPP__

#include "pragma/definitions.h"
#include <fsys/vfileptr.h>
#include <string>
#include <functional>
#include <memory>

namespace pragma {
	class DLLNETWORK CurlQueryHandler {
	  public:
		CurlQueryHandler();
		~CurlQueryHandler();
		void AddResource(const std::string &url, const std::string &fname, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &progressCallback = nullptr, const std::function<void(int32_t)> &onComplete = nullptr);
		void AddRequest(const std::string &url, const std::unordered_map<std::string, std::string> &post, const std::function<void(int32_t, const std::string &)> &onComplete = nullptr, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &progressCallback = nullptr);
		void StartDownload();
		void CancelDownload();
		bool IsComplete() const;
		void SetErrorHandler(const std::function<void(uint32_t)> &f);
		std::string CodeToString(int32_t code) const;
		bool IsErrorCode(int32_t code) const;
	  private:
		void Initialize();
		void *m_curl = nullptr;
		std::vector<std::shared_ptr<VFilePtrInternalReal>> m_files;
		static bool s_bInitialized;
		static void *(*s_fCreate)();
		static void (*s_fRelease)(void *);
		static void (*s_fAddResource)(void *, const std::string &, const std::function<size_t(void *, size_t, size_t)> &, const std::shared_ptr<void> &, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &, const std::function<void(int32_t)> &);
		static void (*s_fStartDownload)(void *);
		static void (*s_fCancelDownload)(void *);
		static bool (*s_fIsComplete)(void *);
		static void (*s_fSetErrorHandler)(void *, const std::function<void(uint32_t)> &);
		static void (*s_fSendRequest)(void *, const std::string &, const std::unordered_map<std::string, std::string> &, const std::function<void(int32_t, const std::string &)> &, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &);
		static std::string (*s_fCodeToString)(int32_t,std::string&);
		static bool (*s_fIsErrorCode)(int32_t);
	};
};

#endif
