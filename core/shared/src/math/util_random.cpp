#include "stdafx_shared.h"
#include "pragma/math/util_random.hpp"
#include <sharedutils/util.h>

template<typename TUniformDist,typename T>
	static TUniformDist to_random_number(const std::string &s,const std::function<T(const std::string&)> &fToValue)
{
	auto t = s;
	ustring::remove_whitespace(t);
	if(t.empty() == false && t.at(0) == '{')
	{
		auto posEnd = t.find('}');
		if(posEnd != std::string::npos)
		{
			auto strValues = t.substr(1ull,posEnd -1ull);
			std::vector<std::string> values;
			ustring::explode(strValues,",",values);
			if(values.empty() == false)
			{
				auto f0 = fToValue(values.at(0));
				auto f1 = f0;
				if(values.size() > 1)
					f1 = fToValue(values.at(1));
				return TUniformDist(f0,f1);
			}
		}
	}
	auto f = fToValue(s);
	return TUniformDist(f,f);
}
void util::to_random_float(const std::string &s,float &outF0,float &outF1)
{
	auto dis = to_random_float(s);
	outF0 = dis.min();
	outF1 = dis.max();
}
std::uniform_real_distribution<float> util::to_random_float(const std::string &s)
{
	return to_random_number<std::uniform_real_distribution<float>,float>(s,static_cast<float(*)(const std::string&)>(util::to_float));
}
void util::to_random_int(const std::string &s,int32_t &outI0,int32_t &outI1)
{
	auto dis = to_random_int(s);
	outI0 = dis.min();
	outI1 = dis.max();
}
std::uniform_int_distribution<int32_t> util::to_random_int(const std::string &s)
{
	return to_random_number<std::uniform_int_distribution<int32_t>,int32_t>(s,static_cast<int32_t(*)(const std::string&)>(util::to_int));
}
