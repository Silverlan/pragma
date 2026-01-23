// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#pragma once

// TODO: Remove this file once this workaround is no longer needed!

#ifdef WINDOWS_CLANG_COMPILER_FIX

#define REGISTER_COMPONENT_EVENT(EVENT) \
    DLLNETWORK ComponentEventId &GET_##EVENT() { \
        static ComponentEventId eventId = INVALID_COMPONENT_ID; \
        return eventId; \
    }

#else

#define REGISTER_COMPONENT_EVENT(EVENT) \
    CLASS_ENUM_COMPAT ComponentEventId EVENT = INVALID_COMPONENT_ID;

#endif
