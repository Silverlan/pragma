// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.decorator;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskDecorator : public BehaviorNode {
		  public:
			enum class DLLSERVER DecoratorType : uint32_t { Inherit = 0, AlwaysFail, AlwaysSucceed, Invert, Limit, Repeat, UntilFail, UntilSuccess };
		  protected:
			bool m_bRestartTaskOnThink = false;
			DecoratorType m_decoratorType = DecoratorType::Inherit;
			bool m_bInitialized = false;
			uint32_t m_count = 0;
			uint32_t m_limit = 0;
			virtual void OnParameterChanged(uint8_t paramId) override;
			void Initialize(const Schedule *sched);
		  public:
			enum class Parameter : uint32_t { DecoratorType = 0, Limit };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Think(const Schedule *sched, BaseAIComponent &ent) override;
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Stop() override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetDecoratorType(DecoratorType decoratorType);
			void SetLimit(uint32_t limit);
		};
	};
};
