# one-time per session (you’ll be prompted for sudo)
cmake -S . -B build
cmake --build build

cmake --build build --target setup_tap
cmake --build build --target grant_caps

# now you can run rootless:
cmake --build build --target run_app

# when done:
cmake --build build --target teardown_tap

