// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - ShaderPlugin - ///////////
#include "ShaderPlugin.hpp"
#include "Log.hpp"

/////////// - STL - ///////////
#include <iostream>

/////////// - StormKit::core - ///////////
#include <storm/core/Strings.hpp>

/////////// - StormKit::render - ///////////
#include <storm/render/core/Instance.hpp>
#include <storm/render/core/Device.hpp>
#include <storm/render/core/Queue.hpp>
#include <storm/render/core/CommandBuffer.hpp>
#include <storm/render/core/PhysicalDevice.hpp>
#include <storm/render/core/OffscreenSurface.hpp>

#include <storm/render/resource/Texture.hpp>
#include <storm/render/resource/Shader.hpp>
#include <storm/render/resource/Texture.hpp>
#include <storm/render/resource/TextureView.hpp>

#include <storm/render/pipeline/GraphicsPipeline.hpp>
#include <storm/render/pipeline/Framebuffer.hpp>
#include <storm/render/pipeline/RenderPass.hpp>

#include <shaderc/shaderc.hpp>

INQUISITOR_PLUGIN(ShaderPlugin)

using namespace std::literals;
using namespace storm::render;

static constexpr auto VERTEX_SHADER = R"(
#version 460 core

#pragma shader_stage(vertex)

layout(location = 0) out vec2 frag_uv;

vec2 positions[6] = vec2[](
    vec2(-1.f, -1.f),
    vec2(1.f, -1.f),
    vec2(-1.f, 1.f),
    vec2(-1.f, 1.f),
    vec2(1.f, -1.f),
    vec2(1.f, 1.f)
);

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 p = vec4(positions[gl_VertexIndex], 0.f, 1.f);

    gl_Position = p;

    frag_uv = vec2(max(0, positions[gl_VertexIndex].x), max(0, positions[gl_VertexIndex].y));
}
)"sv;

static constexpr auto FRAG_TEMPLATE = R"(
#version 460 core

#pragma shader_stage(fragment)

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

)"sv;

//static constexpr auto REGEX = R"(```glsl\n([[:alnum:]]+)\n```)";
static constexpr auto REGEX = R"((```\glsl\n)((.|\n)+)(\n```))";

struct Vertex {
    storm::core::Vector2f position;
};

static constexpr auto MESH_VERTEX_BINDING_DESCRIPTIONS =
    std::array { VertexBindingDescription { .binding = 0, .stride = sizeof(Vertex) } };
static constexpr auto MESH_VERTEX_ATTRIBUTE_DESCRIPTIONS =
    std::array { VertexInputAttributeDescription { .location = 0,
                                                           .binding  = 0,
                                                           .format   = Format::Float2,
                                                           .offset   = offsetof(Vertex, position) }};

/////////////////////////////////////
/////////////////////////////////////
ShaderPlugin::ShaderPlugin()
    : m_regex{REGEX, std::regex::ECMAScript | std::regex::optimize | std::regex::icase} {
    ilog("Initialization of render backend");
    m_instance = std::make_unique<Instance>();
    ilog("Success");

    m_surface = m_instance->createOffscreenSurfacePtr({800, 600}, Surface::Buffering::Simple);

    const auto &physical_device      = m_instance->pickPhysicalDevice();
    const auto &physical_device_info = physical_device.info();

    ilog("Using physical device {}", physical_device_info.device_name);
    ilog("{}", physical_device_info);

    ilog("Initialization of render device");
    m_device = physical_device.createLogicalDevicePtr();
    ilog("Success");

    ilog("Initialization of surface");
    m_surface->initialize(*m_device);
    ilog("Success, with {} image(s)", m_surface->textureCount());

    m_queue = &m_device->graphicsQueue();

    m_surface_view = m_surface->textures()[0].createViewPtr();

    ilog("Compiling vertex shader");
    auto compiler = shaderc::Compiler{};
    auto compile_options = shaderc::CompileOptions{};
    compile_options.SetOptimizationLevel(shaderc_optimization_level_size);

    auto result = compiler.CompileGlslToSpv(std::data(VERTEX_SHADER), std::size(VERTEX_SHADER), shaderc_glsl_vertex_shader, "");

    if(result.GetCompilationStatus() == shaderc_compilation_status_compilation_error) {
        elog("Failed to compile vertex shader, reason: {}", result.GetErrorMessage());
        return;
    }

    m_vertex_shader = m_device->createShaderPtr(result, ShaderStage::Vertex);
    ilog("Success");

    ilog("Checking blit capabilities");
    m_has_blit = [this]() {
            auto &physical_device = m_device->physicalDevice();
            auto swapchain_properties = physical_device.vkGetFormatProperties(toVK(m_surface->pixelFormat()));
            auto destination_properties = physical_device.vkGetFormatProperties(toVK(PixelFormat::RGBA8_UNorm));

            return (swapchain_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) &&
                   (destination_properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst);
        }();
    m_has_blit = false;

    if(m_has_blit) ilog("Success, blit enabled");
    else ilog("Success, bot not supported, falling back on image copy");
}

