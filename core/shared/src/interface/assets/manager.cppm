// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:assets.manager;

import :assets.common;
export import :types;

export namespace pragma::asset {
	class DLLNETWORK AssetManager {
	  public:
		struct DLLNETWORK ImporterInfo {
			std::string name;
			std::string description;
			std::vector<std::pair<std::string, bool /* binary */>> fileExtensions;
		};
		using ExporterInfo = ImporterInfo;
		using ImportHandler = std::function<std::unique_ptr<IAssetWrapper>(Game &, ufile::IFile &, const std::optional<std::string> &, std::string &)>;
		using ExportHandler = std::function<bool(Game &, ufile::IFile &, const IAssetWrapper &, std::string &)>;
		void RegisterImporter(const ImporterInfo &importerInfo, Type type, const ImportHandler &importHandler);
		void RegisterExporter(const ExporterInfo &importerInfo, Type type, const ExportHandler &exportHandler);
		std::unique_ptr<IAssetWrapper> ImportAsset(Game &game, Type type, ufile::IFile *f, const std::optional<std::string> &filePath = {}, std::string *optOutErr = nullptr) const;
		bool ExportAsset(Game &game, Type type, ufile::IFile &f, const IAssetWrapper &assetWrapper, std::string *optOutErr = nullptr) const;

		uint32_t GetImporterCount(Type type) const { return m_importers[math::to_integral(type)].size(); }
		uint32_t GetExporterCount(Type type) const { return m_exporters[math::to_integral(type)].size(); }
		const ImporterInfo *GetImporterInfo(Type type, uint32_t idx) const
		{
			auto &importers = m_importers[math::to_integral(type)];
			return (idx < importers.size()) ? &importers.at(idx).info : nullptr;
		}
		const ExporterInfo *GetExporterInfo(Type type, uint32_t idx) const
		{
			auto &exporters = m_exporters[math::to_integral(type)];
			return (idx < exporters.size()) ? &exporters.at(idx).info : nullptr;
		}
	  private:
		struct DLLNETWORK Importer {
			ImporterInfo info;
			ImportHandler handler;
		};
		struct DLLNETWORK Exporter {
			ExporterInfo info;
			ExportHandler handler;
		};
		std::array<std::vector<Importer>, math::to_integral(Type::Count)> m_importers;
		std::array<std::vector<Exporter>, math::to_integral(Type::Count)> m_exporters;
	};
};
