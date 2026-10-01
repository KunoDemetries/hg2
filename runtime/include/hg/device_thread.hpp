#pragma once
#include "hg/device_link.hpp"
#include <functional>
#include <memory>

namespace hg {
struct Vif1Path;
struct GifPath;
struct GsRegisterState;
// Starts the host device thread that applies DeviceLink commands to the given
// state. `context(true/false)` makes the GS OpenGL context current on / releases
// it from the calling thread; it is invoked on the device thread. The caller's
// thread-local GS accelerators are installed on the device thread.
std::unique_ptr<DeviceLink> start_device_thread(Vif1Path& vif1,GifPath& gif,GsRegisterState& gs,
                                                std::function<void(bool)> context);
}
