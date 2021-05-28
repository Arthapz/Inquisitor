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

/////////// - StormKit::render - ///////////
#include <storm/render/Fwd.hpp>
#include <storm/render/core/Types.hpp>

/////////// - WebP - ///////////
#include <webp/mux_types.h>

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
    using ErrorString = storm::core::NamedType<std::string, struct Tag>;

    std::optional<std::string> getAttachedGlsl(const json &msg) const;
    std::optional<json> getAttachedJson(const json &msg) const;

    std::regex m_glsl_regex;
    std::regex m_json_regex;

    std::optional<std::pair<std::string, std::string>> compileShader(std::string_view glsl, std::vector<storm::render::SpirvID> &output, std::size_t texture_count);

    void singleFrame(std::vector<std::string> textures, std::string_view channel_id, std::string_view glsl, storm::core::Extentu extent);
    void multipleFrame(std::vector<std::string>textures, storm::core::UInt32 frame_count, std::string_view channel_id, std::string_view glsl, storm::core::Extentu extent);

    std::variant<std::string, ErrorString> render(std::span<const storm::render::SpirvID> spirv, std::span<const std::string> textures, storm::core::Extentu extent);

    storm::render::InstanceOwnedPtr m_instance;

    storm::render::DeviceOwnedPtr m_device;

    storm::render::QueueConstPtr m_queue;

    storm::render::ShaderOwnedPtr m_vertex_shader;

    bool m_has_blit = true;

    std::atomic_bool m_is_currently_rendering = false;

    storm::core::Extentu m_max_extent;
};
