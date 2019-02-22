#ifndef __BASE_ATTACHABLE_COMPONENT_HPP__
#define __BASE_ATTACHABLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/parentmode.h"
#include "pragma/entities/parentinfo.h"
#include <sharedutils/property/util_property.hpp>

struct AttachmentData;
struct AttachmentInfo;
namespace pragma
{
	class BaseParentComponent;
	class DLLNETWORK BaseAttachableComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_ATTACHMENT_UPDATE;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		virtual void Initialize() override;
		virtual void OnRemove() override;
		void UpdateAttachmentOffset();

		AttachmentData *AttachToEntity(BaseEntity *ent,const AttachmentInfo &attInfo={});
		AttachmentData *AttachToBone(BaseEntity *ent,uint32_t boneID,const AttachmentInfo &attInfo={});
		AttachmentData *AttachToBone(BaseEntity *ent,std::string bone,const AttachmentInfo &attInfo={});
		AttachmentData *AttachToAttachment(BaseEntity *ent,uint32_t attachmentID,const AttachmentInfo &attInfo={});
		AttachmentData *AttachToAttachment(BaseEntity *ent,std::string attachment,const AttachmentInfo &attInfo={});
		virtual void SetAttachmentFlags(FAttachmentMode flags);
		void AddAttachmentFlags(FAttachmentMode flags);
		void RemoveAttachmentFlags(FAttachmentMode flags);
		FAttachmentMode GetAttachmentFlags() const;
		bool HasAttachmentFlag(FAttachmentMode flag) const;
		BaseParentComponent *GetParent() const;
		AttachmentData *GetAttachmentData() const;
		void UpdateAttachmentData(bool bForceReload=false);
		void ClearAttachment();

		void GetChildren(std::vector<BaseEntity*> &children) const;
		const std::vector<BaseEntity*> &GetChildren() const;
	protected:
		BaseAttachableComponent(BaseEntity &ent);
		virtual AttachmentData *SetupAttachment(BaseEntity *ent,const AttachmentInfo &attInfo);
		virtual void UpdateViewAttachmentOffset(BaseEntity *ent,pragma::BaseCharacterComponent &pl,Vector3 &pos,Quat &rot,Bool bYawOnly=false);
		virtual void OnEntitySpawn() override;
		void AddChild(BaseEntity *ent);
		void RemoveChild(BaseEntity *ent);

		std::vector<BaseEntity*> m_children = {};
		float m_tLastAttachmentUpdate = 0.f;
		std::unique_ptr<AttachmentData> m_attachment = nullptr;
		std::string m_parentName = {};
	};
};

#endif
