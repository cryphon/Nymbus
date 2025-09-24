#pragma once
#include <array>
#include <cerrno>
#include <iostream>
#include <netinet/in.h>
#include <net/ethernet.h>


namespace net  {


// zero-copy
struct FrameView {
    const std::uint8_t* data{nullptr};
    std::size_t len{0};

    bool empty() const noexcept { return data == nullptr || len == 0; }
};



#pragma pack(push, 1)
struct EthHeader {
    std::uint8_t dst[6];
    std::uint8_t src[6];
    std::uint16_t ether_type; // big-endian
};
#pragma pack(pop)

inline bool parse_eth(FrameView f, const EthHeader*& eh) {
    if(f.len < sizeof(EthHeader)) return false;
    eh = reinterpret_cast<const EthHeader*>(f.data);
    return true;
}

// ------------------- Handlers -----------------
struct IHandler {
    virtual ~IHandler() = default;
    virtual void handle(FrameView frame, const EthHeader& eth) = 0;
    virtual std::string_view name() const = 0;
};

struct Ipv6Handler final : IHandler {
    void handle(FrameView frame, const EthHeader&) override {
        std::cout << "[IPv6] frame len=" << frame.len << "\n";
    }
    std::string_view name() const override { return "IPv6Handler"; }
};

struct UnknownHandler final : IHandler {
    void handle(FrameView frame, const EthHeader& eth) override {
        std::cout << "[UNKW] EtherType=0x" << std::hex << ntohs(eth.ether_type)
                  << std::dec << " len=" << frame.len << "\n";
    }
    std::string_view name() const override { return "UnknownHandler"; }
};

class Demux {
public:
    Demux(IHandler& ipv6, IHandler& unk)
    : ipv6_(ipv6), unk_(unk) {}

    void ingest(FrameView f) {
        const EthHeader* eth = nullptr;
        if (!parse_eth(f, eth)) {
            std::cout << "[CLS ] drop runt (" << f.len << " bytes)\n";
            return;
        }

        const auto et = ntohs(eth->ether_type);
        switch (et) {
            case ETH_P_IPV6: ipv6_.handle(f, *eth); break;
            // add other types here
            default: unk_.handle(f, *eth);  break;
        }
    }

private:
    IHandler& ipv6_;
    IHandler& unk_;
};


inline void process_frame(const std::uint8_t* buf, std::size_t n, Demux& dem) {
    dem.ingest(FrameView{buf, n});
}

}
