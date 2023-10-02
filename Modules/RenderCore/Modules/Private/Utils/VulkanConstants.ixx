// Author: Lucas Vilas-Boas
// Year : 2023
// Repo : https://github.com/lucoiso/VulkanRender

module;

#include <array>
#include <volk.h>

export module RenderCore.Utils.VulkanConstants;

namespace RenderCore
{
#ifdef _DEBUG
#if defined(GPU_API_DUMP) && GPU_API_DUMP
    constexpr std::array g_DebugInstanceLayers = {
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_LUNARG_api_dump"};

    constexpr std::array g_DebugDeviceLayers = {
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_LUNARG_api_dump"};
#else
    export constexpr std::array g_DebugInstanceLayers = {
            "VK_LAYER_KHRONOS_validation"};

    export constexpr std::array g_DebugDeviceLayers = {
            "VK_LAYER_KHRONOS_validation"};
#endif

    export constexpr std::array g_DebugInstanceExtensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            "VK_EXT_debug_report",
            "VK_EXT_validation_features"};

    export constexpr std::array g_DebugDeviceExtensions = {
            "VK_EXT_validation_cache",
            "VK_EXT_tooling_info"};

    export constexpr std::array g_EnabledInstanceValidationFeatures = {
            // VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            // VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
            VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
            VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
#endif

    export constexpr std::array<char const*, 0U> g_RequiredInstanceLayers = {};

    export constexpr std::array<char const*, 0U> g_RequiredDeviceLayers = {};

    export constexpr std::array<char const*, 0U> g_RequiredInstanceExtensions = {};

    export constexpr std::array g_RequiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    export constexpr std::array g_DynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};

    export constexpr std::uint8_t g_MaxFramesInFlight = 1U;

    export constexpr std::uint64_t g_BufferMemoryAllocationSize      = 65536U;
    export constexpr std::uint64_t g_ImageBufferMemoryAllocationSize = 262144U;

    export constexpr VkSampleCountFlagBits g_MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    export constexpr std::uint8_t g_FrameRate = 75U;

    export constexpr std::array g_ClearValues {
            VkClearValue {
                    .color = {
                            {0.25F,
                             0.25F,
                             0.5F,
                             1.F}}},
            VkClearValue {.depthStencil = {1.0F, 0U}}};
}// namespace RenderCore