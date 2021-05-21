FROM gcc:latest as dependencies

LABEL description = "Dependencies container"

WORKDIR /usr/src/

RUN apt-get update && apt-get install -y cmake git meson unzip zip wget

RUN git clone https://github.com/microsoft/vcpkg.git --depth 1
RUN vcpkg/bootstrap-vcpkg.sh
RUN vcpkg/vcpkg install glm gli fmt ms-gsl range-v3 nlohmann-json spirv-cross vulkan-headers

FROM gcc:latest as build

RUN apt-get update && apt-get install -y cmake git meson unzip zip wget libboost-dev libboost-system-dev libboost-thread-dev libboost-coroutine-dev

WORKDIR /usr/src/

COPY --from=dependencies /usr/src/vcpkg /usr/src/vcpkg

RUN mkdir /usr/src/inquisitor
COPY . /usr/src/inquisitor/

WORKDIR /usr/src/inquisitor

RUN export NINJA_STATUS=$'\x1b[32m[%f/%t] \x1b[0m'
RUN export CLICOLOR_FORCE="ninja mp"

RUN meson builddir --buildtype release --cmake-prefix-path /usr/src/vcpkg/installed/x64-linux -Dstormkit:buildtype=release -Dvcpkg_toolchain_file=/usr/src/vcpkg/installed/x64-linux
RUN ninja -C builddir -j4

WORKDIR /usr/src/inquisitor/builddir
RUN cp ./inquisitor /root/inquisitor

RUN mkdir /root/plugins
RUN find plugins/ -name "*.so" -type f -exec cp {} /root/plugins \;

FROM alpine:latest as runtime

RUN apk update && apk add --no-cache \ 
    libstdc++ boost

COPY --from=build /root /root

WORKDIR /root

CMD ["./inquisitor"]
