// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :core.addon_install_manager;
import :engine;
import pragma.uva;
import pragma.pad;

static const std::string &get_query_url()
{
	//static auto url = engine_info::get_modding_hub_url() + "html/game/";
	//return url;
	throw std::runtime_error {"This is obsolete, do not use!"};
	static std::string r;
	return r;
}

pragma::AddonInstallManager::AddonUpdateQuery::AddonUpdateQuery(const std::shared_ptr<AddonInfo> &info) : addonInfo(info) {}

std::string pragma::AddonInstallManager::AddonUpdateQuery::GetUpdateFilePath() const
{
	std::string updateFilePath = "addons\\" + addonInfo->GetUniqueId();
	updateFilePath += ".pad.update";
	return updateFilePath;
}

////////////////////

pragma::AddonInstallManager::AddonInstallManager() : m_curlQueryHandler(pragma::util::make_shared<CurlQueryHandler>()) {}

void pragma::AddonInstallManager::CancelDownload() { m_curlQueryHandler->CancelDownload(); }

void pragma::AddonInstallManager::QueryFile(const std::shared_ptr<AddonUpdateQuery> &addon, uint32_t fileId, const std::string &filePath, uint64_t fileSize, const std::function<void(int64_t, int64_t, int64_t, int64_t)> &progressCallback)
{
	m_curlQueryHandler->AddRequest(
	  get_query_url() + "query_addon_request_file.php", {{"addonid", addon->addonInfo->GetUniqueId()}, {"fileid", std::to_string(fileId)}},
	  [this, addon, fileId, filePath, fileSize](int32_t code, const std::string &response) {
		  if(m_curlQueryHandler->IsErrorCode(code) == false) {
			  auto &updateFile = addon->updateFile;
			  updateFile->Write<uint32_t>(fileId);
			  updateFile->WriteString(filePath);
			  updateFile->Write<uint64_t>(response.size());
			  updateFile->Write<uint64_t>(fileSize);
			  updateFile->Write(response.data(), response.size());

			  auto it = std::find(addon->fileIds.begin(), addon->fileIds.end(), fileId);
			  if(it != addon->fileIds.end()) {
				  addon->fileIds.erase(it);
				  if(addon->fileIds.empty() == true && updateFile->ReOpen("rb") == true) {
					  pad::PADPackage *pad = nullptr;
					  uva::ArchiveFile *archFile = nullptr;
					  std::unique_ptr<uva::ArchiveFile> ptrArchFile = nullptr;
					  if(addon->addonInfo->GetLocalPath().empty() == true) // New addon
					  {
						  std::shared_ptr<pad::PADPackage::Header> header = nullptr;
						  ptrArchFile = pragma::pad::open(addon->addonPath, header);
						  archFile = ptrArchFile.get();

						  auto &uniqueId = addon->addonInfo->GetUniqueId();
						  memcpy(header->packageId.data(), uniqueId.data(), header->packageId.size());
					  }
					  else {
						  auto *padManager = get_cengine()->GetPADPackageManager();
						  pad = (padManager != nullptr) ? padManager->GetPackage(addon->addonPath) : nullptr;
						  if(pad != nullptr)
							  archFile = pad->GetArchiveFile();
					  }

					  if(archFile != nullptr) {
						  for(auto &name : addon->removeFiles) {
							  auto *fi = archFile->FindFile(name);
							  if(fi == nullptr)
								  continue;
							  fi->size = 0;
							  fi->sizeUncompressed = 0;
						  }
						  if(addon->updateFile->ReOpen("rb") == true) {
							  fs::VFilePtr updateFile = addon->updateFile;
							  updateFile->Seek(0);
							  auto version = updateFile->Read<util::Version>();
							  std::vector<uint32_t> newFileIds;
							  while(updateFile->ReadChar() != std::char_traits<char>::eof()) {
								  updateFile->Seek(updateFile->Tell() - 1);
								  auto fileId = updateFile->Read<uint32_t>();
								  auto filePath = updateFile->ReadString();

								  auto size = updateFile->Read<uint64_t>();
								  auto sizeUncompressed = updateFile->Read<uint64_t>();
								  auto data = pragma::util::make_shared<std::vector<uint8_t>>(size);
								  updateFile->Read(data->data(), data->size());

								  uint32_t idx = 0;
								  auto *fi = archFile->AddFile(filePath, idx);
								  if(fi != nullptr) {
									  fi->data = data;
									  fi->size = size;
									  fi->sizeUncompressed = sizeUncompressed;

									  newFileIds.push_back(idx);
								  }
							  }

							  // Add new version information
							  uva::VersionInfo vi {};
							  vi.version = version;
							  vi.files = newFileIds;
							  archFile->AddVersion(vi);

							  updateFile = nullptr;

							  auto r = archFile->Export();
							  if(r == true) {
								  addon->updateFile = nullptr;
								  auto updateFilePath = addon->GetUpdateFilePath();
								  if(fs::remove_file(updateFilePath) == false)
									  ;
							  }
							  else
								  Con::CWAR << "Unable to finalize archive file '" << addon->addonPath << "'!" << Con::endl;
						  }

						  // Re-open file handle for existing archives
						  if(pad != nullptr) {
							  pad->Close();
							  if(pad->Open() == false)
								  ;
						  }

						  // New archives are automatically mounted by the engine
					  }
				  }
			  }
		  }
	  },
	  progressCallback);
}

