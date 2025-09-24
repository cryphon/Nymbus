#pragma once
#include <string>
#include <vector>
#include <cstddef>

namespace net {

enum class Mode {
    Tun,
    Tap,
};

class TunTap {
public:
    TunTap(const std::string& ifname, Mode mode, bool no_pi = true);
    ~TunTap();

    TunTap(const TunTap&) = delete;
    TunTap& operator=(const TunTap&) = delete;
    TunTap(TunTap&&) noexcept;
    TunTap& operator=(TunTap&&) noexcept;

    int fd() const noexcept { return fd_; }
    std::string name() const { return name_; }

    std::size_t read(std::vector<std::byte>& buf);
    std::size_t write(const std::vector<std::byte>& buf);

private:
    int fd_{-1};
    std::string name_;
};

}
