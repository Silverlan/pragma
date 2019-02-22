#include "stdafx_engine.h"
#include "pragma/physics/physapi.h"

Vector3 uvec::create(const btVector3 &v) {return Vector3(v.x(),v.y(),v.z());}
btVector3 uvec::create_bt(const Vector3 &v) {return btVector3(v.x,v.y,v.z);}
Quat uquat::create(const btQuaternion &q) {return Quat(CFloat(q.w()),CFloat(q.x()),CFloat(q.y()),CFloat(q.z()));}
btQuaternion uquat::create_bt(const Quat &q) {return btQuaternion(q.x,q.y,q.z,q.w);}