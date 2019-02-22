#ifndef __MODEL_FLEXES_HPP__
#define __MODEL_FLEXES_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <vector>

#pragma pack(push,1)
struct DLLNETWORK FlexController
{
	std::string name = {};
	float min = 0.f;
	float max = 0.f;
};
#pragma pack(pop)

class VertexAnimation;
class MeshVertexAnimation;
class MeshVertexFrame;
class DLLNETWORK Flex
{
public:
	Flex(const std::string &name);
	Flex(const Flex &other)=default;
	struct DLLNETWORK Operation
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
		Operation()=default;
		Operation(Type type,float value);
		Operation(Type type,int32_t index);
		Type type = Type::None;
		union 
		{
			int32_t index;
			float value;
		} d;
	};
	const std::string &GetName() const;
	const std::vector<Operation> &GetOperations() const;
	std::vector<Operation> &GetOperations();

	VertexAnimation *GetVertexAnimation() const;
	MeshVertexAnimation *GetMeshVertexAnimation() const;
	MeshVertexFrame *GetMeshVertexFrame() const;
	void SetVertexAnimation(VertexAnimation &anim,MeshVertexAnimation &meshAnim,MeshVertexFrame &meshFrame);
private:
	std::vector<Operation> m_operations;
	std::string m_name;
	mutable std::weak_ptr<VertexAnimation> m_vertexAnim = {};
	mutable std::weak_ptr<MeshVertexAnimation> m_meshVertexAnim = {};
	mutable std::weak_ptr<MeshVertexFrame> m_meshFrame = {};
};

#endif
