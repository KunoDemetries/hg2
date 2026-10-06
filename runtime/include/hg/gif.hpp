#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hg {

// Structural GIF decoding.  GS register semantics deliberately live behind this
// boundary: no unimplemented register is discarded by the transport parser.
enum class GifTransferKind : std::uint8_t { Prim, Packed, Reglist, Image };

struct GifTransfer {
    GifTransferKind kind{};
    std::uint8_t descriptor{};
    std::uint64_t low{};
    std::uint64_t high{};
    bool reset_q=false;
};

struct GifPacket {
    std::vector<GifTransfer> transfers;
    std::size_t bytes{};
};

namespace detail {
inline std::uint64_t gif_u64le(const std::uint8_t* bytes) {
    std::uint64_t value = 0;
    for (unsigned i = 0; i != 8; ++i) value |= std::uint64_t(bytes[i]) << (i * 8);
    return value;
}

inline void gif_need(std::size_t available, std::size_t offset, std::size_t count,
                     const char* what) {
    if (offset > available || count > available - offset)
        throw std::runtime_error(std::string("truncated GIF ") + what);
}
} // namespace detail

// Return the byte size of the first complete EOP-terminated packet. A missing
// suffix is not an error here: DMA paths may deliver a packet over many qwords.
inline std::optional<std::size_t> gif_packet_size(const std::uint8_t* bytes, std::size_t size) {
    if (!bytes && size) throw std::runtime_error("null GIF input");
    std::size_t offset = 0;
    while (true) {
        if (offset > size || size - offset < 16) return std::nullopt;
        const std::uint64_t tag_lo = detail::gif_u64le(bytes + offset);
        offset += 16;
        const std::size_t nloop = tag_lo & 0x7fffu;
        const bool eop = (tag_lo & (1ull << 15)) != 0;
        if (nloop == 0) { if (eop) return offset; continue; }
        const unsigned format = unsigned((tag_lo >> 58) & 3u);
        const std::size_t encoded_nreg = (tag_lo >> 60) & 0xfu;
        const std::size_t nreg = encoded_nreg ? encoded_nreg : 16;
        const std::size_t data_bytes = format >= 2 ? nloop * 16 :
            format == 0 ? nloop * nreg * 16 : ((nloop * nreg + 1) / 2) * 16;
        if (data_bytes > size - offset) return std::nullopt;
        offset += data_bytes;
        if (eop) return offset;
    }
}

// Decode one complete EOP-terminated GIF packet.  The caller owns transport
// scheduling and GS interpretation. IMAGE payloads are retained as raw
// quadwords so the GS HWREG transfer endpoint can consume them in order.
inline GifPacket decode_gif_packet(const std::uint8_t* bytes, std::size_t size) {
    if (!bytes && size) throw std::runtime_error("null GIF input");
    const auto complete_size = gif_packet_size(bytes, size);
    if (!complete_size) throw std::runtime_error("truncated GIF packet");
    if (*complete_size != size) throw std::runtime_error("trailing GIF packet data");
    GifPacket packet;
    packet.transfers.reserve(size / 16); // Capacity hint: one transfer per qword (REGLIST may use two).
    std::size_t offset = 0;
    bool done = false;
    bool reset_q_pending = true;
    const auto append=[&](GifTransferKind kind,std::uint8_t descriptor,std::uint64_t low,std::uint64_t high) {
        packet.transfers.push_back({kind,descriptor,low,high,reset_q_pending});
        reset_q_pending=false;
    };
    while (!done) {
        detail::gif_need(size, offset, 16, "tag");
        const std::uint64_t tag_lo = detail::gif_u64le(bytes + offset);
        const std::uint64_t descriptors = detail::gif_u64le(bytes + offset + 8);
        offset += 16;
        const std::size_t nloop = tag_lo & 0x7fffu;
        const bool eop = (tag_lo & (1ull << 15)) != 0;
        const bool pre = (tag_lo & (1ull << 46)) != 0;
        const unsigned format = unsigned((tag_lo >> 58) & 3u);
        const std::size_t encoded_nreg = (tag_lo >> 60) & 0xfu;
        const std::size_t nreg = encoded_nreg ? encoded_nreg : 16;
        reset_q_pending = true;
        if (nloop == 0) { done = eop; continue; }
        // PRE/PRIM are ignored in REGLIST and IMAGE mode (EE User's Manual 7.4.1/7.5.1).
        if (pre && format == 0) append(GifTransferKind::Prim, 0,(tag_lo >> 47) & 0x7ffu, 0);
        if (format >= 2) {
            detail::gif_need(size, offset, nloop * 16, "IMAGE data");
            for (std::size_t i = 0; i != nloop; ++i) {
                append(GifTransferKind::Image,0,detail::gif_u64le(bytes + offset),
                       detail::gif_u64le(bytes + offset + 8));
                offset += 16;
            }
            done = eop;
            continue;
        }
        const std::size_t entries = nloop * nreg;
        if (format == 0) {
            detail::gif_need(size, offset, entries * 16, "PACKED data");
            for (std::size_t i = 0; i != entries; ++i) {
                const std::uint8_t descriptor = (descriptors >> ((i % nreg) * 4)) & 0xfu;
                const std::uint64_t low = detail::gif_u64le(bytes + offset);
                const std::uint64_t high = detail::gif_u64le(bytes + offset + 8);
                offset += 16;
                if (descriptor != 0xf)
                    append(GifTransferKind::Packed,descriptor,low,high);
            }
        } else {
            const std::size_t data_bytes = ((entries + 1) / 2) * 16;
            detail::gif_need(size, offset, data_bytes, "REGLIST data");
            for (std::size_t i = 0; i != entries; ++i) {
                const std::uint8_t descriptor = (descriptors >> ((i % nreg) * 4)) & 0xfu;
                const std::uint64_t value = detail::gif_u64le(bytes + offset + (i * 8));
                if (descriptor != 0xf)
                    append(GifTransferKind::Reglist,descriptor,value,0);
            }
            offset += data_bytes;
        }
        done = eop;
    }
    packet.bytes = offset;
    return packet;
}

} // namespace hg