void pragma::AddonInstallManager::QueryUpdateFileInfo(const std::shared_ptr<AddonUpdateQuery> &addon, const std::unordered_map<uint32_t, bool> &skipFileIds)
{
	auto addonVersion = util::Version {};
	m_curlQueryHandler->AddRequest(get_query_url() + "query_addon_update_file_headers.php", {{"addonid", addon->addonInfo->GetUniqueId()}, {"major", std::to_string(addonVersion.major)}, {"minor", std::to_string(addonVersion.minor)}, {"revision", std::to_string(addonVersion.revision)}},
	  [this, addon, skipFileIds](int32_t code, const std::string &response) {
		  if(m_curlQueryHandler->IsErrorCode(code) == false) {
			  std::vector<std::string> fileHeaders;
			  string::explode(response, "\n", fileHeaders);

			  struct FileQuery {
				  uint32_t fileId;
				  std::string filePath;
				  uint64_t fileSize;
				  float progress = 0.f;
			  };
			  auto queries = pragma::util::make_shared<std::vector<FileQuery>>();
			  queries->reserve(fileHeaders.size());
			  addon->fileIds.reserve(fileHeaders.size());
			  uint64_t totalSize = 0;
			  for(auto &line : fileHeaders) {
				  std::vector<std::string> fh;
				  string::explode(line, ";", fh);
				  if(fh.size() < 4)
					  continue;
				  auto fileId = string::to_int(fh.at(0));
				  auto it = skipFileIds.find(fileId);
				  if(it != skipFileIds.end())
					  continue; // Skip this file; We've already downloaded it!

				  FileQuery query {};
				  query.fileId = fileId;
				  query.filePath = fh.at(1);
				  query.fileSize = string::to_int(fh.at(3));
				  if(string::to_int(fh.at(2)) == 0)
					  query.fileSize = 0;
				  if(query.fileSize == 0)
					  addon->removeFiles.push_back(query.filePath);
				  else {
					  queries->push_back(query);
					  addon->fileIds.push_back(query.fileId);
				  }

				  totalSize += string::to_int(fh.at(2));
			  }
			  for(auto &query : *queries) {
				  std::function<void(int64_t, int64_t, int64_t, int64_t)> fUpdateProgress = nullptr;
				  if(addon->totalProgress != nullptr) {
					  fUpdateProgress = [addon, &query, queries](int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow) {
						  if(dltotal == 0 || addon->totalProgress == nullptr)
							  return;
						  query.progress = dlnow / static_cast<float>(dltotal);

						  auto newTotalProgress = 0.f;
						  for(auto &query : *queries)
							  newTotalProgress += query.progress;
						  newTotalProgress /= static_cast<float>(queries->size());
						  *addon->totalProgress = newTotalProgress;
					  };
				  }
				  QueryFile(addon, query.fileId, query.filePath, query.fileSize, fUpdateProgress);
			  }
		  }
	  });
}

void pragma::AddonInstallManager::CheckForUpdates(const std::shared_ptr<AddonInfo> &addon, const std::shared_ptr<std::atomic<float>> &totalProgress)
{
	auto updateQuery = pragma::util::make_shared<AddonUpdateQuery>(addon);
	updateQuery->totalProgress = totalProgress;
	m_curlQueryHandler->AddRequest(get_query_url() + "query_addon_version.php", {{"addonid", addon->GetUniqueId()}}, [this, updateQuery](int32_t code, const std::string &response) {
		if(m_curlQueryHandler->IsErrorCode(code) == false) {
			std::vector<std::string> subStrings;
			string::explode(response, ";", subStrings);
			if(subStrings.size() >= 2) {
				updateQuery->addonPath = "addons\\" + subStrings.at(0);
				auto &addonPath = updateQuery->addonInfo->GetLocalPath();
				if(addonPath.empty() == false && updateQuery->addonPath != addonPath) {
					// File name was changed by addon author; Change local file name accordingly
					if(fs::rename_file(addonPath, updateQuery->addonPath) == false)
						return; // TODO
				}
				auto webVersion = util::Version::FromString(subStrings.at(1));
				auto &addonVersion = updateQuery->addonInfo->GetVersion();
				if(webVersion > addonVersion) {
					auto updateFilePath = updateQuery->GetUpdateFilePath();
					std::unordered_map<uint32_t, bool> skipFileIds; // List of file ids that were already downloaded previously
					if(fs::exists(updateFilePath, fs::SearchFlags::Local) == true) {
						auto fRead = pragma::fs::open_file(updateFilePath.c_str(), fs::FileMode::Read | fs::FileMode::Binary);
						if(fRead != nullptr) {
							auto lastUpdateVersion = fRead->Read<util::Version>();
							if(lastUpdateVersion == webVersion) {
								while(fRead->ReadChar() != std::char_traits<char>::eof()) {
									fRead->Seek(fRead->Tell() - 1);
									auto fileId = fRead->Read<uint32_t>();
									skipFileIds.insert(std::make_pair(fileId, true));
									auto filePath = fRead->ReadString();

									auto size = fRead->Read<uint64_t>();
									auto sizeUncompressed = fRead->Read<uint64_t>();
									fRead->Seek(fRead->Tell() + size); // Skip data and jump to next file
								}
							}
						}
					}
					updateQuery->updateFile = fs::open_file<fs::VFilePtrReal>(updateFilePath, (skipFileIds.empty() == true) ? (fs::FileMode::Write | fs::FileMode::Binary) : (fs::FileMode::Append | fs::FileMode::Binary));
					if(updateQuery->updateFile == nullptr)
						return;
					if(skipFileIds.empty() == true)
						updateQuery->updateFile->Write<util::Version>(webVersion);
					else
						updateQuery->updateFile->Seek(sizeof(util::Version));
					QueryUpdateFileInfo(updateQuery, skipFileIds);
				}
			}
		}
	});
	m_curlQueryHandler->StartDownload();
}
