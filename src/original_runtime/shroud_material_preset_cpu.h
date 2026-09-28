#pragma once

#include "WW3D2/shader.h"

namespace zh::original_runtime::detail {

// Keep the authored debug fog choice testable without changing the release
// profile or exposing a general shader selector to the engine.
inline const ShaderClass& shroud_material_preset(bool debug_profile,bool fog_on) noexcept
{
    return debug_profile && fog_on ? ShaderClass::_PresetAlphaSpriteShader
        : ShaderClass::_PresetMultiplicativeSpriteShader;
}

}
