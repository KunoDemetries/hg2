#pragma once
#include "hg/gs_acceleration.hpp"
#include <memory>

namespace hg {
// Requires a current OpenGL 4.3 context, owned by the calling thread throughout.
std::unique_ptr<GsSpriteAccelerator> make_gl_sprite_accelerator(bool resident=false,bool triangles=false);
}
