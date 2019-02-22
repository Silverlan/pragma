#ifndef __ENV_SPRITE_H__
#define __ENV_SPRITE_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/basetoggle.h"
#include <string>
#include <mathutil/color.h>

namespace pragma
{
	class DLLNETWORK BaseEnvSpriteComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	protected:
		enum class DLLNETWORK SpawnFlags : uint32_t
		{
			BlackToAlpha = 4'096,
			NoSoftParticles = BlackToAlpha<<1
		};
		uint32_t m_particleRenderMode = std::numeric_limits<uint32_t>::max();
		std::string m_spritePath;
		float m_size = 1.f;
		float m_bloomScale = 0.f;
		float m_tFadeIn = 0.f;
		float m_tFadeOut = 0.f;
		Color m_color = {};
	};
};

#endif
