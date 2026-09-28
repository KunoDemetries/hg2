#pragma once
#include <cstdint>

namespace hg {
struct State;
// Used only inside an already synchronous VIF service burst. Returns the number
// of inert input qwords appended, or zero with no state change. Never completes
// a VIF command, a DMA payload or a tag; the original one-qword pump owns those.
std::uint32_t buffer_incomplete_vif_direct(State& state);
}
