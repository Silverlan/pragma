#ifndef __C_LDEF_WGUIHANDLES_H__
#define __C_LDEF_WGUIHANDLES_H__

#include "pragma/clientdefinitions.h"
#include <sharedutils/def_handle.h>
#include <wgui/wibase.h>
#include <wgui/wihandle.h>
#include <wgui/types/wishape.h>
#include <wgui/types/wirect.h>
#include <wgui/types/witext.h>
#include <wgui/types/witextentry.h>
#include <wgui/types/wiscrollbar.h>
#include "pragma/gui/wisilkicon.h"
#include "pragma/gui/wiscrollcontainer.h"
#include <wgui/types/widropdownmenu.h>
#include "pragma/gui/wicheckbox.h"
#include "pragma/gui/witable.h"
#include "pragma/gui/wiframe.h"
#include "pragma/gui/witransformable.h"
#include <wgui/types/wiarrow.h>
#include "pragma/gui/witreelist.h"
#include "pragma/gui/wiprogressbar.h"
#include "pragma/gui/wislider.h"
#include "pragma/gui/wigridpanel.h"
#include "pragma/gui/wicontainer.h"
#include "pragma/gui/wiconsole.hpp"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/gui/debug/widebugshadowmap.hpp"
#include "pragma/gui/debug/widebugssao.hpp"
#include "pragma/gui/wisnaparea.hpp"
#include "pragma/gui/wicommandlineentry.hpp"
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIShape,WIShape);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIText,WIText);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WITextEntry,WITextEntry);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITextEntry,WICommandLineEntry,WICommandLineEntry);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIOutlinedRect,WIOutlinedRect);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WILine,WILine);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIScrollBar,WIScrollBar);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIButton,WIButton);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIContainer,WIContainer);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIScrollContainer,WIScrollContainer);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIProgressBar,WIProgressBar);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WITransformable,WITransformable);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITransformable,WIFrame,WIFrame);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIDebugDepthTexture,WIDebugDepthTexture);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIDebugShadowMap,WIDebugShadowMap);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WISnapArea,WISnapArea);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIConsole,WIConsole);

DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIContainer,WITable,WITable);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIContainer,WITableRow,WITableRow);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIContainer,WITableCell,WITableCell);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITextEntry,WINumericEntry,WINumericEntry);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITextEntry,WIDropDownMenu,WIDropDownMenu);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIProgressBar,WISlider,WISlider);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITable,WITreeList,WITreeList);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITable,WIGridPanel,WIGridPanel);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITableRow,WITreeListElement,WITreeListElement);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WIArrow,WIArrow);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WICheckbox,WICheckbox);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WIRoundedRect,WIRoundedRect);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WITexturedShape,WITexturedShape);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WITexturedRect,WITexturedRect);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WIIcon,WIIcon);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WIRoundedTexturedRect,WIRoundedTexturedRect);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIIcon,WISilkIcon,WISilkIcon);
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WIDebugSSAO,WIDebugSSAO);

#endif