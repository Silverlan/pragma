// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.random;

template<typename TUniformDist, typename T>
static TUniformDist to_random_number(const std::string_view &s, const std::function<T(const std::string_view &)> &fToValue)
{
	auto t = std::string {s}; // TODO: Use string_view instead
	pragma::string::remove_whitespace(t);
	if(t.empty() == false && t.at(0) == '{') {
		auto posEnd = t.find('}');
		if(posEnd != std::string::npos) {
			auto strValues = t.substr(1ull, posEnd - 1ull);
			std::vector<std::string> values;
			pragma::string::explode(strValues, ",", values);
			if(values.empty() == false) {
				auto f0 = fToValue(values.at(0));
				auto f1 = f0;
				if(values.size() > 1)
					f1 = fToValue(values.at(1));
				return TUniformDist(f0, f1);
			}
		}
	}
	auto f = fToValue(s);
	return TUniformDist(f, f);
}
void pragma::math::to_random_float(const std::string &s, float &outF0, float &outF1)
{
	auto dis = to_random_float(s);
	outF0 = dis.min();
	outF1 = dis.max();
}
std::uniform_real_distribution<float> pragma::math::to_random_float(const std::string &s) { return to_random_number<std::uniform_real_distribution<float>, float>(s, static_cast<float (*)(const std::string_view &)>(util::to_float)); }
void pragma::math::to_random_int(const std::string &s, int32_t &outI0, int32_t &outI1)
{
	auto dis = to_random_int(s);
	outI0 = dis.min();
	outI1 = dis.max();
}
std::uniform_int_distribution<int32_t> pragma::math::to_random_int(const std::string &s) { return to_random_number<std::uniform_int_distribution<int32_t>, int32_t>(s, static_cast<int32_t (*)(const std::string_view &)>(util::to_int)); }
