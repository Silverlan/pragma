#include "mdl_flexrule.h"

import::mdl::FlexRule::FlexRule(const VFilePtr &f)
{
	auto offset = f->Tell();
	auto stdFlexRule = f->Read<mstudioflexrule_t>();
	m_flexId = stdFlexRule.flex;

	if(stdFlexRule.numops > 0 && stdFlexRule.opindex != 0)
	{
		f->Seek(offset +stdFlexRule.opindex);

		std::vector<mstudioflexop_t> stdFlexOps(stdFlexRule.numops);
		f->Read(stdFlexOps.data(),stdFlexOps.size() *sizeof(stdFlexOps.front()));

		m_flexOperations.reserve(stdFlexOps.size());
		for(auto &stdFlexOp : stdFlexOps)
		{
			m_flexOperations.push_back({});
			auto &op = m_flexOperations.back();
			op.type = static_cast<Operation::Type>(stdFlexOp.op);
			op.d.value = stdFlexOp.d.value;
		}
	}

	f->Seek(offset +sizeof(mstudioflexrule_t));
}

int32_t import::mdl::FlexRule::GetFlexId() const {return m_flexId;}
const std::vector<import::mdl::FlexRule::Operation> &import::mdl::FlexRule::GetOperations() const {return m_flexOperations;}

