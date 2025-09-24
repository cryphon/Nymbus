#include "TunTap.hpp"

#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

namespace net {
TunTap::TunTap(const std::string& ifname, Mode mode, bool no_pi) {
    fd_ = open("/dev/net/tun", O_RDWR);
    if(fd_ < 0) {
        throw std::runtime_error("Failed to open /dev/net/tun: " + std::string(strerror(errno)));
    }

    struct ifreq ifr {};
    ifr.ifr_flags = (mode == Mode::Tun ? IFF_TUN : IFF_TAP);
    if(no_pi) ifr.ifr_flags |= IFF_NO_PI;


    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ - 1);

    if(ioctl(fd_, TUNSETIFF, (void*)&ifr) < 0) {
        close(fd_);
        throw std::runtime_error("ioctl(TUNSETIFF) failed: " + std::string(strerror(errno)));
    }

    name_ = ifr.ifr_name;
}

TunTap::~TunTap() {
    if (fd_ >= 0) close(fd_);
}

TunTap::TunTap(TunTap&& other) noexcept {
    fd_ = other.fd_;
    name_ = std::move(other.name_);
    other.fd_ = -1;
}

TunTap& TunTap::operator=(TunTap&& other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) close(fd_);
        fd_ = other.fd_;
        name_ = std::move(other.name_);
        other.fd_ = -1;
    }
    return *this;
}

std::size_t TunTap::read(std::vector<std::byte>& buf) {
    ssize_t n = ::read(fd_, buf.data(), buf.size());
    if(n < 0) {
        throw std::runtime_error("read() failed: " + std::string(strerror(errno)));
    }
    return static_cast<std::size_t>(n);
}

std::size_t TunTap::write(const std::vector<std::byte>& buf) {
    ssize_t n = ::write(fd_, buf.data(), buf.size());
    if(n < 0) {
        throw std::runtime_error("write() failed: " + std::string(strerror(errno)));
    }
    return static_cast<std::size_t>(n);
}

}
