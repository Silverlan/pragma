// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:console.line_noise;

export {
    #ifdef __linux__

    namespace pragma::console::impl {
        void init_linenoise();
        void close_linenoise();
        void update_linenoise();
        bool is_linenoise_enabled();
    };

    #endif
};
