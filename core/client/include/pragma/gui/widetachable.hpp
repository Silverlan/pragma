/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIDETACHABLE_HPP__
#define __WIDETACHABLE_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WIDetachable
	: public WIBase
{
public:
	WIDetachable();
	virtual ~WIDetachable() override;
	virtual void OnRemove() override;
	void Detach();
	void Reattach();
protected:
	struct DetachedWindow
	{
		~DetachedWindow();
		void Clear();
		WIHandle reattachElement {};
		std::shared_ptr<prosper::Window> window;
		WIHandle detachedBg {};
		Vector2i origPos;
		Vector2i origSize;
		std::optional<std::array<float,4>> origAnchor;
	};
	std::unique_ptr<DetachedWindow> m_detachedWindow = nullptr;
};

#endif
