#ifndef __C_VERTEX_ANIMATION_HPP__
#define __C_VERTEX_ANIMATION_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/model/animation/vertex_animation.hpp>

class DLLCLIENT CVertexAnimation
	: public VertexAnimation
{
public:
	CVertexAnimation();
	CVertexAnimation(const CVertexAnimation &other);
	CVertexAnimation(const std::string &name);
	virtual std::unique_ptr<VertexAnimation> Copy() const override;

	void UpdateBuffer();
protected:
	//Vulkan::Buffer m_buffer = nullptr; // prosper TODO
};

#endif
