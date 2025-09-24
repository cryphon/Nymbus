cmake_minimum_required(VERSION 3.20)
project(Nymbus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library(nym
    src/tuntap.cpp
    src/util.cpp
)

target_include_directories(nym PUBLIC include)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nym)


# --- Linux-only helpers for TAP + caps ---
if(UNIX AND NOT APPLE)
  # Choose interface/bind address here (or make them CACHE vars)
  set(NYMBUS_TAP_NAME "v6tap")
  set(NYMBUS_IPV6     "2001:db8::1/64")

  # Path to the built app
  set(NYMBUS_APP "$<TARGET_FILE:${PROJECT_NAME}>")

  # Create & configure TAP (requires sudo once per boot/session)
  add_custom_target(setup_tap
    COMMAND sudo ip tuntap add dev ${NYMBUS_TAP_NAME} mode tap user $ENV{USER}
    COMMAND sudo ip link set ${NYMBUS_TAP_NAME} up
    COMMAND sudo ip addr add ${NYMBUS_IPV6} dev ${NYMBUS_TAP_NAME}
    BYPRODUCTS
    COMMENT "Creating ${NYMBUS_TAP_NAME}, bringing it up, assigning ${NYMBUS_IPV6}"
    VERBATIM
  )

  # Optional: remove TAP
  add_custom_target(teardown_tap
    COMMAND sudo ip link set ${NYMBUS_TAP_NAME} down || true
    COMMAND sudo ip tuntap del dev ${NYMBUS_TAP_NAME} mode tap || true
    COMMENT "Removing ${NYMBUS_TAP_NAME}"
    VERBATIM
  )

  # Grant binary the minimal caps so it can open /dev/net/tun and raw sockets without sudo
  add_custom_target(grant_caps
    COMMAND sudo setcap cap_net_admin,cap_net_raw+ep ${NYMBUS_APP}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Granting ${PROJECT_NAME} CAP_NET_ADMIN and CAP_NET_RAW"
    VERBATIM
  )

  # Run without sudo (assumes setup_tap + grant_caps done at least once)
  add_custom_target(run_app
    COMMAND ${NYMBUS_APP}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running ${PROJECT_NAME} without sudo (after setup_tap & grant_caps)"
    VERBATIM
  )
endif()

