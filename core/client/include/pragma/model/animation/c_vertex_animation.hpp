/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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
};

#endif
