// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.schedule.behavior;

export import :ai.schedule.behavior_parameter;
export import :ai.tasks.selector;

export namespace pragma {
	namespace ai {
		// Behavior Nodes
		class Schedule;
		class BehaviorParameter;
		class DLLSERVER BehaviorNode : public std::enable_shared_from_this<BehaviorNode>, public ParameterBase {
		  public:
			enum class DLLSERVER Result : uint32_t {
				Initial = std::numeric_limits<std::underlying_type_t<Result>>::max(),
				Pending = 0,
				Failed,
				Succeeded,
			};
			enum class DLLSERVER Type : uint32_t {
				Selector = 0, // One of the children has to succeed
				Sequence      // All children have to succeed
			};
			struct DLLSERVER DebugInfo {
				uint64_t executionIndex = 0ull;
				float lastStartTime = 0.f;
				float lastEndTime = 0.f;
				Result lastResult = Result::Initial;
				std::string debugName;
			};
		  protected:
			BehaviorNode(const BehaviorNode &other);
			mutable DebugInfo m_debugInfo;
			std::vector<std::shared_ptr<BehaviorNode>> m_childNodes;
			std::shared_ptr<TaskSelector> m_selector;
			bool m_bActive;
			Type m_type;
			std::vector<uint8_t> m_paramIds; // Parameter Id for schedule
			virtual void OnStopped();
			void StopTask(uint32_t taskId);
			Result StartTask(uint32_t taskId, const Schedule *sched, BaseAIComponent &ent);
			Result ThinkTask(uint32_t taskId, const Schedule *sched, BaseAIComponent &ent);
			virtual void OnTaskComplete(const Schedule *sched, uint32_t taskId, Result result);
			using ParameterBase::GetParameter;
			virtual void Clear();
		  public:
			BehaviorNode(Type type = Type::Sequence, SelectorType selectorType = SelectorType::Sequential);
			BehaviorNode(SelectorType selectorType);
			virtual ~BehaviorNode();
			virtual std::shared_ptr<BehaviorNode> Copy() const { return Copy<BehaviorNode>(); }
			BehaviorNode &operator=(const BehaviorNode &other) = delete;
			virtual void Stop();
			virtual Result Think(const Schedule *sched, BaseAIComponent &ent); // Returns true on complete (success or failure)
			void SetType(Type type);
			void SetSelectorType(SelectorType selectorType);
			SelectorType GetSelectorType() const;
			Type GetType() const;
			bool IsActive() const;
			void DebugPrint(const Schedule *sched, std::stringstream &ss, const std::string &t = "") const;
			void AddNode(const std::shared_ptr<BehaviorNode> &node);
			const std::vector<std::shared_ptr<BehaviorNode>> &GetNodes() const;
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent);
			virtual void SetScheduleParameter(uint8_t taskParamId, uint8_t scheduleParamId);
			const BehaviorParameter *GetParameter(const Schedule *sched, uint8_t taskParamId) const;
			DebugInfo &GetDebugInfo() const;
			void SetDebugName(const std::string &name);
			virtual void Print(const Schedule *sched, std::ostream &o) const;
			template<class TBehaviorNode>
			std::shared_ptr<TBehaviorNode> Copy() const;
		};
		template<class TBehaviorNode>
		std::shared_ptr<TBehaviorNode> BehaviorNode::Copy() const
		{
			return std::shared_ptr<TBehaviorNode>(new TBehaviorNode(*static_cast<const TBehaviorNode *>(this)));
		}

		class DLLSERVER TaskManager {
		  protected:
			std::vector<std::function<std::shared_ptr<BehaviorNode>(void)>> m_taskFactories;
			std::vector<const std::type_info *> m_taskIds;
		  public:
			TaskManager() = default;
			TaskManager(const TaskManager &) = delete;
			TaskManager &operator=(const TaskManager &) = delete;
			uint32_t RegisterTask(const std::function<std::shared_ptr<BehaviorNode>(void)> &f);
			uint32_t RegisterTask(const std::type_info &type, const std::function<std::shared_ptr<BehaviorNode>(void)> &f);
			uint32_t RegisterTask(const BehaviorNode &node);
			std::shared_ptr<BehaviorNode> CreateTask(uint32_t taskId) const;
			uint32_t GetTaskId(const std::type_info &type) const;
		};
	};
};
