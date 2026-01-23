// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.schedule;

import :ai.schedule.behavior;
import :entities.components;
import :game;

using namespace pragma;

ai::BehaviorNode::BehaviorNode(Type type, SelectorType selectorType) : std::enable_shared_from_this<BehaviorNode>(), m_bActive(false), m_type(type) { SetSelectorType(selectorType); }
ai::BehaviorNode::BehaviorNode(SelectorType selectorType) : BehaviorNode(Type::Sequence, selectorType) {}
ai::BehaviorNode::~BehaviorNode()
{
	Stop();
	Clear();
}
void ai::BehaviorNode::Clear()
{
	for(auto &node : m_childNodes)
		node->Clear();
}
void ai::BehaviorNode::SetType(Type type) { m_type = type; }
void ai::BehaviorNode::SetSelectorType(SelectorType selectorType)
{
	switch(selectorType) {
	case SelectorType::RandomShuffle:
		m_selector = pragma::util::make_shared<TaskSelectorRandomShuffle>();
		break;
	default:
		m_selector = pragma::util::make_shared<TaskSelectorSequential>();
		break;
	}
}
ai::SelectorType ai::BehaviorNode::GetSelectorType() const
{
	if(m_selector == nullptr)
		SelectorType::Sequential;
	return m_selector->GetType();
}
bool ai::BehaviorNode::IsActive() const { return m_bActive; }
void ai::BehaviorNode::Stop()
{
	if(IsActive() == false)
		return;
	m_debugInfo.lastEndTime = SGame::Get()->CurTime();
	m_bActive = false;
	for(auto &child : m_childNodes) {
		if(child->IsActive() == true)
			child->Stop();
	}
	OnStopped();
}
void ai::BehaviorNode::OnStopped() {}
void ai::BehaviorNode::OnTaskComplete(const Schedule *sched, uint32_t taskId, Result result) {}
void ai::BehaviorNode::AddNode(const std::shared_ptr<BehaviorNode> &node) { m_childNodes.push_back(node); }
ai::BehaviorNode::BehaviorNode(const BehaviorNode &other) : ParameterBase(other)
{
	m_selector = std::shared_ptr<TaskSelector>(other.m_selector->Copy());
	m_childNodes.reserve(other.m_childNodes.size());
	for(auto &child : other.m_childNodes)
		m_childNodes.push_back(std::shared_ptr<BehaviorNode>(child->Copy()));
	m_paramIds = other.m_paramIds;
	m_bActive = false;
	m_type = other.m_type;
}
const ai::Schedule::Parameter *ai::BehaviorNode::GetParameter(const Schedule *sched, uint8_t taskParamId) const
{
	auto *param = GetParameter(taskParamId);
	if(param != nullptr)
		return param;
	return sched->GetParameter((taskParamId < m_paramIds.size()) ? m_paramIds[taskParamId] : -1);
}
void ai::BehaviorNode::SetScheduleParameter(uint8_t taskParamId, uint8_t scheduleParamId)
{
	if(taskParamId >= m_paramIds.size())
		m_paramIds.resize(taskParamId + 1, std::numeric_limits<uint8_t>::max());
	m_paramIds[taskParamId] = scheduleParamId;
}
ai::BehaviorNode::Result ai::BehaviorNode::StartTask(uint32_t taskId, const Schedule *sched, BaseAIComponent &ent)
{
	if(taskId >= m_childNodes.size())
		return Result::Succeeded;
	auto &taskChild = m_childNodes[taskId];
	auto r = taskChild->Start(sched, ent);
	taskChild->m_debugInfo.lastResult = r;
	return r;
}
ai::BehaviorNode::Result ai::BehaviorNode::ThinkTask(uint32_t taskId, const Schedule *sched, BaseAIComponent &ent)
{
	if(taskId >= m_childNodes.size())
		return Result::Succeeded;
	auto &taskChild = m_childNodes[taskId];
	auto r = taskChild->Think(sched, ent);
	taskChild->m_debugInfo.lastResult = r;
	return r;
}
void ai::BehaviorNode::DebugPrint(const Schedule *sched, std::stringstream &ss, const std::string &t) const
{
	auto &taskManager = SGame::Get()->GetAITaskManager();
	const auto &type = typeid(*this);
	auto id = taskManager.GetTaskId(type);
	ss << t << "[";
	Print(sched, ss);
	ss << "] " << ((IsActive() == true) ? "Pending" : "Inactive") << " " << id << "\n";
	for(auto &node : m_childNodes)
		node->DebugPrint(sched, ss, t + "\t");
}
ai::BehaviorNode::Type ai::BehaviorNode::GetType() const { return m_type; }
ai::BehaviorNode::Result ai::BehaviorNode::Start(const Schedule *sched, BaseAIComponent &ent)
{
	m_selector->Reset(static_cast<uint32_t>(m_childNodes.size()));
	m_debugInfo.lastStartTime = SGame::Get()->CurTime();

	static auto executionIndex = 0ull;
	m_debugInfo.executionIndex = executionIndex++; // Used to determine which task was executed last (for debugging purposes only!)

	m_bActive = true;

	auto r = Result::Succeeded;
	auto currentTask = m_selector->GetCurrentTask();
	auto chk = (GetType() == Type::Selector) ? Result::Failed : Result::Succeeded;
	auto numChildren = m_childNodes.size();
	while(currentTask < numChildren && (r = StartTask(currentTask, sched, ent)) == chk) {
		OnTaskComplete(sched, currentTask, r);
		StopTask(currentTask);
		currentTask = m_selector->SelectNextTask();
		//StartTask(currentTask,sched,ent);
	}
	if(r != Result::Pending && currentTask < numChildren) {
		OnTaskComplete(sched, currentTask, r);
		StopTask(currentTask);
	}
	return r;
	//return StartTask(m_selector->GetCurrentTask(),sched,ent);
}
ai::BehaviorNode::Result ai::BehaviorNode::Think(const Schedule *sched, BaseAIComponent &ent)
{
	auto r = Result::Succeeded;
	auto currentTask = m_selector->GetCurrentTask();
	auto chk = (GetType() == Type::Selector) ? Result::Failed : Result::Succeeded;

	// fEndTask ends the current task and starts the next one
	const auto fStartNextTask = [this, sched, &ent](bool &bTaskStarted, uint32_t &currentTask, Result r) {
		OnTaskComplete(sched, currentTask, r);
		StopTask(currentTask);
		currentTask = m_selector->SelectNextTask();
		bTaskStarted = false;
	};
	auto bTaskStarted = true;
	auto numChildren = m_childNodes.size();
	while(currentTask < numChildren) {
		if(bTaskStarted == false) // Child-task hasn't been started yet
		{
			bTaskStarted = true;
			r = StartTask(currentTask, sched, ent);
			if(r == chk) // We can end the task immediately and go to the next one (No need to call Think)
			{
				fStartNextTask(bTaskStarted, currentTask, r);
				continue;
			}
			else if(r != Result::Pending) // Error state; Don't continue
				break;
		}
		if((r = ThinkTask(currentTask, sched, ent)) != chk)
			break;
		fStartNextTask(bTaskStarted, currentTask, r);
	}

	// Deprecated (Old version of above code)
	/*while(currentTask < m_childNodes.size() && (r = ThinkTask(currentTask,sched,ent)) == chk)
	{
		OnTaskComplete(sched,currentTask,r);
		StopTask(currentTask);
		currentTask = m_selector->SelectNextTask();
		StartTask(currentTask,sched,ent);
	}*/

	if(r != Result::Pending && currentTask < numChildren) {
		OnTaskComplete(sched, currentTask, r);
		StopTask(currentTask);
	}
	return r;
}
void ai::BehaviorNode::StopTask(uint32_t taskId)
{
	if(taskId >= m_childNodes.size())
		return;
	m_childNodes[taskId]->Stop();
}
void ai::BehaviorNode::Print(const Schedule *sched, std::ostream &o) const { o << typeid(*this).name(); }
const std::vector<std::shared_ptr<ai::BehaviorNode>> &ai::BehaviorNode::GetNodes() const { return m_childNodes; }
ai::BehaviorNode::DebugInfo &ai::BehaviorNode::GetDebugInfo() const { return m_debugInfo; }
void ai::BehaviorNode::SetDebugName(const std::string &name) { m_debugInfo.debugName = name; }

