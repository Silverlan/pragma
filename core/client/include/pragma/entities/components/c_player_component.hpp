#ifndef __C_PLAYER_COMPONENT_HPP__
#define __C_PLAYER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/util/util_handled.hpp>

class CBaseEntity;
struct DeltaOffset;
struct DeltaTransform;
namespace pragma
{
	class DLLCLIENT CPlayerComponent final
		: public BasePlayerComponent,
		public CBaseNetComponent
	{
	private:
		static std::vector<CPlayerComponent*> s_players;
		const float VIEW_BODY_OFFSET = -20.f;
	public:
		CPlayerComponent(BaseEntity &ent);
		virtual ~CPlayerComponent() override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		CallbackHandle m_cbCalcOrientationView = {};
		Vector3 m_viewOffset = {};
		std::unique_ptr<DeltaOffset> m_crouchViewOffset = nullptr;
		std::unique_ptr<DeltaTransform> m_upDirOffset = nullptr;
		std::shared_ptr<ALSound> m_sndUnderwater = nullptr;
		CallbackHandle m_cbObserver = {};
		EntityHandle m_cbUnderwaterDsp = {};
		void OnWaterSubmerged();
		void OnWaterEmerged();
		void OnUpdateMatrices(Mat4 &transformMatrix);
		void OnDeployWeapon(BaseEntity &ent);
		void OnSetActiveWeapon(BaseEntity *ent);
		void OnSetUpDirection(const Vector3 &direction);
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		void UpdateObserverCallback();
		void OnSetCharacterOrientation(const Vector3 &up);
		bool ShouldDraw(const Vector3 &camOrigin) const;
		bool ShouldDrawShadow(const Vector3 &camOrigin) const;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	public:
		static unsigned int GetPlayerCount();
		static const std::vector<CPlayerComponent*> &GetAll();
		Con::c_cout& print(Con::c_cout&);
		std::ostream& print(std::ostream&);
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Think(double tDelta) override;
		Vector3 &GetViewOffset();
		void SetViewOffset(Vector3 offset);
		virtual void OnCrouch() override;
		virtual void OnUnCrouch() override;
		virtual void SetLocalPlayer(bool b) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;

		void UpdateObserverOffset();
		virtual void SetObserverTarget(BaseObservableComponent *ent) override;
		virtual void SetObserverMode(OBSERVERMODE mode) override;
		virtual void SetObserverCameraOffset(const Vector3 &offset) override;
		virtual void ApplyViewRotationOffset(const EulerAngles &ang,float dur=0.5f) override;
		virtual void PrintMessage(std::string message,MESSAGE type) override;

		// Returns true if observer mode is first-person and observer target is this player (Only works for the local player)
		bool IsInFirstPersonMode() const;

		void UpdateViewModelTransform();
		void UpdateViewFOV();
	};
};

#endif