/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEFUNCBRUSH_H__
#define __BASEFUNCBRUSH_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/func/basefuncsurfacematerial.hpp"
#include <pragma/console/conout.h>

namespace pragma {
	class DLLNETWORK BaseFuncBrushComponent : public BaseFuncSurfaceMaterialComponent {
	  public:
		using BaseFuncSurfaceMaterialComponent::BaseFuncSurfaceMaterialComponent;
		virtual void Initialize() override;
	  protected:
		bool m_kvSolid = true;
	};
};

#endif
