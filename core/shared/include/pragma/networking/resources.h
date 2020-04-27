/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include <string>
#include <vector>
#include "pragma/networkdefinitions.h"

#define FSYS_SEARCH_RESOURCES 4'096

DLLNETWORK bool IsValidResource(std::string res);