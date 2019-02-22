#include "stdafx_shared.h"
#include "pragma/model/model_flexes.hpp"
#include "pragma/model/animation/vertex_animation.hpp"

Flex::Operation::Operation(Type t,float value)
	: type(t)
{
	d.value = value;
}
Flex::Operation::Operation(Type t,int32_t index)
	: type(t)
{
	d.index = index;
}
Flex::Flex(const std::string &name)
	: m_name(name)
{}
const std::string &Flex::GetName() const {return m_name;}
const std::vector<Flex::Operation> &Flex::GetOperations() const {return const_cast<Flex*>(this)->GetOperations();}
std::vector<Flex::Operation> &Flex::GetOperations() {return m_operations;}

VertexAnimation *Flex::GetVertexAnimation() const {return m_vertexAnim.lock().get();}
MeshVertexAnimation *Flex::GetMeshVertexAnimation() const {return m_meshVertexAnim.lock().get();}
MeshVertexFrame *Flex::GetMeshVertexFrame() const {return m_meshFrame.lock().get();}
void Flex::SetVertexAnimation(VertexAnimation &anim,MeshVertexAnimation &meshAnim,MeshVertexFrame &meshFrame)
{
	m_vertexAnim = anim.shared_from_this();
	m_meshVertexAnim = meshAnim.shared_from_this();
	m_meshFrame = meshFrame.shared_from_this();
}
