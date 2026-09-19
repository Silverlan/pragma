// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <glaze/glaze.hpp>

export module pragma.client:gui.json_skin;

export import pragma.lua;
export import pragma.gui;

export namespace pragma::gui {
	struct DLLCLIENT JsonSkinClass {
		JsonSkinClass() = default;
		JsonSkinClass(const JsonSkinClass &) = delete;
		JsonSkinClass &operator=(const JsonSkinClass &) = delete;

		string::StringMap<glz::json_t> properties;
		string::StringMap<std::unique_ptr<JsonSkinClass>> children;
		string::StringMap<std::unique_ptr<JsonSkinClass>> states;
		string::StringMap<std::unique_ptr<JsonSkinClass>> decorators;
		std::vector<std::string> inherits;

		JsonSkinClass *Copy() const;

		void MergeFrom(const JsonSkinClass &baseClass);
	};

	class DLLCLIENT JsonSkin : public WISkin {
	  public:
		struct Settings {
			JsonSkin *base = nullptr;
			glz::json_t jsonData;
		};
	  protected:
		JsonSkinClass m_rootClass;
		string::StringMap<std::string> m_constants;

		void ParseConstants(const glz::json_t &j);
		void ParseFonts(const glz::json_t &j);
		void ParseClass(const glz::json_t &j, JsonSkinClass &outClass);

		void ResolveVariables(JsonSkinClass &cl);
		void ResolveMixins(JsonSkinClass &target, const JsonSkinClass &root);
		void MergeBaseSkin(const JsonSkinClass &baseRoot, JsonSkinClass &cl);
	  public:
		JsonSkin();

		void Load(const Settings &settings);

		void Initialize(types::WIBase *el) override;
		void Release(types::WIBase *el) override;
	};
};
