// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#if defined(__linux) && defined(__clang__)
//this file is included solely beacuse some symbols never get emitted.

#include <shader/prosper_shader.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <shader/prosper_shader_t.hpp>

template const std::shared_ptr<prosper::IRenderPass> &prosper::ShaderGraphics::GetRenderPass<prosper::ShaderBlurBase>(prosper::IPrContext &, uint32_t);
#endif
