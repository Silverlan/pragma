// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:core.input_binding_layer;

export import :core.key_bind;
export import pragma.udm;

export struct DLLCLIENT InputBindingLayer {
	static bool Save(const std::vector<std::shared_ptr<InputBindingLayer>> &layers, udm::AssetDataArg outData, std::string &outErr);
	static bool Load(const udm::AssetData &data, std::vector<std::shared_ptr<InputBindingLayer>> &outLayers, std::string &outErr);

	std::string identifier;
	uint32_t priority = 0;
	bool enabled = true;
	void MapKey(short c, std::string cmd);
	void MapKey(short c, luabind::function<> function);
	void MapKey(short c, std::unordered_map<std::string, std::vector<std::string>> &binds);
	void AddKeyMapping(short c, std::string cmd);
	void RemoveKeyMapping(short c, std::string cmd);
	void UnmapKey(short c);
	void ClearLuaKeyMappings();
	virtual std::unordered_map<short, KeyBind> &GetKeyMappings();
	const std::unordered_map<short, KeyBind> &GetKeyMappings() const { return const_cast<InputBindingLayer *>(this)->GetKeyMappings(); }
	void ClearKeyMappings();
  private:
	std::unordered_map<short, KeyBind> m_keybinds;
};

export struct DLLCLIENT CoreInputBindingLayer : public InputBindingLayer {
	virtual std::unordered_map<short, KeyBind> &GetKeyMappings() override;
};
