#ifndef __BASEFUNCBRUSH_H__
#define __BASEFUNCBRUSH_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/func/basefuncsurfacematerial.hpp"
#include <pragma/console/conout.h>

namespace pragma
{
	class DLLNETWORK BaseFuncBrushComponent
		: public BaseFuncSurfaceMaterialComponent
	{
	public:
		using BaseFuncSurfaceMaterialComponent::BaseFuncSurfaceMaterialComponent;
		virtual void Initialize() override;
	protected:
		bool m_kvSolid = true;
	};
};

#endif