/////////////////////////////

uint32_t ai::TaskManager::RegisterTask(const std::function<std::shared_ptr<BehaviorNode>(void)> &f)
{
	m_taskFactories.push_back(f);
	return static_cast<uint32_t>(m_taskFactories.size() - 1);
}

uint32_t ai::TaskManager::RegisterTask(const std::type_info &type, const std::function<std::shared_ptr<BehaviorNode>(void)> &f)
{
	auto taskId = RegisterTask(f);
	if(taskId >= m_taskIds.size())
		m_taskIds.resize(taskId + 1);
	m_taskIds[taskId] = &type;
	return taskId;
}

uint32_t ai::TaskManager::RegisterTask(const BehaviorNode &node)
{
	auto ptr = node.shared_from_this();
	auto taskId = RegisterTask([ptr]() { return ptr->Copy(); });
	if(taskId >= m_taskIds.size())
		m_taskIds.resize(taskId + 1);
	m_taskIds[taskId] = nullptr;
	return taskId;
}

uint32_t ai::TaskManager::GetTaskId(const std::type_info &type) const
{
	auto it = std::find(m_taskIds.begin(), m_taskIds.end(), &type);
	if(it == m_taskIds.end())
		return std::numeric_limits<uint32_t>::max();
	return it - m_taskIds.begin();
}

std::shared_ptr<ai::BehaviorNode> ai::TaskManager::CreateTask(uint32_t taskId) const
{
	if(taskId >= m_taskFactories.size())
		return nullptr;
	return m_taskFactories[taskId]();
}
