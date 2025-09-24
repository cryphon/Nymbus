#include <cstddef>
#include <cstdio>

void hexdump(const void* data, std::size_t len) {
    auto* p = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < len; i += 16) {
        std::printf("%04zx: ", i);
        for (std::size_t j = 0; j < 16; ++j) {
            if (i + j < len) std::printf("%02x ", p[i+j]);
            else std::printf("   ");
        }
        std::printf("  ");
        for (std::size_t j = 0; j < 16 && i + j < len; ++j) {
            unsigned char c = p[i+j];
            std::putchar((c >= 0x20 && c <= 0x7e) ? c : '.');
        }
        std::putchar('\n');
    }
}
