#include "stdafx_server.h"
#if 0
#include "pragma/entities/player.h"

SBaseNPC::ControlInfo::ControlInfo()
	: hController{},hCbOnRemove{},hCbOnKilled{},
	hCbOnActionInput{}
{}

void SBaseNPC::ControlInfo::Clear()
{
	hController = {};
	if(hCbOnRemove.IsValid())
		hCbOnRemove.Remove();
	if(hCbOnKilled.IsValid())
		hCbOnKilled.Remove();
	if(hCbOnActionInput.IsValid())
		hCbOnActionInput.Remove();
}

void SBaseNPC::SelectControllerSchedule() {}

bool SBaseNPC::IsControllable() const {return m_bControllable;}
void SBaseNPC::SetControllable(bool b)
{
	m_bControllable = b;
	if(b == false)
		EndControl();
}
void SBaseNPC::StartControl(Player *pl)
{
	if(IsControllable() == false || IsControlled() == true)
		return;
	pl->SetNoTarget(true);
	CancelSchedule();
	m_controlInfo.actions = Action::None; // We want to overwrite the player's controls, so we need to keep track of them ourselves
	m_controlInfo.hCbOnRemove = pl->AddCallback("OnRemove",FunctionCallback<>::Create(std::bind(&SBaseNPC::EndControl,this)));
	m_controlInfo.hCbOnKilled = pl->AddCallback("OnDeath",FunctionCallback<>::Create(std::bind(&SBaseNPC::EndControl,this)));
	m_controlInfo.hCbOnActionInput = pl->AddCallback("OnActionInput",FunctionCallback<bool,bool*,Action,bool>::Create([this](bool *ret,Action action,bool bPressed) {
		*ret = false;
		if(bPressed == true)
		{
			if((m_controlInfo.actions &action) != Action::None)
				return true;
			m_controlInfo.actions |= action;
		}
		else
		{
			if((m_controlInfo.actions &action) == Action::None)
				return true;
			m_controlInfo.actions &= ~action;
		}
		OnControllerActionInput(action,bPressed);
		return true;
	}));
	m_controlInfo.hController = pl->GetHandle();
	pl->SetObserverMode(OBSERVERMODE::THIRDPERSON);
	pl->SetObserverTarget(this);
	DisableAI();
	OnStartControl(pl);
}
Action SBaseNPC::GetControllerActionInput() const {return m_controlInfo.actions;}
void SBaseNPC::OnControllerActionInput(Action,bool) {}
void SBaseNPC::OnStartControl(Player*) {}
void SBaseNPC::OnEndControl() {}
void SBaseNPC::EndControl()
{
	if(IsControlled() == false)
		return;
	auto *pl = static_cast<Player*>(m_controlInfo.hController.get());
	if(pl != nullptr)
	{
		pl->SetNoTarget(false);
		pl->SetObserverMode(OBSERVERMODE::FIRSTPERSON);
		pl->SetObserverTarget(pl);
		pl->ResetObserverOffset();
	}
	OnEndControl();
	EnableAI();
	m_controlInfo.Clear();
}
bool SBaseNPC::IsControlled() const {return m_controlInfo.hController.IsValid();}
Player *SBaseNPC::GetController() const {return static_cast<Player*>(m_controlInfo.hController.get());}

#endif
