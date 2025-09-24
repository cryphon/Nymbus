#include "TunTap.hpp"
#include "Demux.hpp"
#include <iostream>

extern void hexdump(const void*, std::size_t);

int main(void) {
    try {
        net::TunTap dev("v6tap", net::Mode::Tap); 
        std::vector<std::byte> buf(65536);


        net::Ipv6Handler ipv6;
        net::UnknownHandler unk;
        net::Demux demux{ipv6, unk};

        while(true) {
            std::size_t n = dev.read(buf);

            // classify packet
            net::process_frame(reinterpret_cast<const std::uint8_t *>(buf.data()), n, demux);

            std::cout << "--- packet: " << n << " bytes ---\n";
            hexdump(buf.data(), (n > 256) ? 256 : n);
        }
    }
    catch(const std::exception& ex) {
        std::cerr << "Fatal " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
