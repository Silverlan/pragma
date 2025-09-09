// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"

export module pragma.entities.handle_wrapper;

export namespace pragma {
    // Note: Using EntityHandle can cause a LNK1169 linker error with msvc due to a compiler bug.
    // If that happens, we can use EntityHandleWrapper as a temporary work-around until that bug is fixed.
    struct DLLNETWORK EntityHandleWrapper {
        EntityHandleWrapper(BaseEntity *ent);
        ~EntityHandleWrapper();
        operator EntityHandle&();
        operator const EntityHandle&() const;
    private:
        EntityHandle *handle = nullptr;
    };
};
