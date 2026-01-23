// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:networking.model_load_manager;

export import pragma.util;
export import pragma.shared;

export {
	class ModelLoadManager {
	  private:
		static std::shared_ptr<ModelLoadManager> s_manager;
	  private:
		struct ModelLoadQuery {
			NetPacket packet;
			std::shared_ptr<pragma::asset::Model> model;
			std::string fileName;
		};
		std::mutex m_loadMutex;
		std::mutex m_completeMutex;
		std::atomic<bool> m_bComplete = {false};
		std::queue<std::shared_ptr<ModelLoadQuery>> m_loadQueries;
		std::queue<std::shared_ptr<ModelLoadQuery>> m_completeQueries;
		std::thread m_thread;
		CallbackHandle m_hCbTick;
		CallbackHandle m_hCbOnGameEnd;
		void Update();
	  protected:
		ModelLoadManager();
	  public:
		~ModelLoadManager();
		static ModelLoadManager &Initialize();
		void AddQuery(const NetPacket &packet, const std::shared_ptr<pragma::asset::Model> &mdl, const std::string &fileName);
	};
}
