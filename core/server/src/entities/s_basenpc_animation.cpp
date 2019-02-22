#include "stdafx_server.h"
#if 0
#include "pragma/ai/ai_schedule.h"

void SBaseNPC::LockAnimation(bool b) {m_bAnimLocked = b;}
bool SBaseNPC::IsAnimationLocked() const {return m_bAnimLocked;}

void SBaseNPC::MaintainAnimationMovement(const Vector3 &disp)
{
	Entity::MaintainAnimationMovement(disp);
	if(disp.x == 0.f && disp.y == 0.f && disp.z == 0.f)
		return;
	Vector3 dirAnimMove = uvec::get_normal(disp);
	Vector3 dirMove = uvec::get_normal(m_posMove); // TODO: Move in actual animation move dir, not disp
	float dot = glm::dot(dirAnimMove,dirMove);
	dot = (dot +1.f) /2.f;
	Vector3 mv = disp *dot; // TODO
	//SetVelocity(mv);
}
#endif
