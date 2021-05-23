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

/////////// - StormKit::render - ///////////
#include <storm/render/Fwd.hpp>
#include <storm/render/core/Types.hpp>

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
    std::regex m_regex;

    std::optional<std::string> compileShader(std::string_view glsl, std::vector<storm::render::SpirvID> &output);

    std::string render(std::span<const storm::render::SpirvID> spirv);

    storm::render::InstanceOwnedPtr m_instance;
    storm::render::OffscreenSurfaceOwnedPtr m_surface;

    storm::render::DeviceOwnedPtr m_device;

    storm::render::QueueConstPtr m_queue;

    storm::render::TextureViewOwnedPtr m_surface_view;

    storm::render::ShaderOwnedPtr m_vertex_shader;

    bool m_has_blit = true;

    std::atomic_bool m_is_currently_rendering = false;
};
