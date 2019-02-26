#ifndef __MDL_FLEXRULE_H__
#define __MDL_FLEXRULE_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudioflexrule_t
		{
			int32_t flex;
			int32_t numops;
			int32_t opindex;
		};
		struct mstudioflexop_t
		{
			int32_t op;
			union 
			{
				int32_t index;
				float value;
			} d;
		};
		class FlexRule
		{
		public:
			FlexRule(const VFilePtr &f);
			int32_t GetFlexId() const;

			struct Operation
			{
				enum class Type : uint32_t
				{
					None = 0u,
					Const,
					Fetch,
					Fetch2,
					Add,
					Sub,
					Mul,
					Div,
					Neg, // not implemented
					Exp, // not implemented
					Open,
					Close,
					Comma,
					Max,
					Min,
					TwoWay0,
					TwoWay1,
					NWay,
					Combo,
					Dominate,
					DMELowerEyelid,
					DMEUpperEyelid
				};
				Type type = Type::None;
				union 
				{
					int32_t index;
					float value;
				} d;
			};
			const std::vector<Operation> &GetOperations() const;
		private:
			int32_t m_flexId = -1;
			std::vector<Operation> m_flexOperations = {};
		};
	};
};
#pragma pack(pop)

#endif
