// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:networking.resource_manager;

export import std;

#undef FindResource

export namespace pragma::networking {
	class DLLSERVER ResourceManager {
	  private:
		struct ResourceInfo {
			ResourceInfo(const std::string &fileName, bool stream);
			std::string fileName;
			bool stream;
		};
		static std::vector<ResourceInfo> m_resources;
	  public:
		static const std::vector<ResourceInfo> &GetResources();
		static bool AddResource(std::string res, bool stream = false);
		static unsigned int GetResourceCount();
		static bool IsValidResource(std::string res);
		static void ClearResources();
		static const ResourceInfo *FindResource(const std::string &fileName);
	};
}
