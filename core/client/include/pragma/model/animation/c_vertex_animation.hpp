#ifndef __C_VERTEX_ANIMATION_HPP__
#define __C_VERTEX_ANIMATION_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/model/animation/vertex_animation.hpp>

class DLLCLIENT CVertexAnimation
	: public VertexAnimation
{
public:
	static std::shared_ptr<CVertexAnimation> Create();
	static std::shared_ptr<CVertexAnimation> Create(const CVertexAnimation &other);
	static std::shared_ptr<CVertexAnimation> Create(const std::string &name);

	virtual std::shared_ptr<VertexAnimation> Copy() const override;

	void UpdateBuffer();
protected:
	CVertexAnimation();
	CVertexAnimation(const CVertexAnimation &other);
	CVertexAnimation(const std::string &name);
	//Vulkan::Buffer m_buffer = nullptr; // prosper TODO
};

#endif
