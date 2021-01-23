/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __PRAGMA_UTIL_ASSET_HPP__
#define __PRAGMA_UTIL_ASSET_HPP__

#include "pragma/networkdefinitions.h"

namespace pragma::asset
{
	enum class Type : uint8_t
	{
		Model = 0,
		Material,
		Texture,
		Sound,
		ParticleSystem,

		Count
	};
	DLLNETWORK bool exists(NetworkState &nw,const std::string &name,Type type);
	DLLNETWORK std::optional<std::string> find_file(NetworkState &nw,const std::string &name,Type type);
	DLLNETWORK bool is_loaded(NetworkState &nw,const std::string &name,Type type);

	struct DLLNETWORK IAssetWrapper
	{
		virtual ~IAssetWrapper()=default;
		virtual Type GetType() const=0;
	};

	struct DLLNETWORK ModelAssetWrapper
		: public IAssetWrapper
	{
		virtual Type GetType() const override {return Type::Model;}
		void SetModel(Model &model);
		Model *GetModel() const;
	private:
		std::shared_ptr<Model> m_model = nullptr;
	};

	struct DLLNETWORK MaterialAssetWrapper
		: public IAssetWrapper
	{
		virtual Type GetType() const override {return Type::Material;}
		void SetMaterial(Material &mat);
		Material *GetMaterial() const;
	private:
		MaterialHandle m_material {};
	};

	class DLLNETWORK AssetManager
	{
	public:
		struct DLLNETWORK ImporterInfo
		{
			std::string name;
			std::string description;
			std::vector<std::string> fileExtensions;
		};
		using ExporterInfo = ImporterInfo;
		using ImportHandler = std::function<std::unique_ptr<IAssetWrapper>(VFilePtr,const std::optional<std::string>&,std::string&)>;
		using ExportHandler = std::function<bool(VFilePtrReal,const IAssetWrapper&,std::string&)>;
		void RegisterImporter(const ImporterInfo &importerInfo,Type type,const ImportHandler &importHandler);
		void RegisterExporter(const ExporterInfo &importerInfo,Type type,const ExportHandler &exportHandler);
		std::unique_ptr<IAssetWrapper> ImportAsset(Type type,VFilePtr f,const std::optional<std::string> &filePath={},std::string *optOutErr=nullptr) const;
		bool ExportAsset(Type type,VFilePtrReal f,const IAssetWrapper &assetWrapper,std::string *optOutErr=nullptr) const;

		uint32_t GetImporterCount(Type type) const {return m_importers[umath::to_integral(type)].size();}
		uint32_t GetExporterCount(Type type) const {return m_exporters[umath::to_integral(type)].size();}
		const ImporterInfo *GetImporterInfo(Type type,uint32_t idx) const
		{
			auto &importers = m_importers[umath::to_integral(type)];
			return (idx < importers.size()) ? &importers.at(idx).info : nullptr;
		}
		const ExporterInfo *GetExporterInfo(Type type,uint32_t idx) const
		{
			auto &exporters = m_exporters[umath::to_integral(type)];
			return (idx < exporters.size()) ? &exporters.at(idx).info : nullptr;
		}
	private:
		struct DLLNETWORK Importer
		{
			ImporterInfo info;
			ImportHandler handler;
		};
		struct DLLNETWORK Exporter
		{
			ExporterInfo info;
			ExportHandler handler;
		};
		std::array<std::vector<Importer>,umath::to_integral(Type::Count)> m_importers;
		std::array<std::vector<Exporter>,umath::to_integral(Type::Count)> m_exporters;
	};
};

#endif
