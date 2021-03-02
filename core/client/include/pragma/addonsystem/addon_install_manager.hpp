/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ADDON_UPDATE_HANDLER_HPP__
#define __ADDON_UPDATE_HANDLER_HPP__

#include "pragma/clientdefinitions.h"
#include <vector>
#include <atomic>

class AddonInfo;
namespace pragma
{
	class CurlQueryHandler;
	class DLLCLIENT AddonInstallManager
	{
	public:
		AddonInstallManager();
		void CheckForUpdates(const std::shared_ptr<AddonInfo> &addon,const std::shared_ptr<std::atomic<float>> &totalProgress=nullptr);
		void CancelDownload();
	private:
		std::shared_ptr<CurlQueryHandler> m_curlQueryHandler = nullptr;

		struct AddonUpdateQuery
		{
			AddonUpdateQuery(const std::shared_ptr<AddonInfo> &info);
			std::shared_ptr<AddonInfo> addonInfo;
			std::string addonPath;
			VFilePtrReal updateFile;
			// Contains file ids for files which aren't downloaded yet
			std::vector<uint32_t> fileIds;
			std::vector<std::string> removeFiles;
			std::shared_ptr<std::atomic<float>> totalProgress = nullptr;
			std::string GetUpdateFilePath() const;
		};

		void QueryUpdateFileInfo(const std::shared_ptr<AddonUpdateQuery> &addon,const std::unordered_map<uint32_t,bool> &skipFileIds);
		void QueryFile(const std::shared_ptr<AddonUpdateQuery> &addon,uint32_t fileId,const std::string &filePath,uint64_t fileSize,const std::function<void(int64_t,int64_t,int64_t,int64_t)> &progressCallback);
	};
};

#endif
