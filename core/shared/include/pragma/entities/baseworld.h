#ifndef __BASEWORLD_H__
#define __BASEWORLD_H__

#include "pragma/entities/components/base_entity_component.hpp"

#define WORLD_BASE_LOD 20
#define WORLD_LOD_OFFSET 4

class PolyMesh;
#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma
{
	class DLLNETWORK BaseWorldComponent
		: public BaseEntityComponent
	{
	public:
		virtual Con::c_cout &print(Con::c_cout&);
	public:
		BaseWorldComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		void SetBrushes(std::vector<PolyMesh*> brushes);
		void GetBrushes(std::vector<PolyMesh*> **brushes);
		virtual void OnEntitySpawn() override;
	protected:
		std::vector<PolyMesh*> m_brushes;
	};
};
#pragma warning(pop)

#endif
