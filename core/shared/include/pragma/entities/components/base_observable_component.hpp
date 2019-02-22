#ifndef __BASE_OBSERVABLE_COMPONENT_HPP__
#define __BASE_OBSERVABLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property_vector.h>

namespace pragma
{
	class DLLNETWORK BaseObservableComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;

		virtual void SetFirstPersonObserverOffset(const Vector3 &offset);
		virtual void SetThirdPersonObserverOffset(const Vector3 &offset);
		void SetObserverOffset(const Vector3 &offset);
		bool GetFirstPersonObserverOffset(Vector3 &offset) const;
		bool GetThirdPersonObserverOffset(Vector3 &offset) const;
		const Vector3 &GetFirstPersonObserverOffset() const;
		const Vector3 &GetThirdPersonObserverOffset() const;
		void ResetFirstPersonObserverOffset();
		void ResetThirdPersonObserverOffset();
		void ResetObserverOffset();

		void SetFirstPersonObserverOffsetEnabled(bool bEnabled);
		void SetThirdPersonObserverOffsetEnabled(bool bEnabled);
		bool IsFirstPersonObserverOffsetEnabled() const;
		bool IsThirdPersonObserverOffsetEnabled() const;

		const util::PBoolProperty &GetFirstPersonModeEnabledProperty() const;
		const util::PBoolProperty &GetThirdPersonModeEnabledProperty() const;
		bool IsFirstPersonModeEnabled() const;
		bool IsThirdPersonModeEnabled() const;

		const util::PVector3Property &GetFirstPersonObserverOffsetProperty() const;
		const util::PVector3Property &GetThirdPersonObserverOffsetProperty() const;

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;
	protected:
		BaseObservableComponent(BaseEntity &ent);

		pragma::NetEventId m_netEvSetFirstPersonObserverOffset = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetThirdPersonObserverOffset = pragma::INVALID_NET_EVENT;

		util::PBoolProperty m_bFirstPersonEnabled;
		util::PBoolProperty m_bThirdPersonEnabled;
		util::PVector3Property m_firstPersonObserverOffset;
		util::PVector3Property m_thirdPersonObserverOffset;
	};
};

#endif
