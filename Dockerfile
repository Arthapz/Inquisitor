FROM gcc:latest

WORKDIR /usr/src/

RUN apt-get update && apt-get install -y cmake git meson zip libboost-dev libboost-system-dev libboost-thread-dev libboost-coroutine-dev

RUN git clone https://github.com/microsoft/vcpkg.git --depth 1
RUN vcpkg/bootstrap-vcpkg.sh
RUN vcpkg/vcpkg install glm gli fmt ms-gsl range-v3 nlohmann-json spirv-cross vulkan-headers

RUN git clone https://gitlab.com/tapzcrew/inquisitor-cpp.git --depth 1 inquisitor --recursive

WORKDIR /usr/src/inquisitor

RUN export NINJA_STATUS=$'\x1b[32m[%f/%t] \x1b[0m'
RUN export CLICOLOR_FORCE="ninja mp"

RUN meson builddir --buildtype release --cmake-prefix-path /usr/src/vcpkg/installed/x64-linux
RUN ninja -C builddir -j9

COPY settings.json /root/
COPY facts.txt /root/

WORKDIR /usr/src/inquisitor/builddir

RUN cp ./inquisitor /root/inquisitor

RUN rm -Rf /usr/src/*
RUN rm -Rf /root/.cache/vcpkg

CMD ["./inquisitor"]