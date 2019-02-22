#ifndef __BASE_GAME_OBJECT_COMPONENT_HPP__
#define __BASE_GAME_OBJECT_COMPONENT_HPP__

// TODO: Remove this file
#if 0
#include "pragma/networkdefinitions.h"

namespace pragma
{
	class GameObject;
	class DLLNETWORK BaseGameObjectComponent
	{
	public:
		virtual ~BaseGameObjectComponent()=default;
		const GameObject &GetGameObject() const;
		GameObject &GetGameObject();
		const GameObject &operator->() const;
		GameObject &operator->();
	protected:
		BaseGameObjectComponent(GameObject &obj);
		GameObject &m_gameObject;
	};
};

#endif
#endif
