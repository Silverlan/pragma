#include "stdafx_server.h"
#include "pragma/ai/ai_task_decorator.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/debug/debugoverlay.h"
#include <pragma/model/animation/activities.h>

using namespace pragma;

extern DLLSERVER SGame *s_game;

void ai::TaskDecorator::Initialize(const Schedule *sched)
{
	if(m_bInitialized == true)
		return;
	m_bInitialized = true;

	auto *paramType = GetParameter(sched,umath::to_integral(Parameter::DecoratorType));
	if(paramType != nullptr)
		m_decoratorType = static_cast<DecoratorType>(paramType->GetInt());
	if(m_decoratorType == DecoratorType::Limit || m_decoratorType == DecoratorType::Repeat)
	{
		auto *paramLimit = GetParameter(sched,umath::to_integral(Parameter::Limit));
		if(paramLimit != nullptr)
			m_limit = paramLimit->GetInt();
	}
}

void ai::TaskDecorator::SetDecoratorType(DecoratorType decoratorType)
{
	m_decoratorType = decoratorType;
}
void ai::TaskDecorator::SetLimit(uint32_t limit)
{
	m_limit = limit;
}

void ai::TaskDecorator::OnParameterChanged(uint8_t paramId)
{
	BehaviorNode::OnParameterChanged(paramId);
	if(paramId == 0)
	{
		auto *param = GetParameter(paramId);
		m_decoratorType = static_cast<DecoratorType>(param->GetInt());
	}
}

ai::BehaviorNode::Result ai::TaskDecorator::Start(const Schedule *sched,pragma::SAIComponent &ent)
{
	m_bRestartTaskOnThink = false;
	Initialize(sched);
	if(m_decoratorType == DecoratorType::Limit || m_decoratorType == DecoratorType::Repeat)
	{
		if(m_limit != -1 && m_count++ >= m_limit)
			return Result::Succeeded;
	}
	auto r = BehaviorNode::Start(sched,ent);
	switch(m_decoratorType)
	{
		case DecoratorType::Inherit:
			return r;
		case DecoratorType::AlwaysFail:
			return Result::Failed;
		case DecoratorType::AlwaysSucceed:
			return Result::Succeeded;
		case DecoratorType::Invert:
			return (r == Result::Pending) ? r : ((r == Result::Succeeded) ? Result::Failed : Result::Succeeded);
		case DecoratorType::UntilFail:
		case DecoratorType::UntilSuccess:
		case DecoratorType::Repeat:
		{
			if(r != Result::Pending)
			{
				if((m_decoratorType == DecoratorType::UntilFail && r == Result::Failed) || (m_decoratorType == DecoratorType::UntilSuccess && r == Result::Succeeded))
					return Result::Succeeded;
				m_bRestartTaskOnThink = true; // Special case; We'll want to make sure :Start is called instead of :Think in the next tick
				return Result::Pending;
			}
		}
	}
	return r;
}

void ai::TaskDecorator::Stop()
{
	BehaviorNode::Stop();
	if(m_decoratorType == DecoratorType::Repeat)
		m_count = 0;
}

ai::BehaviorNode::Result ai::TaskDecorator::Think(const Schedule *sched,pragma::SAIComponent &ent)
{
	if(m_bRestartTaskOnThink == true)
		return Start(sched,ent);
	auto r = BehaviorNode::Think(sched,ent);
	switch(m_decoratorType)
	{
		case DecoratorType::Inherit:
			return r;
		case DecoratorType::AlwaysFail:
			return Result::Failed;
		case DecoratorType::AlwaysSucceed:
			return Result::Succeeded;
		case DecoratorType::Invert:
			return (r == Result::Pending) ? r : ((r == Result::Succeeded) ? Result::Failed : Result::Succeeded);
		case DecoratorType::UntilFail:
		case DecoratorType::UntilSuccess:
		case DecoratorType::Repeat:
		{
			if(r != Result::Pending)
			{
				if((m_decoratorType == DecoratorType::UntilFail && r == Result::Failed) || (m_decoratorType == DecoratorType::UntilSuccess && r == Result::Succeeded))
					return Result::Succeeded;
				return Start(sched,ent);
				
			}
		}
	}
	return r;
}

void ai::TaskDecorator::Print(const Schedule *sched,std::ostream &o) const
{
	//BehaviorNode::Print(sched,o);
	//o<<" [type:";
	o<<"Deco[";
	switch(m_decoratorType)
	{
		case DecoratorType::Inherit:
			o<<"inherit";
			break;
		case DecoratorType::AlwaysFail:
			o<<"always fail";
			break;
		case DecoratorType::AlwaysSucceed:
			o<<"always succeed";
			break;
		case DecoratorType::Invert:
			o<<"invert";
			break;
		case DecoratorType::Limit:
			o<<"limit["<<m_count<<"/"<<m_limit<<"]";
			break;
		case DecoratorType::Repeat:
			o<<"repeat["<<m_count<<"]";
			break;
		case DecoratorType::UntilFail:
			o<<"until fail";
			break;
		case DecoratorType::UntilSuccess:
			o<<"until success";
			break;
	}
	o<<"]";
}
