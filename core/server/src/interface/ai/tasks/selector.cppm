// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.selector;

export import std.compat;

export namespace pragma {
	namespace ai {
		// Selectors
		enum class SelectorType { Sequential = 0, RandomShuffle };
		class DLLSERVER TaskSelector {
		  public:
			virtual std::shared_ptr<TaskSelector> Copy() const = 0;
			virtual uint32_t SelectNextTask() = 0;
			virtual uint32_t GetCurrentTask() const = 0;
			virtual SelectorType GetType() const = 0;
			virtual void Reset(uint32_t taskCount);
		};
		class DLLSERVER TaskSelectorSequential : public TaskSelector {
		  protected:
			uint32_t m_currentTask;
		  public:
			TaskSelectorSequential();
			virtual std::shared_ptr<TaskSelector> Copy() const override;
			virtual uint32_t SelectNextTask() override;
			virtual uint32_t GetCurrentTask() const override;
			virtual void Reset(uint32_t taskCount) override;
			virtual SelectorType GetType() const override;
		};
		class DLLSERVER TaskSelectorRandomShuffle : public TaskSelector {
		  protected:
			uint32_t m_taskOffset;
			std::vector<uint32_t> m_tasks;
		  public:
			TaskSelectorRandomShuffle();
			virtual std::shared_ptr<TaskSelector> Copy() const override;
			virtual uint32_t SelectNextTask() override;
			virtual void Reset(uint32_t taskCount) override;
			virtual uint32_t GetCurrentTask() const override;
			virtual SelectorType GetType() const override;
		};
	};
};
