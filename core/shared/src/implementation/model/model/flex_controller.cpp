// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model;

std::vector<pragma::animation::FlexController>::const_iterator pragma::asset::Model::FindFlexController(const std::string &name) const { return const_cast<Model *>(this)->FindFlexController(name); }
std::vector<pragma::animation::FlexController>::iterator pragma::asset::Model::FindFlexController(const std::string &name)
{
	return std::find_if(m_flexControllers.begin(), m_flexControllers.end(), [&name](const animation::FlexController &fc) { return string::compare(fc.name, name, false); });
}
bool pragma::asset::Model::GetFlexControllerId(const std::string &name, uint32_t &id) const
{
	auto it = FindFlexController(name);
	if(it == m_flexControllers.end())
		return false;
	id = it - m_flexControllers.begin();
	return true;
}
const std::vector<pragma::animation::FlexController> &pragma::asset::Model::GetFlexControllers() const { return const_cast<Model *>(this)->GetFlexControllers(); }
std::vector<pragma::animation::FlexController> &pragma::asset::Model::GetFlexControllers() { return m_flexControllers; }
const pragma::animation::FlexController *pragma::asset::Model::GetFlexController(uint32_t id) const { return const_cast<Model *>(this)->GetFlexController(id); }
pragma::animation::FlexController *pragma::asset::Model::GetFlexController(uint32_t id) { return (id < m_flexControllers.size()) ? &m_flexControllers.at(id) : nullptr; }
const pragma::animation::FlexController *pragma::asset::Model::GetFlexController(const std::string &name) const { return const_cast<Model *>(this)->GetFlexController(name); }
pragma::animation::FlexController *pragma::asset::Model::GetFlexController(const std::string &name)
{
	auto it = FindFlexController(name);
	return (it != m_flexControllers.end()) ? &(*it) : nullptr;
}
pragma::animation::FlexController &pragma::asset::Model::AddFlexController(const std::string &name)
{
	if(name.find_first_of("/\\") != std::string::npos)
		throw std::invalid_argument {"Name '" + name + "' contains illegal characters!"};
	auto it = FindFlexController(name);
	if(it == m_flexControllers.end()) {
		m_flexControllers.push_back({});
		m_flexControllers.back().name = name;
		it = m_flexControllers.end() - 1;
	}
	return *it;
}
void pragma::asset::Model::RemoveFlexController(uint32_t id)
{
	if(id >= m_flexControllers.size())
		return;
	m_flexControllers.erase(m_flexControllers.begin() + id);
}
void pragma::asset::Model::RemoveFlexController(const std::string &name)
{
	auto it = FindFlexController(name);
	if(it == m_flexControllers.end())
		return;
	m_flexControllers.erase(it);
}
uint32_t pragma::asset::Model::GetFlexControllerCount() const { return m_flexControllers.size(); }
const std::string *pragma::asset::Model::GetFlexControllerName(uint32_t id) const
{
	if(id >= m_flexControllers.size())
		return nullptr;
	return &m_flexControllers.at(id).name;
}

//////////////

void pragma::asset::Model::SetMaxEyeDeflection(math::Degree eyeDeflection) { m_maxEyeDeflection = eyeDeflection; }
pragma::math::Degree pragma::asset::Model::GetMaxEyeDeflection() const { return m_maxEyeDeflection; }

