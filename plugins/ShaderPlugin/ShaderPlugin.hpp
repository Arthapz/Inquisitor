// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <string_view>
#include <random>
#include <chrono>
#include <vector>
#include <regex>
#include <span>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

/////////// - StormKit::core - ///////////
#include <storm/core/Types.hpp>
#include <storm/core/HashMap.hpp>
#include <storm/core/NamedType.hpp>

/////////// - StormKit::image - ///////////
#include <storm/image/Fwd.hpp>

/////////// - StormKit::render - ///////////
#include <storm/render/Fwd.hpp>
#include <storm/render/core/Types.hpp>

/////////// - FFMpeg - ///////////
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

STORMKIT_RAII_CAPSULE_PP(AVCodecContext, AVCodecContext, avcodec_free_context);
STORMKIT_RAII_CAPSULE(AVFormatContext, AVFormatContext, avformat_free_context);

class ShaderPlugin final: public PluginInterface {
  public:
    ShaderPlugin();
    ~ShaderPlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> commands() const override;
    [[nodiscard]] std::string_view help() const override;
    void onCommand(std::string_view command, const json &msg) override;

  protected:
    void initialize(const json &options) override;

  private:
    using ErrorString = stormkit::core::NamedType<std::string, struct Tag>;

    std::optional<std::string> getAttachedGlsl(const json &msg) const;
    std::optional<json> getAttachedJson(const json &msg) const;

    std::regex m_glsl_regex;
    std::regex m_json_regex;

    std::optional<std::pair<std::string, std::string>> compileShader(std::string_view glsl, std::vector<stormkit::render::SpirvID> &output, std::size_t texture_count);

    void singleFrame(std::vector<std::string> textures, std::string_view channel_id, std::string_view glsl, const stormkit::core::Extentu &extent);
    void multipleFrame(std::vector<std::string>textures, stormkit::core::UInt32 frame_count, stormkit::core::UInt32 fps, std::string_view channel_id, std::string_view glsl, const stormkit::core::Extentu &extent);

    std::variant<std::pair<stormkit::core::ByteArray, stormkit::core::UInt32>, ErrorString> render(std::span<const stormkit::render::SpirvID> spirv, std::span<const stormkit::image::Image> textures, stormkit::core::Extentu extent, stormkit::core::UInt32 frame, float time);

    std::variant<stormkit::core::ByteArray, ErrorString> encode(std::span<std::pair<stormkit::core::ByteArray, stormkit::core::UInt32>> data, const stormkit::core::Extentu &extent, stormkit::core::UInt32 fps);

    stormkit::render::InstanceOwnedPtr m_instance;

    stormkit::render::DeviceOwnedPtr m_device;

    stormkit::render::QueueConstPtr m_queue;

    stormkit::render::ShaderOwnedPtr m_vertex_shader;

    bool m_has_blit = true;

    std::atomic_bool m_is_currently_rendering = false;

    stormkit::core::Extentu m_max_extent;

    AVFormatContextScoped m_avformat_context;
};
