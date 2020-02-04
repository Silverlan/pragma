#ifndef __ENV_DECAL_H__
#define __ENV_DECAL_H__

#include "pragma/networkdefinitions.h"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvDecalComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;

		float GetSize() const;
		const std::string &GetMaterial() const;
		bool GetStartDisabled() const;
	protected:
		float m_size = 1.f;
		std::string m_material = "";
		bool m_startDisabled = false;
	};
};

#endif
