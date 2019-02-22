#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/model/brush/brushmesh.h"
bool Game::CollisionTest()
{
	if(!m_bCollisionsEnabled) return false;
	return false;
	//Player *pl = GetLocalPlayer();
	//if(pl == NULL) return false;
	//return CollisionTest(pl->GetPosition());
}

void Game::EnableCollisions(bool b) {m_bCollisionsEnabled = b;}

/*
void Game::CollisionTest(Entity *a,Entity *b)
{
	float delta = DeltaTime();
	Vector3 *posA = a->GetPosition();
	Vector3 extentsA = a->GetExtents();

	Vector3 *posB = b->GetPosition();
	Vector3 extentsB = b->GetExtents();

	Vector3 vel = *(b->GetVelocity()) -*(a->GetVelocity());
	float tfirst = 0;
	float tlast = FLT_MAX;

	std::vector<BrushMesh*> *meshesA;
	for(int i=0;i<meshesA->size();i++)
	{
		BrushMesh *mesh = (*meshesA)[i];
		std::vector<Side*> *sides;
		mesh->GetSides(&sides);
		for(int j=0;j<sides->size();j++)
		{
			Side *side = (*sides)[j];

		}
	}
}
*/
void Game::CollisionTest(BaseEntity *a,BaseEntity *b)
{
	/*float delta = 0;//DeltaTime(); // WEAVETODO
	auto &posA = a->GetPosition();
	auto &velA = a->GetVelocity();
	auto posANext = posA +(velA *delta);
	auto extentsA = a->GetExtents();

	auto &posB = b->GetPosition();
	auto &velB = b->GetVelocity();
	auto posBNext = posB +(velB *delta);
	auto extentsB = b->GetExtents();
	auto velBDelta = velB *delta;

	std::vector<BrushMesh*> *meshesA;
	a->GetBrushMeshes(&meshesA);*/
	//if(true) return;
	/*
	float distance;
	Vector3 hitnormal;
	for(int i=0;i<meshesA->size();i++)
	{
		distance = 0;
		if((*meshesA)[i]->IntersectAABB(*posB,posBNext,extentsB,*posA,&distance,&hitnormal))
		{
			for(int j=0;j<3;j++)
			{
				if(j == axis)
				{
					posBNext[j] = (*posB)[j] +(velBDelta[j] *(distance -0.0001f));
					(*velB)[j] = 0;
				}
				else
					posBNext[j] += velBDelta[j];
			}
		}
	}
	b->SetPosition(&posBNext);
	b->SetVelocity(velB);*/
}

bool Game::CollisionTest(pragma::BasePlayerComponent&,float*,Vector3*)
{
#ifdef COMMENT
	if(!m_bCollisionsEnabled) return false;
	BaseWorld *wrld = GetWorld();
	if(wrld == NULL) return false;
	Vector3 *plPos = pl->GetPosition();
	Vector3 *plVel = pl->GetVelocity();
	Vector3 pos = *plPos +(*plVel *float(DeltaTime()));
	Vector3 plMin,plMax;
	pl->GetCollisionBounds(&plMin,&plMax);

	Vector3 extentsThis = (plMax -plMin) *0.5f;
	Vector3 posThis = *plPos +plMin +extentsThis;
	Vector3 posThisNext = pos +plMin +extentsThis;
	Vector3 dir = Vector3::getNormal(plVel);
	int i = 0;
	bool collision = false;
	if(wrld->IntersectAABB(posThis,posThisNext,extentsThis,distance,hitnormal,&i))
	{
		collision = true;
		//pl->SetPosition(&((*plPos) -(dir *(*plVel)) *(*ua)));
		//return true;
	}
	//plMin += pos;
	//plMax += pos;
	//if(wrld->IntersectAABB(plMin,plMax))
	//{
	//	return true;
	//}
	/*std::vector<PolyMesh*> *brushes;
	wrld->GetBrushes(&brushes);
	Vector3 min,max;
	int c = 0;
	for(int i=0;i<brushes->size();i++)
	{
		PolyMesh *brush = (*brushes)[i];
		if(brush->IntersectAABB(&plMin,&plMax))
			return true;
		//if(Intersection::AABBInAABB(&plMin,&plMax,&min,&max))
		//	c++;
	}
	return c > 0;*/
	return collision;
#endif // WEAVETODO
	return false;
}

bool Game::CollisionTest(Vector3*)
{
	/*if(!m_bCollisionsEnabled) return false;
	BaseWorld *wrld = GetWorld();
	if(wrld == NULL) return false;
	std::vector<PolyMesh*> *brushes;
	wrld->GetBrushes(&brushes);
	Vector3 min,max;
	int c = 0;
	for(int i=0;i<brushes->size();i++)
	{
		PolyMesh *brush = (*brushes)[i];
		Vector3 *center = brush->GetWorldPosition();
		brush->GetBounds(&min,&max);
		min += *center;
		max += *center;
		//std::cout<<"Bounds: "<<Vector3::tostring(&min)<<"\t"<<Vector3::tostring(&max)<<std::endl;
		if(pos->x >= min.x && pos->y >= min.y && pos->z >= min.z
			&& pos->x <= max.x && pos->y <= max.y && pos->z <= max.z)
		{
			c++;
		}
	}
	return c > 0;*/
	return false;
}