std::vector<pragma::animation::Flex>::const_iterator pragma::asset::Model::FindFlex(const std::string &name) const { return const_cast<Model *>(this)->FindFlex(name); }
std::vector<pragma::animation::Flex>::iterator pragma::asset::Model::FindFlex(const std::string &name)
{
	return std::find_if(m_flexes.begin(), m_flexes.end(), [&name](const animation::Flex &flex) { return string::compare(flex.GetName(), name, false); });
}
bool pragma::asset::Model::GetFlexId(const std::string &name, uint32_t &id) const
{
	auto it = FindFlex(name);
	if(it == m_flexes.end())
		return false;
	id = it - m_flexes.begin();
	return true;
}
const std::vector<pragma::animation::Flex> &pragma::asset::Model::GetFlexes() const { return const_cast<Model *>(this)->GetFlexes(); }
std::vector<pragma::animation::Flex> &pragma::asset::Model::GetFlexes() { return m_flexes; }
const pragma::animation::Flex *pragma::asset::Model::GetFlex(uint32_t id) const { return const_cast<Model *>(this)->GetFlex(id); }
pragma::animation::Flex *pragma::asset::Model::GetFlex(uint32_t id) { return (id < m_flexes.size()) ? &m_flexes.at(id) : nullptr; }
const pragma::animation::Flex *pragma::asset::Model::GetFlex(const std::string &name) const { return const_cast<Model *>(this)->GetFlex(name); }
pragma::animation::Flex *pragma::asset::Model::GetFlex(const std::string &name)
{
	auto it = FindFlex(name);
	return (it != m_flexes.end()) ? &(*it) : nullptr;
}
pragma::animation::Flex &pragma::asset::Model::AddFlex(const std::string &name)
{
	if(name.find_first_of("/\\") != std::string::npos)
		throw std::invalid_argument {"Name '" + name + "' contains illegal characters!"};
	auto it = FindFlex(name);
	if(it == m_flexes.end()) {
		m_flexes.push_back({name});
		it = m_flexes.end() - 1;
	}
	return *it;
}
void pragma::asset::Model::RemoveFlex(uint32_t id)
{
	if(id >= m_flexes.size())
		return;
	m_flexes.erase(m_flexes.begin() + id);
}
void pragma::asset::Model::RemoveFlex(const std::string &name)
{
	auto it = FindFlex(name);
	if(it == m_flexes.end())
		return;
	m_flexes.erase(it);
}
uint32_t pragma::asset::Model::GetFlexCount() const { return m_flexes.size(); }
const std::string *pragma::asset::Model::GetFlexName(uint32_t id) const
{
	if(id >= m_flexes.size())
		return nullptr;
	return &m_flexes.at(id).GetName();
}
bool pragma::asset::Model::GetFlexFormula(uint32_t id, std::string &formula) const
{
	auto *flex = GetFlex(id);
	if(flex == nullptr)
		return false;
	struct OpExpression {
		OpExpression(const std::string &e = "", uint32_t p = 0u) : expression(e), precedence(p) {}
		std::string expression;
		uint32_t precedence = 0u;
	};
	std::stack<OpExpression> opStack {};
	for(auto &op : flex->GetOperations()) {
		switch(op.type) {
		case animation::Flex::Operation::Type::None:
			break;
		case animation::Flex::Operation::Type::Const:
			opStack.push({std::to_string(op.d.value), 10u});
			break;
		case animation::Flex::Operation::Type::Fetch:
			{
				auto *flexCon = GetFlexController(op.d.index);
				opStack.push({(flexCon != nullptr) ? flexCon->name : "invalid_flex_controller", 10u});
				break;
			}
		case animation::Flex::Operation::Type::Fetch2:
			{
				auto *flex = GetFlex(op.d.index);
				opStack.push({"%" + ((flex != nullptr) ? flex->GetName() : "invalid_flex"), 10u});
				break;
			}
		case animation::Flex::Operation::Type::Add:
			{
				auto r = opStack.top();
				opStack.pop();
				auto l = opStack.top();
				opStack.pop();

				opStack.push({l.expression + " + " + r.expression, 1u});
				break;
			}
		case animation::Flex::Operation::Type::Sub:
			{
				auto r = opStack.top();
				opStack.pop();
				auto l = opStack.top();
				opStack.pop();

				opStack.push({l.expression + " - " + r.expression, 1u});
				break;
			}
		case animation::Flex::Operation::Type::Mul:
			{
				auto r = opStack.top();
				opStack.pop();
				auto rExpr = std::string {};
				if(r.precedence < 2)
					rExpr = "(" + r.expression + ")";
				else
					rExpr = r.expression;

				auto l = opStack.top();
				opStack.pop();
				auto lExpr = std::string {};
				if(l.precedence < 2)
					lExpr = "(" + l.expression + ")";
				else
					lExpr = l.expression;

				opStack.push({lExpr + " * " + rExpr, 2});
				break;
			}
		case animation::Flex::Operation::Type::Div:
			{
				auto r = opStack.top();
				opStack.pop();
				auto rExpr = std::string {};
				if(r.precedence < 2)
					rExpr = "(" + r.expression + ")";
				else
					rExpr = r.expression;

				auto l = opStack.top();
				opStack.pop();
				auto lExpr = std::string {};
				if(l.precedence < 2)
					lExpr = "(" + l.expression + ")";
				else
					lExpr = l.expression;

				opStack.push({lExpr + " / " + rExpr, 2});
				break;
			}
		case animation::Flex::Operation::Type::Neg:
			{
				auto r = opStack.top();
				opStack.pop();
				opStack.push({"-" + r.expression, 10});
				break;
			}
		case animation::Flex::Operation::Type::Exp:
			break;
		case animation::Flex::Operation::Type::Open:
			break;
		case animation::Flex::Operation::Type::Close:
			break;
		case animation::Flex::Operation::Type::Comma:
			break;
		case animation::Flex::Operation::Type::Max:
			{
				auto r = opStack.top();
				opStack.pop();
				auto rExpr = std::string {};
				if(r.precedence < 5)
					rExpr = "(" + r.expression + ")";
				else
					rExpr = r.expression;

				auto l = opStack.top();
				opStack.pop();
				auto lExpr = std::string {};
				if(l.precedence < 5)
					lExpr = "(" + l.expression + ")";
				else
					lExpr = l.expression;

				opStack.push({" max(" + lExpr + ", " + rExpr + ")", 5});
				break;
			}
		case animation::Flex::Operation::Type::Min:
			{
				auto r = opStack.top();
				opStack.pop();
				auto rExpr = std::string {};
				if(r.precedence < 5)
					rExpr = "(" + r.expression + ")";
				else
					rExpr = r.expression;

				auto l = opStack.top();
				opStack.pop();
				auto lExpr = std::string {};
				if(l.precedence < 5)
					lExpr = "(" + l.expression + ")";
				else
					lExpr = l.expression;

				opStack.push({" min(" + lExpr + ", " + rExpr + ")", 5});
				break;
			}
		case animation::Flex::Operation::Type::TwoWay0:
			{
				auto *flexCon = GetFlexController(op.d.index);
				opStack.push({" (1 - (min(max(" + ((flexCon != nullptr) ? flexCon->name : "invalid_flex_controller") + " + 1, 0), 1)))", 5});
				break;
			}
		case animation::Flex::Operation::Type::TwoWay1:
			{
				auto *flexCon = GetFlexController(op.d.index);
				opStack.push({" (min(max(" + ((flexCon != nullptr) ? flexCon->name : "invalid_flex_controller") + ", 0), 1))", 5});
				break;
			}
		case animation::Flex::Operation::Type::NWay:
			{
				auto *flexCon = GetFlexController(op.d.index);
				auto v = (flexCon != nullptr) ? flexCon->name : "invalid_flex_controller";

				auto &valueControllerIndex = opStack.top();
				opStack.pop();
				auto *flValueController = GetFlexController(string::to_int(valueControllerIndex.expression));
				auto flValue = (flValueController != nullptr) ? flValueController->name : "invalid_flex_controller";

				auto &filterRampW = opStack.top();
				opStack.pop();
				auto &filterRampZ = opStack.top();
				opStack.pop();
				auto &filterRampY = opStack.top();
				opStack.pop();
				auto &filterRampX = opStack.top();
				opStack.pop();

				auto greaterThanX = "min(1, (-min(0, (" + filterRampX.expression + " - " + flValue + "))))";
				auto lessThanY = "min(1, (-min(0, (" + flValue + " - " + filterRampY.expression + "))))";
				auto remapX = "min(max((" + flValue + " - " + filterRampX.expression + ") / (" + filterRampY.expression + " - " + filterRampX.expression + "), 0), 1)";
				auto greaterThanEqualY = "-(min(1, (-min(0, (" + flValue + " - " + filterRampY.expression + ")))) - 1)";
				auto lessThanEqualZ = "-(min(1, (-min(0, (" + filterRampZ.expression + " - " + flValue + ")))) - 1)";
				auto greaterThanZ = "min(1, (-min(0, (" + filterRampZ.expression + " - " + flValue + "))))";
				auto lessThanW = "min(1, (-min(0, (" + flValue + " - " + filterRampW.expression + "))))";
				auto remapZ = "(1 - (min(max((" + flValue + " - " + filterRampZ.expression + ") / (" + filterRampW.expression + " - " + filterRampZ.expression + "), 0), 1)))";

				auto expValue = "((" + greaterThanX + " * " + lessThanY + ") * " + remapX + ") + (" + greaterThanEqualY + " * " + lessThanEqualZ + ") + ((" + greaterThanZ + " * " + lessThanW + ") * " + remapZ + ")";

				opStack.push({"((" + expValue + ") * (" + v + "))", 5});
				break;
			}
		case animation::Flex::Operation::Type::Combo:
			{
				auto count = op.d.index;
				auto newExpr = OpExpression {};
				auto intExpr = opStack.top();
				opStack.pop();
				newExpr.expression += intExpr.expression;
				for(auto i = decltype(count) {1}; i < count; ++i) {
					intExpr = opStack.top();
					opStack.pop();
					newExpr.expression += " * " + intExpr.expression;
				}
				newExpr.expression = "(" + newExpr.expression + ")";
				newExpr.precedence = 5u;
				opStack.push(newExpr);
				break;
			}
		case animation::Flex::Operation::Type::Dominate:
			{
				auto count = op.d.index;
				auto newExpr = OpExpression {};
				auto intExpr = opStack.top();
				opStack.pop();
				newExpr.expression += intExpr.expression;
				for(auto i = decltype(count) {1}; i < count; ++i) {
					intExpr = opStack.top();
					opStack.pop();
					newExpr.expression += " * " + intExpr.expression;
				}
				intExpr = opStack.top();
				opStack.pop();
				newExpr.expression = intExpr.expression + " * (1 - " + newExpr.expression + ")";
				newExpr.expression = "(" + newExpr.expression + ")";
				newExpr.precedence = 5u;
				opStack.push(newExpr);
				break;
			}
		case animation::Flex::Operation::Type::DMELowerEyelid:
			{
				auto *pCloseLidVController = GetFlexController(op.d.index);
				auto pCloseLidV = (pCloseLidVController != nullptr) ? pCloseLidVController->name : "invalid_flex_controller";
				auto flCloseLidVMin = std::to_string((pCloseLidVController != nullptr) ? pCloseLidVController->min : 0.f);
				auto flCloseLidVMax = std::to_string((pCloseLidVController != nullptr) ? pCloseLidVController->max : 0.f);
				auto flCloseLidV = "(min(max((" + pCloseLidV + " - " + flCloseLidVMin + ") / (" + flCloseLidVMax + " - " + flCloseLidVMin + "), 0), 1))";

				auto closeLidIndex = opStack.top();
				opStack.pop();

				auto *pCloseLidController = GetFlexController(string::to_int(closeLidIndex.expression));
				auto pCloseLid = (pCloseLidController != nullptr) ? pCloseLidController->name : "invalid_flex_controller";
				auto flCloseLidMin = std::to_string((pCloseLidController != nullptr) ? pCloseLidController->min : 0.f);
				auto flCloseLidMax = std::to_string((pCloseLidController != nullptr) ? pCloseLidController->max : 0.f);
				auto flCloseLid = "(min(max((" + pCloseLid + " - " + flCloseLidMin + ") / (" + flCloseLidMax + " - " + flCloseLidMin + "), 0), 1))";

				opStack.pop();

				auto eyeUpDownIndex = opStack.top();
				opStack.pop();
				auto *pEyeUpDownController = GetFlexController(string::to_int(eyeUpDownIndex.expression));
				auto pEyeUpDown = (pEyeUpDownController != nullptr) ? pEyeUpDownController->name : "invalid_flex_controller";
				auto flEyeUpDownMin = std::to_string((pEyeUpDownController != nullptr) ? pEyeUpDownController->min : 0.f);
				auto flEyeUpDownMax = std::to_string((pEyeUpDownController != nullptr) ? pEyeUpDownController->max : 0.f);
				auto flEyeUpDown = "(-1 + 2 * (min(max((" + pEyeUpDown + " - " + flEyeUpDownMin + ") / (" + flEyeUpDownMax + " - " + flEyeUpDownMin + "), 0), 1)))";

				opStack.push({"(min(1, (1 - " + flEyeUpDown + ")) * (1 - " + flCloseLidV + ") * " + flCloseLid + ")", 5});
				break;
			}
		case animation::Flex::Operation::Type::DMEUpperEyelid:
			{
				auto *pCloseLidVController = GetFlexController(op.d.index);
				auto pCloseLidV = (pCloseLidVController != nullptr) ? pCloseLidVController->name : "invalid_flex_controller";
				auto flCloseLidVMin = std::to_string((pCloseLidVController != nullptr) ? pCloseLidVController->min : 0.f);
				auto flCloseLidVMax = std::to_string((pCloseLidVController != nullptr) ? pCloseLidVController->max : 0.f);
				auto flCloseLidV = "(min(max((" + pCloseLidV + " - " + flCloseLidVMin + ") / (" + flCloseLidVMax + " - " + flCloseLidVMin + "), 0), 1))";

				auto closeLidIndex = opStack.top();
				opStack.pop();
				auto *pCloseLidController = GetFlexController(string::to_int(closeLidIndex.expression));
				auto pCloseLid = (pCloseLidController != nullptr) ? pCloseLidController->name : "invalid_flex_controller";
				auto flCloseLidMin = std::to_string((pCloseLidController != nullptr) ? pCloseLidController->min : 0.f);
				auto flCloseLidMax = std::to_string((pCloseLidController != nullptr) ? pCloseLidController->max : 0.f);
				auto flCloseLid = "(min(max((" + pCloseLid + " - " + flCloseLidMin + ") / (" + flCloseLidMax + " - " + flCloseLidMin + "), 0), 1))";

				opStack.pop();

				auto eyeUpDownIndex = opStack.top();
				opStack.pop();
				auto *pEyeUpDownController = GetFlexController(string::to_int(eyeUpDownIndex.expression));
				auto pEyeUpDown = (pEyeUpDownController != nullptr) ? pEyeUpDownController->name : "invalid_flex_controller";
				auto flEyeUpDownMin = std::to_string((pEyeUpDownController != nullptr) ? pEyeUpDownController->min : 0.f);
				auto flEyeUpDownMax = std::to_string((pEyeUpDownController != nullptr) ? pEyeUpDownController->max : 0.f);
				auto flEyeUpDown = "(-1 + 2 * (min(max((" + pEyeUpDown + " - " + flEyeUpDownMin + ") / (" + flEyeUpDownMax + " - " + flEyeUpDownMin + "), 0), 1)))";

				opStack.push({"(min(1, (1 + " + flEyeUpDown + ")) * " + flCloseLidV + " * " + flCloseLid + ")", 5});
				break;
			}
		}
	}
	if(opStack.size() == 1) {
		formula = opStack.top().expression;
		return true;
	}
	return false;
}
bool pragma::asset::Model::GetFlexFormula(const std::string &name, std::string &formula) const
{
	auto id = 0u;
	if(GetFlexId(name, id) == false)
		return false;
	return GetFlexFormula(id, formula);
}