/////////////////////////////////////
/////////////////////////////////////
ShaderPlugin::~ShaderPlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::name() const -> std::string_view {
    return "ShaderPlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::commands() const -> std::vector<std::string_view> {
    return { "shader" };
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::help() const -> std::string_view {
    return "🔵 **shader** -> Render a shader";
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::onCommand(std::string_view command, const json &msg) -> void {
    auto content = msg["content"].get<std::string>();
    auto channel = msg["channel_id"].get<std::string>();

    auto matches = std::smatch{};

    if(!std::regex_search(content, matches, m_regex)) {
        auto response = json {
            {"content", "Invalid usage, correct usage is\n;inquisitor shader\n\\`\\`\\`glsl\n   //CODE\n\\`\\`\\`"}
        };

        sendMessage(channel, std::move(response));
        return;
    }

    auto glsl = matches[2].str();

    auto spirv = std::vector<SpirvID>{};

    auto error = compileShader(glsl, spirv);
    if(error) {
        auto response = json {
            {"content", error.value() }
        };

        sendMessage(channel, std::move(response));
        return;
    }

    {
        auto response = json {
            {"content", fmt::format(":white_check_mark: Compilation success! :white_check_mark:")}
        };

        sendMessage(channel, std::move(response));
    }

    if(m_is_currently_rendering) {
        auto response = json {
            {"content", "Waiting for an other rendering request"}
        };

        sendMessage(channel, std::move(response));

        while(m_is_currently_rendering) {}
    }
    m_is_currently_rendering = true;

    auto result = render(spirv);

    m_is_currently_rendering = false;

    auto response = json{};
    sendFile(channel, "result.jpg", "image/jpeg", std::move(result), response);
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::initialize(const json &options) -> void {
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::compileShader(std::string_view glsl, std::vector<SpirvID> &output) -> std::optional<std::string> {
    auto compiler = shaderc::Compiler{};
    auto compile_options = shaderc::CompileOptions{};

    auto defines = storm::core::HashMap<std::string, std::string>{};

    for(const auto &[name, value] : defines)
        compile_options.AddMacroDefinition(name, value);

    compile_options.SetOptimizationLevel(shaderc_optimization_level_size);

    auto source = std::string{FRAG_TEMPLATE} + std::string{glsl};

    auto result = compiler.CompileGlslToSpv(source, shaderc_glsl_fragment_shader, "", compile_options);

    if(result.GetCompilationStatus() == shaderc_compilation_status_compilation_error) {
        auto error = fmt::format(":warning: Failed to compile shader :warning:\n\n**reason:**\n```\n{}\n```\n\n**code:**\n```glsl\n{}\n```", result.GetErrorMessage(), source);
        return error;
    }

    std::ranges::copy(result, std::back_inserter(output));

    return std::nullopt;
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::render(std::span<const SpirvID> spirv) -> std::string {

    const auto surface_extentu  = m_surface->extent();
    const auto surface_extent  = storm::core::Extentf { m_surface->extent() };

    auto fragment_shader = m_device->createShader(spirv, ShaderStage::Fragment);

    auto description = RenderPassDescription {
                                               .attachments = { { .format = m_surface->pixelFormat(), .source_layout = TextureLayout::Color_Attachment_Optimal, .destination_layout = TextureLayout::Color_Attachment_Optimal } },
        .subpasses   = { { .bind_point      = PipelineBindPoint::Graphics,
                         .attachment_refs = { { .attachment_id = 0u } } } }
    };
    auto render_pass = m_device->createRenderPass(std::move(description));

    auto pipeline       = m_device->createGraphicsPipeline();
    const auto state = GraphicsPipelineState {
        .viewport_state     = {
             .viewports = {
                  Viewport { .position = { 0.f, 0.f },
                             .extent   = surface_extent,
                             .depth    = { 0.f, 1.f } } },
                             .scissors  = { Scissor { .offset = { 0, 0 },
                                                      .extent = surface_extent } } },
        .rasterization_state = {
            .cull_mode = CullMode::None
        },
             .color_blend_state  = { .attachments = { {} } },
             .shader_state       = { .shaders = storm::core::makeConstObserverArray(m_vertex_shader, fragment_shader) } };

    pipeline.setState(std::move(state));
    pipeline.setRenderPass(render_pass);
    pipeline.build();

    auto frame_buffer = render_pass.createFramebuffer(surface_extent, storm::core::makeConstObserverArray(m_surface_view));

    auto render_command_buffer = m_queue->createCommandBuffer();
    auto fence = m_device->createFence();

    render_command_buffer.begin(true);
    render_command_buffer.beginRenderPass(render_pass, frame_buffer);
    render_command_buffer.bindGraphicsPipeline(pipeline);
    render_command_buffer.draw(6);
    render_command_buffer.endRenderPass();
    render_command_buffer.end();

    render_command_buffer.build();

    render_command_buffer.submit({}, {}, &fence);

    fence.wait();

    const auto &source = m_surface->textures()[0];
    auto destination =
        m_device->createTexture(surface_extent,
                                PixelFormat::RGBA8_UNorm,
                                1u,
                                1u,
                                TextureType::T2D,
                                TextureCreateFlag::None,
                                SampleCountFlag::C1_BIT,
                                TextureUsage::Transfert_Dst,
                                TextureTiling::Linear,
                                MemoryProperty::Host_Visible | MemoryProperty::Host_Coherent);

    auto copy_command_buffer = m_queue->createCommandBuffer();
    fence.reset();

    copy_command_buffer.begin(true);
    copy_command_buffer.transitionTextureLayout(source, TextureLayout::Color_Attachment_Optimal, TextureLayout::Transfer_Src_Optimal);
    copy_command_buffer.transitionTextureLayout(destination, TextureLayout::Undefined, TextureLayout::Transfer_Dst_Optimal);
    
    if(m_has_blit) {
        auto region = BlitRegion {
            .source_offset = { storm::core::ExtentuOffset{0u, 0u, 0u}, storm::core::ExtentuOffset{surface_extentu.width, surface_extentu.height, 1u} },
            .destination_offset = { storm::core::ExtentuOffset{0u, 0u, 0u}, storm::core::ExtentuOffset{surface_extentu.width, surface_extentu.height, 1u} }
        };
        
        copy_command_buffer.blitTexture(source,
                                        destination,
                                        TextureLayout::Transfer_Src_Optimal,
                                        TextureLayout::Transfer_Dst_Optimal,
                                        { region },
                                        Filter::Nearest);
    } else {
        copy_command_buffer.copyTexture(source,
                                        destination,
                                        TextureLayout::Transfer_Src_Optimal,
                                        TextureLayout::Transfer_Dst_Optimal,
                                        {},
                                        {},
                                        surface_extentu);
    }
    
    copy_command_buffer.transitionTextureLayout(source, TextureLayout::Transfer_Src_Optimal, TextureLayout::Color_Attachment_Optimal);
    copy_command_buffer.transitionTextureLayout(destination, TextureLayout::Transfer_Dst_Optimal, TextureLayout::General);
    copy_command_buffer.end();

    copy_command_buffer.build();

    copy_command_buffer.submit({}, {}, &fence);

    fence.wait();
    
    auto data = m_device->mapVmaMemory(destination.vkAllocation());

    auto subresource = vk::ImageSubresource{ vk::ImageAspectFlagBits::eColor, 0, 0 };
    auto subresource_layout = m_device->vkDevice().getImageSubresourceLayout(destination, subresource, m_device->vkDispatcher());

    auto data_span = storm::core::ByteConstSpan(data + subresource_layout.offset, surface_extentu.width * surface_extentu.height * 4u);

    auto image = storm::image::Image{};
    image.create(surface_extentu, storm::image::Image::Format::RGBA8_UNorm);

    for(auto i = 0u; i < surface_extentu.height; ++i) {
        auto data = data_span.subspan(i * subresource_layout.rowPitch, surface_extentu.width * 4u);

        std::ranges::copy(data, std::begin(image.data()) + i * surface_extentu.width * 4u);
    }

    m_device->unmapVmaMemory(destination.vkAllocation());

    auto output = image.saveToMemory(storm::image::Image::Codec::JPEG);
    image.saveToFile("./test.jpg", storm::image::Image::Codec::JPEG);

    auto output_str = std::string{};
    output_str.resize(std::size(output), '\0');

    std::ranges::transform(output, std::begin(output_str), [](auto byte) { return static_cast<char>(byte);});

    return output_str;
}

std::string ShaderPlugin::toBase64(storm::core::ByteConstSpan data) {
    static const auto base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/"sv;

    auto result = std::string{};
    result.reserve(((std::size(data) / 4) + (std::size(data) % 3u > 0)) * 4);

    auto temp = std::uint32_t{};
    auto it = reinterpret_cast<const storm::core::UInt8 *>(std::data(data));

    for(auto i = 0u; i < std::size(data) / 3; ++i) {
        temp = (*it++)  << 16;
        temp += (*it++) << 8;
        temp += (*it++);

        result += base64_chars[(temp & 0x00FC0000) >> 18];
        result += base64_chars[(temp & 0x0003F000) >> 12];
        result += base64_chars[(temp & 0x00000FC0) >> 6];
        result += base64_chars[temp & 0x0000003F];
    }

    if(std::size(data) % 3 == 1) {
        temp = (*it++) << 16;
        result += base64_chars[(temp & 0x00FC0000) >> 18];
        result += base64_chars[(temp & 0x0003F000) >> 12];
        result += '=';
    } else if(std::size(data) % 3 == 2) {
        temp = (*it++) << 16;
        temp += (*it++) << 8;

        result += base64_chars[(temp & 0x00FC0000) >> 18];
        result += base64_chars[(temp & 0x0003F000) >> 12];
        result += base64_chars[(temp & 0x00000FC0) >> 6];
    }

    return result;
}
