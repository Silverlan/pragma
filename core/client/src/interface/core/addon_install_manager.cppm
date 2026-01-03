// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:core.addon_install_manager;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT AddonInstallManager {
	  public:
		AddonInstallManager();
		void CheckForUpdates(const std::shared_ptr<AddonInfo> &addon, const std::shared_ptr<std::atomic<float>> &totalProgress = nullptr);
		void CancelDownload();
	  private:
		std::shared_ptr<CurlQueryHandler> m_curlQueryHandler = nullptr;

		struct AddonUpdateQuery {
			AddonUpdateQuery(const std::shared_ptr<AddonInfo> &info);
			std::shared_ptr<AddonInfo> addonInfo;
			std::string addonPath;
			fs::VFilePtrReal updateFile;
			// Contains file ids for files which aren't downloaded yet
			std::vector<uint32_t> fileIds;
			std::vector<std::string> removeFiles;
			std::shared_ptr<std::atomic<float>> totalProgress = nullptr;
			std::string GetUpdateFilePath() const;
		};

		void QueryUpdateFileInfo(const std::shared_ptr<AddonUpdateQuery> &addon, const std::unordered_map<uint32_t, bool> &skipFileIds);
		void QueryFile(const std::shared_ptr<AddonUpdateQuery> &addon, uint32_t fileId, const std::string &filePath, uint64_t fileSize, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &progressCallback);
	};
};
