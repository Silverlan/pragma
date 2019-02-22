#ifndef __BASE_PARENT_COMPONENT_HPP__
#define __BASE_PARENT_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

struct AttachmentData;
namespace pragma
{
	class BaseAttachableComponent;
	class DLLNETWORK BaseParentComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;

		void UpdateChildAttachmentData();

		const std::vector<util::WeakHandle<BaseAttachableComponent>> &GetChildren() const;
		std::vector<util::WeakHandle<BaseAttachableComponent>> &GetChildren();
	protected:
		friend BaseAttachableComponent;
		BaseParentComponent(BaseEntity &ent);
		void AddChild(BaseAttachableComponent &ent);
		void RemoveChild(BaseAttachableComponent &ent);

		std::vector<util::WeakHandle<BaseAttachableComponent>> m_children = {};
	};
};

#endif
