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

/////////// - ShaderC - ///////////
#include <shaderc/shaderc.hpp>

/////////// - WebP - ///////////
#include <webp/encode.h>

INQUISITOR_PLUGIN(ShaderPlugin)

using namespace std::literals;
using namespace storm;
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
})"sv;

static constexpr auto FRAG_TEMPLATE = R"(
#version 460 core

#pragma shader_stage(fragment)

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform PushConstants {
    float time;
    uint  frame;
    uvec2 resolution;
} constants;

TEXTURES

SHADER_SOURCE)"sv;

//static constexpr auto REGEX = R"(```glsl\n([[:alnum:]]+)\n```)";
static constexpr auto GLSL_REGEX = R"(```glsl\n([a-zA-Z0-9\n\r\t^,:{}\[\]\-\.+=*$&\-\_%?\/" \(\);<>!]+)\n```)";
static constexpr auto JSON_REGEX = R"(```json\n([a-zA-Z0-9\n\r\t^,:{}\[\]\-\.+=*$&\-\_%?\/" \(\);<>!]+)\n```)";

struct Vertex {
    core::Vector2f position;
};

static constexpr auto MESH_VERTEX_BINDING_DESCRIPTIONS =
    std::array { VertexBindingDescription { .binding = 0, .stride = sizeof(Vertex) } };
static constexpr auto MESH_VERTEX_ATTRIBUTE_DESCRIPTIONS =
    std::array { VertexInputAttributeDescription { .location = 0,
                                                           .binding  = 0,
                                                           .format   = Format::Float2,
                                                           .offset   = offsetof(Vertex, position) }};

struct alignas(16) PushConstants {
    float time;
    core::UInt32 frame;
    core::Vector2u resolution;
};

/////////////////////////////////////
/////////////////////////////////////
ShaderPlugin::ShaderPlugin()
    : m_glsl_regex{GLSL_REGEX, std::regex::ECMAScript | std::regex::optimize | std::regex::icase},
      m_json_regex{JSON_REGEX, std::regex::ECMAScript | std::regex::optimize | std::regex::icase} {
    ilog("Initialization of render backend");
    m_instance = std::make_unique<Instance>();
    ilog("Success");

    const auto &physical_device      = m_instance->pickPhysicalDevice();
    const auto &physical_device_info = physical_device.info();

    ilog("Using physical device {}", physical_device_info.device_name);
    ilog("{}", physical_device_info);

    ilog("Initialization of render device");
    m_device = physical_device.createLogicalDevicePtr();
    ilog("Success");

    m_queue = &m_device->graphicsQueue();

    ilog("Compiling vertex shader");
    auto compiler = shaderc::Compiler{};
    auto compile_options = shaderc::CompileOptions{};
    compile_options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto result = compiler.CompileGlslToSpv(std::data(VERTEX_SHADER), std::size(VERTEX_SHADER), shaderc_glsl_vertex_shader, "vertex.glsl", compile_options);

    if(result.GetCompilationStatus() == shaderc_compilation_status_compilation_error) {
        elog("Failed to compile vertex shader, reason: {}", result.GetErrorMessage());
        return;
    }

    m_vertex_shader = m_device->createShaderPtr(result, ShaderStage::Vertex);
    ilog("Success");

    ilog("Checking blit capabilities");
    m_has_blit = [&physical_device]() {
            auto swapchain_properties = physical_device.vkGetFormatProperties(toVK(PixelFormat::RGBA8_UNorm));
            auto destination_properties = physical_device.vkGetFormatProperties(toVK(PixelFormat::RGBA8_UNorm));

            return (swapchain_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) &&
                   (destination_properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst);
        }();

    if(m_has_blit) ilog("Success, blit enabled");
    else ilog("Success, bot not supported, falling back on image copy");

    ilog("Detecting max extent");
    m_max_extent = [&physical_device]() {
        auto &capabilities = physical_device.capabilities();

        return core::Extentu{ capabilities.limits.max_viewport_dimensions[0], capabilities.limits.max_viewport_dimensions[1] };
    }();
    ilog("Success: {}", m_max_extent);
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
    const auto channel_id = msg["channel_id"].get<std::string>();
    const auto options_opt = getAttachedJson(msg);

    auto options = options_opt.value_or(json{});
    auto textures = std::vector<std::string>{};
    auto frame_count = 1u;
    auto extent = core::Extentu{800, 600};

    if(options.contains("textures") && options["textures"].is_array())
        textures = options["textures"].get<std::vector<std::string>>();

    if(options.contains("frame_count") && options["frame_count"].is_number_unsigned())
        frame_count = std::max(1u, options["frame_count"].get<core::UInt32>());

    if(options.contains("extent") && options["extent"].is_object()) {
       if(options["extent"].contains("width") && options["extent"]["width"].is_number_unsigned())
           extent.width = std::min(options["extent"]["width"].get<core::UInt32>(), m_max_extent.width);

       if(options["extent"].contains("height") && options["extent"]["height"].is_number_unsigned())
           extent.height = std::min(options["extent"]["height"].get<core::UInt32>(), m_max_extent.height);
    }

    auto glsl_opt = getAttachedGlsl(msg);
    if(!glsl_opt) {
        auto response = json {
            {"content", "Invalid usage, correct usage is\n;inquisitor shader\n\\`\\`\\`glsl\n   //CODE\n\\`\\`\\`\n or \n;inquisitor shader (with file embed)"}
        };

        sendMessage(channel_id, std::move(response));
        return;
    }

    const auto glsl = std::move(glsl_opt.value());

    if(frame_count == 1u)
        singleFrame(textures, channel_id, glsl, extent);
    else
        multipleFrame(textures, frame_count, channel_id, glsl, extent);
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::initialize(const json &options) -> void {
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::getAttachedGlsl(const json &msg) const -> std::optional<std::string> {
    const auto content = msg["content"].get<std::string>();
    auto matches = std::smatch{};

    if(std::regex_search(content, matches, m_glsl_regex))
        return matches[1].str();

    if(!msg.contains("attachments")) return std::nullopt;

    auto index = 0;

    for(const auto &attachment : msg["attachments"]) {
        if(attachment.contains("filename")) {
            auto ext = core::split(attachment["filename"].get<std::string>(), '.')[1];

            if(ext == "glsl")  {
                const auto url = attachment["proxy_url"].get<std::string>();

                return getHttpFile(fmt::format("https://cdn.discordapp.com/{}", std::string_view{url}.substr(29)));
            }
        }

        ++index;
    }

    return std::nullopt;
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::getAttachedJson(const json &msg) const -> std::optional<json> {
    const auto content = msg["content"].get<std::string>();
    auto matches = std::smatch{};

    if(!std::regex_search(content, matches, m_json_regex)) return std::nullopt;

    if(!json::accept(matches[1].str()))
        return json{};

    return json::parse(matches[1].str());
}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::compileShader(std::string_view glsl, std::vector<SpirvID> &output, std::size_t texture_count) -> std::optional<std::pair<std::string, std::string>> {
    auto compiler = shaderc::Compiler{};
    auto preprocess_options = shaderc::CompileOptions{};
    auto compile_options = shaderc::CompileOptions{};

    auto defines = core::HashMap<std::string, std::string>{};
    defines.emplace("TEXTURES", "");

    auto frag_template = std::string{FRAG_TEMPLATE};
    auto pos = frag_template.find("SHADER_SOURCE");
    frag_template.replace(pos, std::size("SHADER_SOURCE"), glsl);

    if(texture_count >= 1u)
        defines["TEXTURES"] = fmt::format("layout(set = 0, binding = 0) uniform sampler2D textures[{}];", texture_count);

    preprocess_options.SetOptimizationLevel(shaderc_optimization_level_size);
    for(const auto &[name, value] : defines)
        preprocess_options.AddMacroDefinition(name, value);

    auto preprocessed = compiler.PreprocessGlsl(frag_template, shaderc_glsl_fragment_shader, "fragment.glsl", preprocess_options);

    if(preprocessed.GetCompilationStatus() != shaderc_compilation_status_success) {
        return std::pair{ std::string{preprocessed.GetErrorMessage()}, std::string{FRAG_TEMPLATE} };
    }

    auto source = std::string{};
    std::ranges::copy(preprocessed, std::back_inserter(source));

    compile_options.SetOptimizationLevel(shaderc_optimization_level_performance);
    compile_options.SetGenerateDebugInfo();

    auto result = compiler.CompileGlslToSpv(source, shaderc_glsl_fragment_shader, "", compile_options);
    if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
        return std::pair{ std::string{result.GetErrorMessage()}, std::move(source)};
    }

    std::ranges::copy(result, std::back_inserter(output));

    return std::nullopt;
}

auto ShaderPlugin::singleFrame(std::vector<std::string> textures, std::string_view channel_id, std::string_view glsl, core::Extentu extent) -> void {
    auto spirv = std::vector<SpirvID>{};

    auto content = std::string{};

    auto error = compileShader(glsl, spirv, std::size(textures));
    if(error) {
        content = fmt::format(":warning: Compilation failed! :warning:\n\n**reason:**\n```\n{}\n```\n", error.value().first);

        auto response = json {
            {"content", std::move(content)}
        };

        sendFile(channel_id, "shader.glsl", "text/glsl", error.value().second, std::move(response));
        return;
    }

    auto textures_str = std::string{};
    auto i = 0u;
    for(const auto &texture : textures)
        textures_str += fmt::format("    textures[{}] = {},\n", i++, texture);

    auto opt_string = fmt::format("Options: \n```textures:\n{}\nframe_count: {}\nextent:\n    width: {},\n    height: {}```", textures_str, 1u, extent.width, extent.height);
    content = fmt::format(":white_check_mark: Compilation success! :white_check_mark:\n{}", opt_string);

    if(m_is_currently_rendering) {
        auto response = json {
            {"content", "Waiting for an other rendering request"}
        };

        sendMessage(channel_id, std::move(response));

        while(m_is_currently_rendering) {}
    }
    m_is_currently_rendering = true;

    auto result_var = render(spirv, textures, extent);

    m_is_currently_rendering = false;

    auto result = std::string{};
    if(std::holds_alternative<ErrorString>(result_var)) {
        content += fmt::format("\n:warning: Rendering failed! :warning:\n **reason:** {}", std::get<ErrorString>(result_var).get());
    } else {
        content += "\n:white_check_mark: Rendering success! :white_check_mark:";

        result = std::move(std::get<std::string>(result_var));
    }

    auto response = json {
        {"content", std::move(content)}
    };

    sendFile(channel_id, "result.png", "image/png", std::move(result), response);
}

auto ShaderPlugin::multipleFrame(std::vector<std::string> textures, storm::core::UInt32 frame_count, std::string_view channel_id, std::string_view glsl, core::Extentu extent) -> void {

}

/////////////////////////////////////
/////////////////////////////////////
auto ShaderPlugin::render(std::span<const SpirvID> spirv, std::span<const std::string> textures, core::Extentu extent) -> std::variant<std::string, ErrorString> {
    auto render_image =
        m_device->createTexture(extent,
                                PixelFormat::RGBA8_UNorm,
                                1u,
                                1u,
                                TextureType::T2D,
                                TextureCreateFlag::None,
                                SampleCountFlag::C1_BIT,
                                TextureUsage::Color_Attachment | TextureUsage::Transfert_Src);
    auto render_image_view = render_image.createView();

    auto fragment_shader = m_device->createShader(spirv, ShaderStage::Fragment);

    auto description = RenderPassDescription {
                                               .attachments = { { .format = PixelFormat::RGBA8_UNorm, .source_layout = TextureLayout::Color_Attachment_Optimal, .destination_layout = TextureLayout::Color_Attachment_Optimal } },
        .subpasses   = { { .bind_point      = PipelineBindPoint::Graphics,
                         .attachment_refs = { { .attachment_id = 0u } } } }
    };
    auto render_pass = m_device->createRenderPass(std::move(description));

    auto pipeline       = m_device->createGraphicsPipeline();
    auto state = GraphicsPipelineState {
        .viewport_state     = {
             .viewports = {
                  Viewport { .position = { 0.f, 0.f },
                             .extent   = extent,
                             .depth    = { 0.f, 1.f } } },
                             .scissors  = { Scissor { .offset = { 0, 0 },
                                                      .extent = extent } } },
        .rasterization_state = {
            .cull_mode = CullMode::None
        },
             .color_blend_state  = { .attachments = { {} } },
             .shader_state       = { .shaders = core::makeConstObserverArray(m_vertex_shader, fragment_shader) } };

    auto gpu_textures = std::vector<render::TextureOwnedPtr>{};
    auto gpu_texture_views = std::vector<render::TextureViewOwnedPtr>{};

    auto sampler = render::SamplerOwnedPtr{nullptr};

    auto descriptor_set_layout = DescriptorSetLayoutOwnedPtr{nullptr};
    auto descriptor_pool = DescriptorPoolOwnedPtr{nullptr};

    auto descriptor_set = DescriptorSetOwnedPtr{nullptr};

    if(!std::empty(textures)) {
        sampler = m_device->createSamplerPtr();

        descriptor_set_layout = m_device->createDescriptorSetLayoutPtr();
        descriptor_set_layout->addBinding({
            .binding = 0,
            .type    = render::DescriptorType::Combined_Texture_Sampler,
            .stages  = render::ShaderStage::Fragment,
            .descriptor_count = std::size(textures)
        });
        descriptor_set_layout->bake();

        descriptor_pool =
            m_device->createDescriptorPoolPtr({ { DescriptorType::Combined_Texture_Sampler, std::size(textures) } }, 1);

        state.layout.descriptor_set_layouts = core::makeConstObserverArray(descriptor_set_layout);

        descriptor_set = descriptor_pool->allocateDescriptorSetPtr(*descriptor_set_layout);

        auto descriptors = render::DescriptorArray{};
            descriptors.reserve(std::size(textures));

        auto i = 0u;
        for(const auto &texture : textures) {
            auto file = getHttpFile(texture);

            auto data = core::toConstByteSpan(file);

            if(std::empty(data))
                return ErrorString{fmt::format("Failed to get image file {}", texture)};

            auto image = image::Image{};
            auto loaded = image.loadFromMemory(data);

            if(!loaded)
                return ErrorString{fmt::format("Failed to load image file {}, codec not supported or maybe not an image", texture)};

            auto &gpu_texture = gpu_textures.emplace_back(m_device->createTexturePtr(image.extent()));
            gpu_texture->loadFromImage(image.toFormat(image::Image::Format::RGBA8_UNorm));

            auto &view = gpu_texture_views.emplace_back(gpu_texture->createViewPtr());

            descriptors.emplace_back(render::TextureDescriptor{
                .binding      = 0,
                .layout       = render::TextureLayout::Shader_Read_Only_Optimal,
                .texture_view = view.get(),
                .sampler      = sampler.get()
            });

            ++i;
        }

        descriptor_set->update(descriptors);
    }

    auto push_constants_range = PushConstantRange {
        .stages = ShaderStage::Fragment,
        .offset = 0,
        .size   = sizeof(PushConstants)
    };

    state.layout.push_constant_ranges.emplace_back(push_constants_range);

    pipeline.setState(std::move(state));
    pipeline.setRenderPass(render_pass);
    pipeline.build();

    auto push_constants = PushConstants {
        .time = 0.f,
        .frame = 1,
        .resolution = core::Vector2u{extent.width, extent.height}
    };

    auto frame_buffer = render_pass.createFramebuffer(extent, core::makeConstObserverArray(render_image_view));

    auto render_command_buffer = m_queue->createCommandBuffer();
    auto fence = m_device->createFence();

    render_command_buffer.begin(true);
    render_command_buffer.transitionTextureLayout(render_image, TextureLayout::Undefined, TextureLayout::Color_Attachment_Optimal);
    render_command_buffer.beginRenderPass(render_pass, frame_buffer);
    render_command_buffer.bindGraphicsPipeline(pipeline);

    if(descriptor_set)
        render_command_buffer.bindDescriptorSets(pipeline, {*descriptor_set});

    auto push_data_span = core::toConstByteSpan(&push_constants);
    auto push_data = core::ByteArray{};
    push_data.reserve(std::size(push_data_span));

    std::ranges::copy(push_data_span, std::back_inserter(push_data));

    render_command_buffer.pushConstants(pipeline, ShaderStage::Fragment, std::move(push_data), 0u);
    render_command_buffer.draw(6);
    render_command_buffer.endRenderPass();
    render_command_buffer.end();

    render_command_buffer.build();

    render_command_buffer.submit({}, {}, &fence);

    fence.wait();

    auto destination =
        m_device->createTexture(extent,
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
    copy_command_buffer.transitionTextureLayout(render_image, TextureLayout::Color_Attachment_Optimal, TextureLayout::Transfer_Src_Optimal);
    copy_command_buffer.transitionTextureLayout(destination, TextureLayout::Undefined, TextureLayout::Transfer_Dst_Optimal);
    
    if(m_has_blit) {
        auto region = BlitRegion {
            .source_offset = { core::ExtentuOffset{0u, 0u, 0u}, core::ExtentuOffset{extent.width, extent.height, 1u} },
            .destination_offset = { core::ExtentuOffset{0u, 0u, 0u}, core::ExtentuOffset{extent.width, extent.height, 1u} }
        };
        
        copy_command_buffer.blitTexture(render_image,
                                        destination,
                                        TextureLayout::Transfer_Src_Optimal,
                                        TextureLayout::Transfer_Dst_Optimal,
                                        { region },
                                        Filter::Nearest);
    } else {
        copy_command_buffer.copyTexture(render_image,
                                        destination,
                                        TextureLayout::Transfer_Src_Optimal,
                                        TextureLayout::Transfer_Dst_Optimal,
                                        {},
                                        {},
                                        extent);
    }
    
    copy_command_buffer.end();

    copy_command_buffer.build();

    copy_command_buffer.submit({}, {}, &fence);

    fence.wait();
    
    auto data = m_device->mapVmaMemory(destination.vkAllocation());

    auto subresource = vk::ImageSubresource{ vk::ImageAspectFlagBits::eColor, 0, 0 };
    auto subresource_layout = m_device->vkDevice().getImageSubresourceLayout(destination, subresource, m_device->vkDispatcher());

    auto data_span = core::ByteConstSpan(data + subresource_layout.offset, extent.width * extent.height * 4u);

    auto image = image::Image{};
    image.create(extent, image::Image::Format::RGBA8_UNorm);

    for(auto i = 0u; i < extent.height; ++i) {
        auto data = data_span.subspan(i * subresource_layout.rowPitch, extent.width * 4u);

        std::ranges::copy(data, std::begin(image.data()) + i * extent.width * 4u);
    }

    m_device->unmapVmaMemory(destination.vkAllocation());

    image.saveToFile("./test.png", image::Image::Codec::PNG);
    auto output = image.saveToMemory(image::Image::Codec::PNG);

    auto output_str = std::string{};
    output_str.resize(std::size(output), '\0');

    std::ranges::transform(output, std::begin(output_str), [](auto byte) { return static_cast<char>(byte);});

    return output_str;
}
