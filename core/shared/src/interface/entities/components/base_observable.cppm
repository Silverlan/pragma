// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"


export module pragma.shared:entities.components.base_observable;

export import :entities.components.base;
export import pragma.util;

export namespace pragma {
	class BaseObserverComponent;
	struct DLLNETWORK ObserverCameraData {
		ObserverCameraData();
		ObserverCameraData(const ObserverCameraData &) = default;
		ObserverCameraData &operator=(const ObserverCameraData &) = default;
		util::PBoolProperty enabled;
		// If not set, the entity's eye offset will be used as origin instead
		std::optional<Vector3> localOrigin = {};
		util::PVector3Property offset;
		bool rotateWithObservee = false;
		std::optional<std::pair<EulerAngles, EulerAngles>> angleLimits = {};
	};
		namespace baseObservableComponent {
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_OBSERVER_CHANGED;
	}
class DLLNETWORK BaseObservableComponent : public BaseEntityComponent {
	  public:
		enum class CameraType : uint8_t {
			FirstPerson,
			ThirdPerson,

			Count
		};

		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		virtual void Initialize() override;

		virtual void SetLocalCameraOrigin(CameraType type, const Vector3 &origin);
		void ClearLocalCameraOrigin(CameraType type);
		Vector3 GetLocalCameraOrigin(CameraType type) const;

		virtual void SetLocalCameraOffset(CameraType type, const Vector3 &offset);
		const Vector3 &GetLocalCameraOffset(CameraType type) const;

		const ObserverCameraData &GetCameraData(CameraType type) const;
		ObserverCameraData &GetCameraData(CameraType type);

		void SetCameraEnabled(CameraType type, bool enabled);
		bool IsCameraEnabled(CameraType type) const;

		const util::PBoolProperty &GetCameraEnabledProperty(CameraType type) const;
		const util::PVector3Property &GetCameraOffsetProperty(CameraType type) const;

		BaseObserverComponent *GetObserver();

		Vector3 &GetViewOffset();
		virtual void SetViewOffset(const Vector3 &offset);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseObservableComponent(pragma::ecs::BaseEntity &ent);

		friend BaseObserverComponent;
		void ClearObserver();
		void SetObserver(BaseObserverComponent *observer);

		pragma::NetEventId m_netSetObserverOffset = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netSetObserverOrigin = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netSetViewOffset = pragma::INVALID_NET_EVENT;
		std::array<ObserverCameraData, umath::to_integral(CameraType::Count)> m_cameraData = {};
		pragma::ComponentHandle<pragma::BaseObserverComponent> m_observer;
		Vector3 m_viewOffset = {};
	};
};
