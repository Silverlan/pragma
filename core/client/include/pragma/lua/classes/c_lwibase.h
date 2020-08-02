/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LWIBASE_H__
#define __C_LWIBASE_H__
#include "pragma/definitions.h"
#include "pragma/clientstate/clientstate.h"
#include <wgui/wibase.h>
#include <pragma/lua/ldefinitions.h>
class WIHandle;
class WIShapeHandle;
class WITexturedShapeHandle;
class WITextHandle;
class WITextEntryHandle;
class WIOutlinedRectHandle;
class WILineHandle;
class WIRoundedRectHandle;
class WIRoundedTexturedRectHandle;
class WIScrollBarHandle;
class WISilkIconHandle;
class WIIconHandle;
class WIDropDownMenuHandle;
class WIConsoleHandle;
class WICheckboxHandle;
class WIArrowHandle;
class WIButtonHandle;
class WITableHandle;
class WIGridPanelHandle;
class WITableRowHandle;
class WITableCellHandle;
class WIFrameHandle;
class WITransformableHandle;
class WITreeListHandle;
class WITreeListElementHandle;
class WIProgressBarHandle;
class WISliderHandle;
class WINumericEntryHandle;
class WIContainerHandle;
namespace prosper {class Texture;};
namespace Lua
{
	namespace gui
	{
		DLLCLIENT void clear_lua_callbacks(lua_State *l);
	};
	namespace WIBase
	{
		DLLCLIENT void register_class(luabind::class_<WIHandle> &classDef);
		DLLCLIENT void IsValid(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void Remove(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void RemoveSafely(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetZPos(lua_State *l,WIHandle &hPanel,int zpos);
		DLLCLIENT void GetZPos(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void HasFocus(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void RequestFocus(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void KillFocus(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void TrapFocus(lua_State *l,WIHandle &hPanel,bool bTrap);
		DLLCLIENT void TrapFocus(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void IsFocusTrapped(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void IsVisible(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetVisible(lua_State *l,WIHandle &hPanel,bool b);
		DLLCLIENT void GetMouseInputEnabled(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetMouseInputEnabled(lua_State *l,WIHandle &hPanel,bool b);
		DLLCLIENT void GetKeyboardInputEnabled(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetKeyboardInputEnabled(lua_State *l,WIHandle &hPanel,bool b);
		DLLCLIENT void GetScrollInputEnabled(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetScrollInputEnabled(lua_State *l,WIHandle &hPanel,bool b);
		DLLCLIENT void GetMouseMovementCheckEnabled(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetMouseMovementCheckEnabled(lua_State *l,WIHandle &hPanel,bool b);
		DLLCLIENT void GetPos(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetPos(lua_State *l,WIHandle &hPanel,Vector2 pos);
		DLLCLIENT void SetPos(lua_State *l,WIHandle &hPanel,float x,float y);
		DLLCLIENT void GetAbsolutePos(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetAbsolutePos(lua_State *l,WIHandle &hPanel,Vector2 pos);
		DLLCLIENT void SetAbsolutePos(lua_State *l,WIHandle &hPanel,float x,float y);
		DLLCLIENT void GetColor(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetColorProperty(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetColor(lua_State *l,WIHandle &hPanel,Color col);
		DLLCLIENT void GetAlpha(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetAlpha(lua_State *l,WIHandle &hPanel,float alpha);
		DLLCLIENT void GetWidth(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetHeight(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetSize(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetSize(lua_State *l,WIHandle &hPanel,Vector2 size);
		DLLCLIENT void SetSize(lua_State *l,WIHandle &hPanel,float x,float y);
		DLLCLIENT void Wrap(lua_State *l,WIHandle &hPanel,const std::string &wrapperClassName);
		DLLCLIENT void Wrap(lua_State *l,WIHandle &hPanel,WIHandle &hWrapper);
		DLLCLIENT void GetParent(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetParent(lua_State *l,WIHandle &hPanel,WIHandle &hParent);
		DLLCLIENT void ClearParent(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetChildren(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetChildren(lua_State *l,WIHandle &hPanel,std::string className);
		DLLCLIENT void GetFirstChild(lua_State *l,WIHandle &hPanel,std::string className);
		DLLCLIENT void GetChild(lua_State *l,WIHandle &hPanel,unsigned int idx);
		DLLCLIENT void GetChild(lua_State *l,WIHandle &hPanel,std::string className,unsigned int idx);
		DLLCLIENT void PosInBounds(lua_State *l,WIHandle &hPanel,Vector2 pos);
		DLLCLIENT void MouseInBounds(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetMousePos(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void Draw(lua_State *l,WIHandle &hPanel,const ::WIBase::DrawInfo &drawInfo);
		DLLCLIENT void Draw(lua_State *l,WIHandle &hPanel,const ::WIBase::DrawInfo &drawInfo,const Vector2i &scissorOffset,const Vector2i &scissorSize);
		DLLCLIENT void Draw(lua_State *l,WIHandle &hPanel,const ::WIBase::DrawInfo &drawInfo,const Vector2i &scissorOffset,const Vector2i &scissorSize,const Vector2i &offsetParent);
		DLLCLIENT void GetX(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetY(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetX(lua_State *l,WIHandle &hPanel,float x); // TODO This should be int, but float-to-int conversion is currently broken with luabind
		DLLCLIENT void SetY(lua_State *l,WIHandle &hPanel,float y);
		DLLCLIENT void SetWidth(lua_State *l,WIHandle &hPanel,float w);
		DLLCLIENT void SetHeight(lua_State *l,WIHandle &hPanel,float h);
		DLLCLIENT void SizeToContents(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7);
		DLLCLIENT void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8);
		DLLCLIENT void AddCallback(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o);
		DLLCLIENT void FadeIn(lua_State *l,WIHandle &hPanel,float tFadeIn);
		DLLCLIENT void FadeIn(lua_State *l,WIHandle &hPanel,float tFadeIn,float alphaTarget);
		DLLCLIENT void FadeOut(lua_State *l,WIHandle &hPanel,float tFadeOut);
		DLLCLIENT void FadeOut(lua_State *l,WIHandle &hPanel,float tFadeOut,bool remove);
		DLLCLIENT void IsFading(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void IsFadingIn(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void IsFadingOut(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetClass(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void Think(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void InjectMouseMoveInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos);
		DLLCLIENT void InjectMouseInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos,int button,int action,int mods);
		DLLCLIENT void InjectMouseInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos,int button,int action);
		DLLCLIENT void InjectKeyboardInput(lua_State *l,WIHandle &hPanel,int key,int action,int mods);
		DLLCLIENT void InjectKeyboardInput(lua_State *l,WIHandle &hPanel,int key,int action);
		DLLCLIENT void InjectCharInput(lua_State *l,WIHandle &hPanel,std::string c,uint32_t mods);
		DLLCLIENT void InjectCharInput(lua_State *l,WIHandle &hPanel,std::string c);
		DLLCLIENT void InjectScrollInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos,const Vector2 &offset);
		DLLCLIENT void IsDescendant(lua_State *l,WIHandle &hPanel,WIHandle &hOther);
		DLLCLIENT void IsDescendantOf(lua_State *l,WIHandle &hPanel,WIHandle &hOther);
		DLLCLIENT void IsAncestor(lua_State *l,WIHandle &hPanel,WIHandle &hOther);
		DLLCLIENT void IsAncestorOf(lua_State *l,WIHandle &hPanel,WIHandle &hOther);
		DLLCLIENT void GetName(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetName(lua_State *l,WIHandle &hPanel,std::string name);
		DLLCLIENT void FindChildByName(lua_State *l,WIHandle &hPanel,std::string name);
		DLLCLIENT void FindChildrenByName(lua_State *l,WIHandle &hPanel,std::string name);
		DLLCLIENT void SetAutoAlignToParent(lua_State *l,WIHandle &hPanel,bool b);
		DLLCLIENT void SetAutoAlignToParent(lua_State *l,WIHandle &hPanel,bool bX,bool bY);
		DLLCLIENT void GetAutoAlignToParentX(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetAutoAlignToParentY(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void Resize(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void ScheduleUpdate(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetSkin(lua_State *l,WIHandle &hPanel,std::string skin);
		DLLCLIENT void ResetSkin(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetStyleClasses(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void AddStyleClass(lua_State *l,WIHandle &hPanel,std::string styleClass);
		DLLCLIENT void SetCursor(lua_State *l,WIHandle &hPanel,uint32_t cursor);
		DLLCLIENT void GetCursor(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void RemoveElementOnRemoval(lua_State *l,WIHandle &hPanel,WIHandle &hOther);
		DLLCLIENT void GetTooltip(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetTooltip(lua_State *l,WIHandle &hPanel,const std::string &tooltip);
		DLLCLIENT void HasTooltip(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetLeft(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetTop(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetRight(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetBottom(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void GetEndPos(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetClippingEnabled(lua_State *l,WIHandle &hPanel,bool b);
		DLLCLIENT void IsClippingEnabled(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetBounds(lua_State *l,WIHandle &hPanel,const Vector2 &start,const Vector2 &end);
		DLLCLIENT void SetBackgroundElement(lua_State *l,WIHandle &hPanel,bool backgroundElement,bool autoAlignToParent);
		DLLCLIENT void SetBackgroundElement(lua_State *l,WIHandle &hPanel,bool backgroundElement);
		DLLCLIENT void SetBackgroundElement(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void IsBackgroundElement(lua_State *l,WIHandle &hPanel);

		DLLCLIENT void SetAlwaysUpdate(lua_State *l,WIHandle &hPanel,bool b);

		DLLCLIENT void AddAttachment(lua_State *l,WIHandle &hPanel,const std::string &name,const Vector2 &position);
		DLLCLIENT void AddAttachment(lua_State *l,WIHandle &hPanel,const std::string &name);
		DLLCLIENT void SetAttachmentPos(lua_State *l,WIHandle &hPanel,const std::string &name,const Vector2 &position);
		DLLCLIENT void GetAttachmentPos(lua_State *l,WIHandle &hPanel,const std::string &name);
		DLLCLIENT void GetAbsoluteAttachmentPos(lua_State *l,WIHandle &hPanel,const std::string &name);
		DLLCLIENT void GetAttachmentPosProperty(lua_State *l,WIHandle &hPanel,const std::string &name);

		DLLCLIENT void SetAnchor(lua_State *l,WIHandle &hPanel,float left,float top,float right,float bottom);
		DLLCLIENT void SetAnchor(lua_State *l,WIHandle &hPanel,float left,float top,float right,float bottom,uint32_t refWidth,uint32_t refHeight);
		DLLCLIENT void SetAnchorLeft(lua_State *l,WIHandle &hPanel,float f);
		DLLCLIENT void SetAnchorRight(lua_State *l,WIHandle &hPanel,float f);
		DLLCLIENT void SetAnchorTop(lua_State *l,WIHandle &hPanel,float f);
		DLLCLIENT void SetAnchorBottom(lua_State *l,WIHandle &hPanel,float f);
		DLLCLIENT void GetAnchor(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void HasAnchor(lua_State *l,WIHandle &hPanel);
		DLLCLIENT void SetRemoveOnParentRemoval(lua_State *l,WIHandle &hPanel,bool b);
	};

	namespace WIButton
	{
		DLLCLIENT void register_class(luabind::class_<WIButtonHandle,WIHandle> &classDef);
		DLLCLIENT void SetText(lua_State *l,WIButtonHandle &hPanel,std::string text);
		DLLCLIENT void GetText(lua_State *l,WIButtonHandle &hPanel);
	};

	namespace WIProgressBar
	{
		DLLCLIENT void register_class(luabind::class_<WIProgressBarHandle,WIHandle> &classDef);
		DLLCLIENT void SetProgress(lua_State *l,WIProgressBarHandle &hBar,float progress);
		DLLCLIENT void GetProgress(lua_State *l,WIProgressBarHandle &hBar);
		DLLCLIENT void SetValue(lua_State *l,WIProgressBarHandle &hBar,float v);
		DLLCLIENT void GetValue(lua_State *l,WIProgressBarHandle &hBar);
		DLLCLIENT void SetRange(lua_State *l,WIProgressBarHandle &hBar,float min,float max,float stepSize);
		DLLCLIENT void SetRange(lua_State *l,WIProgressBarHandle &hBar,float min,float max);
		DLLCLIENT void SetOptions(lua_State *l,WIProgressBarHandle &hBar,luabind::object o);
		DLLCLIENT void AddOption(lua_State *l,WIProgressBarHandle &hBar,const std::string &option);
		DLLCLIENT void SetPostFix(lua_State *l,WIProgressBarHandle &hBar,const std::string &postfix);
	};

	namespace WISlider
	{
		DLLCLIENT void register_class(luabind::class_<WISliderHandle,luabind::bases<WIProgressBarHandle,WIHandle>> &classDef);
	};

	namespace WIShape
	{
		DLLCLIENT void register_class(luabind::class_<WIShapeHandle,WIHandle> &classDef);
		DLLCLIENT void AddVertex(lua_State *l,WIShapeHandle &hPanel,Vector2 v);
		DLLCLIENT void SetVertexPos(lua_State *l,WIShapeHandle &hPanel,unsigned int vertID,Vector2 v);
		DLLCLIENT void ClearVertices(lua_State *l,WIShapeHandle &hPanel);
		DLLCLIENT void InvertVertexPositions(lua_State *l,WIShapeHandle &hPanel,bool x,bool y);
		DLLCLIENT void InvertVertexPositions(lua_State *l,WIShapeHandle &hPanel);
	};

	namespace WITexturedShape
	{
		DLLCLIENT void register_class(luabind::class_<WITexturedShapeHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef);
		DLLCLIENT void SetMaterial(lua_State *l,WITexturedShapeHandle &hPanel,std::string mat);
		DLLCLIENT void SetMaterial(lua_State *l,WITexturedShapeHandle &hPanel,Material *mat);
		DLLCLIENT void GetMaterial(lua_State *l,WITexturedShapeHandle &hPanel);
		DLLCLIENT void SetTexture(lua_State *l,WITexturedShapeHandle &hPanel,std::shared_ptr<prosper::Texture> &tex);
		DLLCLIENT void GetTexture(lua_State *l,WITexturedShapeHandle &hPanel);
		DLLCLIENT void AddVertex(lua_State *l,WITexturedShapeHandle &hPanel,Vector2 v);
		DLLCLIENT void AddVertex(lua_State *l,WITexturedShapeHandle &hPanel,Vector2 v,Vector2 uv);
		DLLCLIENT void SetVertexUVCoord(lua_State *l,WITexturedShapeHandle &hPanel,unsigned int vertID,Vector2 uv);
		DLLCLIENT void InvertVertexUVCoordinates(lua_State *l,WITexturedShapeHandle &hPanel,bool x,bool y);
		DLLCLIENT void InvertVertexUVCoordinates(lua_State *l,WITexturedShapeHandle &hPanel);
	};

	namespace WIText
	{
		DLLCLIENT void register_class(luabind::class_<WITextHandle,WIHandle> &classDef);
		DLLCLIENT void SetText(lua_State *l,WITextHandle &hPanel,std::string text);
		DLLCLIENT void GetText(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void SetFont(lua_State *l,WITextHandle &hPanel,std::string font);
		DLLCLIENT void SetFont(lua_State *l,WITextHandle &hPanel,std::string font);
		DLLCLIENT void EnableShadow(lua_State *l,WITextHandle &hPanel,bool b);
		DLLCLIENT void IsShadowEnabled(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void SetShadowColor(lua_State *l,WITextHandle &hPanel,Color &col);
		DLLCLIENT void SetShadowOffset(lua_State *l,WITextHandle &hPanel,Vector2 pos);
		DLLCLIENT void SetShadowXOffset(lua_State *l,WITextHandle &hPanel,float x);
		DLLCLIENT void SetShadowYOffset(lua_State *l,WITextHandle &hPanel,float y);
		DLLCLIENT void GetShadowColor(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void GetShadowOffset(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void GetShadowXOffset(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void GetShadowYOffset(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void SetShadowAlpha(lua_State *l,WITextHandle &hPanel,float alpha);
		DLLCLIENT void GetShadowAlpha(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void SetShadowBlurSize(lua_State *l,WITextHandle &hPanel,float blurSize);
		DLLCLIENT void GetShadowBlurSize(lua_State *l,WITextHandle &hPanel);
		DLLCLIENT void SetAutoBreakMode(lua_State *l,WITextHandle &hPanel,uint32_t autoBreakMode);
	};

	namespace WITextEntry
	{
		DLLCLIENT void register_class(luabind::class_<WITextEntryHandle,WIHandle> &classDef);
		DLLCLIENT void SetText(lua_State *l,WITextEntryHandle &hPanel,std::string text);
		DLLCLIENT void GetText(lua_State *l,WITextEntryHandle &hPanel);
		DLLCLIENT void IsNumeric(lua_State *l,WITextEntryHandle &hPanel);
		DLLCLIENT void IsEditable(lua_State *l,WITextEntryHandle &hPanel);
		DLLCLIENT void SetEditable(lua_State *l,WITextEntryHandle &hPanel,bool b);
		DLLCLIENT void SetMaxLength(lua_State *l,WITextEntryHandle &hPanel,int len);
		DLLCLIENT void GetMaxLength(lua_State *l,WITextEntryHandle &hPanel);
		DLLCLIENT void IsMultiLine(lua_State *l,WITextEntryHandle &hPanel);
		DLLCLIENT void SetMultiLine(lua_State *l,WITextEntryHandle &hPanel,bool b);
	};

	namespace WINumericEntry
	{
		DLLCLIENT void register_class(luabind::class_<WINumericEntryHandle,luabind::bases<WITextEntryHandle,WIHandle>> &classDef);
		DLLCLIENT void SetMinValue(lua_State *l,WINumericEntryHandle &hPanel,int32_t min);
		DLLCLIENT void SetMinValue(lua_State *l,WINumericEntryHandle &hPanel);
		DLLCLIENT void SetMaxValue(lua_State *l,WINumericEntryHandle &hPanel,int32_t max);
		DLLCLIENT void SetMaxValue(lua_State *l,WINumericEntryHandle &hPanel);
		DLLCLIENT void SetRange(lua_State *l,WINumericEntryHandle &hPanel,int32_t min,int32_t max);
		DLLCLIENT void GetMinValue(lua_State *l,WINumericEntryHandle &hPanel);
		DLLCLIENT void GetMaxValue(lua_State *l,WINumericEntryHandle &hPanel);
	};

	namespace WIOutlinedRect
	{
		DLLCLIENT void register_class(luabind::class_<WIOutlinedRectHandle,WIHandle> &classDef);
		DLLCLIENT void SetOutlineWidth(lua_State *l,WIOutlinedRectHandle &hPanel,int w);
		DLLCLIENT void GetOutlineWidth(lua_State *l,WIOutlinedRectHandle &hPanel);
	};

	namespace WILine
	{
		DLLCLIENT void register_class(luabind::class_<WILineHandle,WIHandle> &classDef);
		DLLCLIENT void SetLineWidth(lua_State *l,WILineHandle &hPanel,int w);
		DLLCLIENT void GetLineWidth(lua_State *l,WILineHandle &hPanel);
		DLLCLIENT void SetStartPos(lua_State *l,WILineHandle &hPanel,Vector2 &pos);
		DLLCLIENT void SetEndPos(lua_State *l,WILineHandle &hPanel,Vector2 &pos);
		DLLCLIENT void GetStartPos(lua_State *l,WILineHandle &hPanel);
		DLLCLIENT void GetEndPos(lua_State *l,WILineHandle &hPanel);
		DLLCLIENT void SetStartColor(lua_State *l,WILineHandle &hPanel,const Color &col);
		DLLCLIENT void SetEndColor(lua_State *l,WILineHandle &hPanel,const Color &col);
		DLLCLIENT void GetStartColor(lua_State *l,WILineHandle &hPanel);
		DLLCLIENT void GetEndColor(lua_State *l,WILineHandle &hPanel);
		DLLCLIENT void GetStartPosProperty(lua_State *l,WILineHandle &hPanel);
		DLLCLIENT void GetEndPosProperty(lua_State *l,WILineHandle &hPanel);
	};

	namespace WIRoundedRect
	{
		DLLCLIENT void register_class(luabind::class_<WIRoundedRectHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef);
		DLLCLIENT void GetRoundness(lua_State *l,WIRoundedRectHandle &hPanel);
		DLLCLIENT void SetRoundness(lua_State *l,WIRoundedRectHandle &hPanel,char roundness);
		DLLCLIENT void SetCornerSize(lua_State *l,WIRoundedRectHandle &hPanel,float cornerSize);
		DLLCLIENT void GetCornerSize(lua_State *l,WIRoundedRectHandle &hPanel);
		DLLCLIENT void SetRoundTopRight(lua_State *l,WIRoundedRectHandle &hPanel,bool b);
		DLLCLIENT void SetRoundTopLeft(lua_State *l,WIRoundedRectHandle &hPanel,bool b);
		DLLCLIENT void SetRoundBottomLeft(lua_State *l,WIRoundedRectHandle &hPanel,bool b);
		DLLCLIENT void SetRoundBottomRight(lua_State *l,WIRoundedRectHandle &hPanel,bool b);
		DLLCLIENT void IsTopRightRound(lua_State *l,WIRoundedRectHandle &hPanel);
		DLLCLIENT void IsTopLeftRound(lua_State *l,WIRoundedRectHandle &hPanel);
		DLLCLIENT void IsBottomLeftRound(lua_State *l,WIRoundedRectHandle &hPanel);
		DLLCLIENT void IsBottomRightRound(lua_State *l,WIRoundedRectHandle &hPanel);
	};

	namespace WIRoundedTexturedRect
	{
		DLLCLIENT void register_class(luabind::class_<WIRoundedTexturedRectHandle,luabind::bases<WITexturedShapeHandle,WIShapeHandle,WIHandle>> &classDef);
		DLLCLIENT void GetRoundness(lua_State *l,WIRoundedTexturedRectHandle &hPanel);
		DLLCLIENT void SetRoundness(lua_State *l,WIRoundedTexturedRectHandle &hPanel,char roundness);
		DLLCLIENT void SetCornerSize(lua_State *l,WIRoundedTexturedRectHandle &hPanel,float cornerSize);
		DLLCLIENT void GetCornerSize(lua_State *l,WIRoundedTexturedRectHandle &hPanel);
		DLLCLIENT void SetRoundTopRight(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b);
		DLLCLIENT void SetRoundTopLeft(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b);
		DLLCLIENT void SetRoundBottomLeft(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b);
		DLLCLIENT void SetRoundBottomRight(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b);
		DLLCLIENT void IsTopRightRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel);
		DLLCLIENT void IsTopLeftRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel);
		DLLCLIENT void IsBottomLeftRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel);
		DLLCLIENT void IsBottomRightRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel);
	};

	namespace WIScrollBar
	{
		DLLCLIENT void register_class(luabind::class_<WIScrollBarHandle,WIHandle> &classDef);
		DLLCLIENT void GetScrollAmount(lua_State *l,WIScrollBarHandle &hPanel);
		DLLCLIENT void SetScrollAmount(lua_State *l,WIScrollBarHandle &hPanel,int am);
		DLLCLIENT void GetScrollOffset(lua_State *l,WIScrollBarHandle &hPanel);
		DLLCLIENT void SetScrollOffset(lua_State *l,WIScrollBarHandle &hPanel,int offset);
		DLLCLIENT void AddScrollOffset(lua_State *l,WIScrollBarHandle &hPanel,int offset);
		DLLCLIENT void SetUp(lua_State *l,WIScrollBarHandle &hPanel,int numElementsListed,int numElementsTotal);
		DLLCLIENT void SetHorizontal(lua_State *l,WIScrollBarHandle &hPanel,bool b);
		DLLCLIENT void IsHorizontal(lua_State *l,WIScrollBarHandle &hPanel);
		DLLCLIENT void IsVertical(lua_State *l,WIScrollBarHandle &hPanel);
	};

	namespace WIIcon
	{
		DLLCLIENT void register_class(luabind::class_<WIIconHandle,luabind::bases<WITexturedShapeHandle,WIShapeHandle,WIHandle>> &classDef);
		DLLCLIENT void SetClipping(lua_State *l,WIIconHandle &hIcon,uint32_t xStart,uint32_t yStart,uint32_t width,uint32_t height);
	};

	namespace WISilkIcon
	{
		DLLCLIENT void register_class(luabind::class_<WISilkIconHandle,luabind::bases<WIIconHandle,WITexturedShapeHandle,WIShapeHandle,WIHandle>> &classDef);
		DLLCLIENT void SetIcon(lua_State *l,WISilkIconHandle &hIcon,std::string icon);
	};

	namespace WIDropDownMenu
	{
		DLLCLIENT void register_class(luabind::class_<WIDropDownMenuHandle,luabind::bases<WITextEntryHandle,WIHandle>> &classDef);
		DLLCLIENT void SelectOptionByText(lua_State *l,WIDropDownMenuHandle &hDm,const std::string &text);
		DLLCLIENT void ClearOptions(lua_State *l,WIDropDownMenuHandle &hDm);
		DLLCLIENT void SelectOption(lua_State *l,WIDropDownMenuHandle &hDm,unsigned int idx);
		DLLCLIENT void SelectOption(lua_State *l,WIDropDownMenuHandle &hDm,const std::string &option);
		DLLCLIENT void GetText(lua_State *l,WIDropDownMenuHandle &hDm);
		DLLCLIENT void GetValue(lua_State *l,WIDropDownMenuHandle &hDm);
		DLLCLIENT void SetText(lua_State *l,WIDropDownMenuHandle &hDm,std::string text);
		DLLCLIENT void GetOptionText(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx);
		DLLCLIENT void GetOptionValue(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx);
		DLLCLIENT void SetOptionText(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx,const std::string &text);
		DLLCLIENT void SetOptionValue(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx,const std::string &val);
		DLLCLIENT void GetOptionCount(lua_State *l,WIDropDownMenuHandle &hDm);
		DLLCLIENT void AddOption(lua_State *l,WIDropDownMenuHandle &hDm,std::string option);
		DLLCLIENT void AddOption(lua_State *l,WIDropDownMenuHandle &hDm,std::string option,const std::string &optionValue);
		DLLCLIENT void OpenMenu(lua_State *l,WIDropDownMenuHandle &hDm);
		DLLCLIENT void CloseMenu(lua_State *l,WIDropDownMenuHandle &hDm);
		DLLCLIENT void ToggleMenu(lua_State *l,WIDropDownMenuHandle &hDm);
		DLLCLIENT void IsMenuOpen(lua_State *l,WIDropDownMenuHandle &hDm);
	};

	namespace WICheckbox
	{
		DLLCLIENT void register_class(luabind::class_<WICheckboxHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef);
		DLLCLIENT void SetChecked(lua_State *l,WICheckboxHandle &hCheckBox,bool bChecked);
		DLLCLIENT void IsChecked(lua_State *l,WICheckboxHandle &hCheckBox);
		DLLCLIENT void Toggle(lua_State *l,WICheckboxHandle &hCheckBox);
	};

	namespace WIArrow
	{
		DLLCLIENT void register_class(luabind::class_<WIArrowHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef);
		DLLCLIENT void SetDirection(lua_State *l,WIArrowHandle &hArrow,uint32_t dir);
	};

	namespace WITransformable
	{
		DLLCLIENT void register_class(luabind::class_<WITransformableHandle,WIHandle> &classDef);
		DLLCLIENT void SetDraggable(lua_State *l,WITransformableHandle &hTransformable,bool b);
		DLLCLIENT void SetResizable(lua_State *l,WITransformableHandle &hTransformable,bool b);
		DLLCLIENT void IsDraggable(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void IsResizable(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void SetMinWidth(lua_State *l,WITransformableHandle &hTransformable,int32_t width);
		DLLCLIENT void SetMinHeight(lua_State *l,WITransformableHandle &hTransformable,int32_t height);
		DLLCLIENT void SetMinSize(lua_State *l,WITransformableHandle &hTransformable,int32_t width,int32_t height);
		DLLCLIENT void SetMinSize(lua_State *l,WITransformableHandle &hTransformable,const Vector2i &size);
		DLLCLIENT void GetMinWidth(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void GetMinHeight(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void GetMinSize(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void SetMaxWidth(lua_State *l,WITransformableHandle &hTransformable,int32_t width);
		DLLCLIENT void SetMaxHeight(lua_State *l,WITransformableHandle &hTransformable,int32_t height);
		DLLCLIENT void SetMaxSize(lua_State *l,WITransformableHandle &hTransformable,int32_t width,int32_t height);
		DLLCLIENT void SetMaxSize(lua_State *l,WITransformableHandle &hTransformable,const Vector2i &size);
		DLLCLIENT void GetMaxWidth(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void GetMaxHeight(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void GetMaxSize(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void Close(lua_State *l,WITransformableHandle &hTransformable);
		DLLCLIENT void GetDragArea(lua_State *l,WITransformableHandle &hTransformable);
	};

	namespace WIGridPanel
	{
		DLLCLIENT void register_class(luabind::class_<WIGridPanelHandle,luabind::bases<WITableHandle,WIHandle>> &classDef);
		DLLCLIENT void AddItem(lua_State *l,WIGridPanelHandle &hGridPanel,WIHandle &hEl,uint32_t row,uint32_t col);
		DLLCLIENT void GetColumnCount(lua_State *l,WIGridPanelHandle &hGridPanel);
	};

	namespace WITreeList
	{
		DLLCLIENT void register_class(luabind::class_<WITreeListHandle,luabind::bases<WITableHandle,WIHandle>> &classDef);
		DLLCLIENT void ExpandAll(lua_State *l,WITreeListHandle &hTreeList);
		DLLCLIENT void CollapseAll(lua_State *l,WITreeListHandle &hTreeList);
		DLLCLIENT void GetRootItem(lua_State *l,WITreeListHandle &hTreeList);
	};

	namespace WITreeListElement
	{
		DLLCLIENT void register_class(luabind::class_<WITreeListElementHandle,luabind::bases<WITableRowHandle,WIHandle>> &classDef);
		DLLCLIENT void AddItem(lua_State *l,WITreeListElementHandle &hElement,const std::string &text);
		DLLCLIENT void Expand(lua_State *l,WITreeListElementHandle &hTreeList,bool bAll);
		DLLCLIENT void Expand(lua_State *l,WITreeListElementHandle &hTreeList);
		DLLCLIENT void Collapse(lua_State *l,WITreeListElementHandle &hTreeList,bool bAll);
		DLLCLIENT void Collapse(lua_State *l,WITreeListElementHandle &hTreeList);
		DLLCLIENT void GetItems(lua_State *l,WITreeListElementHandle &hTreeList);
	};

	namespace WIContainer
	{
		DLLCLIENT void register_class(luabind::class_<WIContainerHandle,WIHandle> &classDef);
		DLLCLIENT void SetPadding(lua_State *l,WIContainerHandle &hContainer,int32_t padding);
		DLLCLIENT void SetPadding(lua_State *l,WIContainerHandle &hContainer,int32_t top,int32_t right,int32_t bottom,int32_t left);
		DLLCLIENT void SetPaddingTop(lua_State *l,WIContainerHandle &hContainer,int32_t top);
		DLLCLIENT void SetPaddingRight(lua_State *l,WIContainerHandle &hContainer,int32_t right);
		DLLCLIENT void SetPaddingBottom(lua_State *l,WIContainerHandle &hContainer,int32_t bottom);
		DLLCLIENT void SetPaddingLeft(lua_State *l,WIContainerHandle &hContainer,int32_t left);
		DLLCLIENT void GetPadding(lua_State *l,WIContainerHandle &hContainer);
		DLLCLIENT void GetPaddingTop(lua_State *l,WIContainerHandle &hContainer);
		DLLCLIENT void GetPaddingRight(lua_State *l,WIContainerHandle &hContainer);
		DLLCLIENT void GetPaddingBottom(lua_State *l,WIContainerHandle &hContainer);
		DLLCLIENT void GetPaddingLeft(lua_State *l,WIContainerHandle &hContainer);
	};

	namespace WITable
	{
		DLLCLIENT void register_class(luabind::class_<WITableHandle,luabind::bases<WIContainerHandle,WIHandle>> &classDef);
		DLLCLIENT void SetRowHeight(lua_State *l,WITableHandle &hTable,int height);
		DLLCLIENT void SetSelectable(lua_State *l,WITableHandle &hTable,uint32_t mode);
		DLLCLIENT void IsSelectable(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void SetColumnWidth(lua_State *l,WITableHandle &hTable,uint32_t colId,uint32_t width);
		DLLCLIENT void AddRow(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void AddHeaderRow(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void GetRowCount(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void SetSortable(lua_State *l,WITableHandle &hTable,bool b);
		DLLCLIENT void IsSortable(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void SetScrollable(lua_State *l,WITableHandle &hTable,bool b);
		DLLCLIENT void IsScrollable(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void Clear(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void GetRow(lua_State *l,WITableHandle &hTable,uint32_t rowId);
		DLLCLIENT void GetSelectedRows(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void GetRows(lua_State *l,WITableHandle &hTable);
		DLLCLIENT void RemoveRow(lua_State *l,WITableHandle &hTable,uint32_t rowIdx);
	};

	namespace WITableRow
	{
		DLLCLIENT void register_class(luabind::class_<WITableRowHandle,luabind::bases<WIContainerHandle,WIHandle>> &classDef);
		DLLCLIENT void Select(lua_State *l,WITableRowHandle &hRow);
		DLLCLIENT void Deselect(lua_State *l,WITableRowHandle &hRow);
		DLLCLIENT void IsSelected(lua_State *l,WITableRowHandle &hRow);
		DLLCLIENT void SetCellWidth(lua_State *l,WITableRowHandle &hRow,uint32_t column,uint32_t width);
		DLLCLIENT void SetValue(lua_State *l,WITableRowHandle &hRow,uint32_t column,const std::string &value);
		DLLCLIENT void GetValue(lua_State *l,WITableRowHandle &hRow,uint32_t column);
		DLLCLIENT void InsertElement(lua_State *l,WITableRowHandle &hRow,uint32_t column,WIHandle hElement);
		DLLCLIENT void GetCellCount(lua_State *l,WITableRowHandle &hRow);
		DLLCLIENT void GetCell(lua_State *l,WITableRowHandle &hRow,uint32_t cellId);
	};

	namespace WITableCell
	{
		DLLCLIENT void register_class(luabind::class_<WITableCellHandle,luabind::bases<WIContainerHandle,WIHandle>> &classDef);
		DLLCLIENT void GetFirstElement(lua_State *l,WITableCellHandle &hCell);
		DLLCLIENT void GetColSpan(lua_State *l,WITableCellHandle &hCell);
		DLLCLIENT void GetRowSpan(lua_State *l,WITableCellHandle &hCell);
		DLLCLIENT void SetColSpan(lua_State *l,WITableCellHandle &hCell,int32_t colSpan);
		DLLCLIENT void SetRowSpan(lua_State *l,WITableCellHandle &hCell,int32_t rowSpan);
	};

	namespace WIFrame
	{
		DLLCLIENT void register_class(luabind::class_<WIFrameHandle,luabind::bases<WITransformableHandle,WIHandle>> &classDef);
		DLLCLIENT void SetTitle(lua_State *l,WIFrameHandle &hFrame,std::string title);
		DLLCLIENT void GetTitle(lua_State *l,WIFrameHandle &hFrame);
	};
};

#endif