#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/engine.h>
extern DLLENGINE Engine *engine;
double &NetworkState::RealTime() {return m_tReal;}
double &NetworkState::DeltaTime() {return m_tDelta;}
double &NetworkState::LastThink() {return m_tLast;}