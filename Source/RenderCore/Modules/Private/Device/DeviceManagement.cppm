// Author: Lucas Vilas-Boas
// Year : 2023
// Repo : https://github.com/lucoiso/VulkanRenderer

module;

#include <GLFW/glfw3.h>
#include <boost/log/trivial.hpp>
#include <volk.h>

module RenderCore.Management.DeviceManagement;

import <optional>;

import RenderCore.Management.BufferManagement;
import RenderCore.Utils.Helpers;
import RenderCore.Utils.Constants;
import RenderCore.Utils.EnumConverter;
import Timer.ExecutionCounter;

using namespace RenderCore;

VkPhysicalDevice g_PhysicalDevice {VK_NULL_HANDLE};
VkDevice g_Device {VK_NULL_HANDLE};
DeviceProperties g_DeviceProperties {};
std::pair<std::uint8_t, VkQueue> g_GraphicsQueue {};
std::pair<std::uint8_t, VkQueue> g_PresentationQueue {};
std::pair<std::uint8_t, VkQueue> g_TransferQueue {};
std::vector<std::uint8_t> g_UniqueQueueFamilyIndices {};

bool GetQueueFamilyIndices(VkSurfaceKHR const& VulkanSurface,
                           std::optional<std::uint8_t>& GraphicsQueueFamilyIndex,
                           std::optional<std::uint8_t>& PresentationQueueFamilyIndex,
                           std::optional<std::uint8_t>& TransferQueueFamilyIndex)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Getting queue family indices";

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    std::uint32_t QueueFamilyCount = 0U;
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &QueueFamilyCount, std::data(QueueFamilies));

    for (std::uint32_t Iterator = 0U; Iterator < QueueFamilyCount; ++Iterator)
    {
        if (!GraphicsQueueFamilyIndex.has_value() && (QueueFamilies.at(Iterator).queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U)
        {
            GraphicsQueueFamilyIndex = static_cast<std::uint8_t>(Iterator);
        }
        else if (!TransferQueueFamilyIndex.has_value() && (QueueFamilies.at(Iterator).queueFlags & VK_QUEUE_TRANSFER_BIT) != 0U)
        {
            TransferQueueFamilyIndex = static_cast<std::uint8_t>(Iterator);
        }
        else if (!PresentationQueueFamilyIndex.has_value())
        {
            VkBool32 PresentationSupport = 0U;
            CheckVulkanResult(vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, Iterator, VulkanSurface, &PresentationSupport));

            if (PresentationSupport != 0U)
            {
                PresentationQueueFamilyIndex = static_cast<std::uint8_t>(Iterator);
            }
        }

        if (GraphicsQueueFamilyIndex.has_value() && PresentationQueueFamilyIndex.has_value() && TransferQueueFamilyIndex.has_value())
        {
            break;
        }
    }

    return GraphicsQueueFamilyIndex.has_value() && PresentationQueueFamilyIndex.has_value() && TransferQueueFamilyIndex.has_value();
}

#ifdef _DEBUG

void ListAvailablePhysicalDevices()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Listing available vulkan physical devices...";

    for (VkPhysicalDevice const& DeviceIter: GetAvailablePhysicalDevices())
    {
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(DeviceIter, &DeviceProperties);

        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Name: " << DeviceProperties.deviceName;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target ID: " << DeviceProperties.deviceID;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Vendor ID: " << DeviceProperties.vendorID;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Driver Version: " << DeviceProperties.driverVersion;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target API Version: " << DeviceProperties.apiVersion << std::endl;
    }
}

void ListAvailablePhysicalDeviceLayers()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Listing available vulkan physical device layers...";

    for (auto const& [LayerName, SpecVer, ImplVer, Descr]: GetAvailablePhysicalDeviceLayers())
    {
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Name: " << LayerName;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Spec Version: " << SpecVer;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Implementation Version: " << ImplVer;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Description: " << Descr << std::endl;
    }
}

void ListAvailablePhysicalDeviceLayerExtensions(std::string_view const& LayerName)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Listing available vulkan physical device layer '" << LayerName << "' extensions...";

    for (auto const& [ExtName, SpecVer]: GetAvailablePhysicalDeviceLayerExtensions(LayerName))
    {
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Name: " << ExtName;
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Target Spec Version: " << SpecVer << std::endl;
    }
}

void ListAvailablePhysicalDeviceSurfaceCapabilities(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Listing available vulkan physical device surface capabilities...";

    VkSurfaceCapabilitiesKHR const SurfaceCapabilities = GetAvailablePhysicalDeviceSurfaceCapabilities(VulkanSurface);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Min ImageAllocation Count: " << SurfaceCapabilities.minImageCount;
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Max ImageAllocation Count: " << SurfaceCapabilities.maxImageCount;
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Current Extent: (" << SurfaceCapabilities.currentExtent.width << ", " << SurfaceCapabilities.currentExtent.height << ")";
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Min ImageAllocation Extent: (" << SurfaceCapabilities.minImageExtent.width << ", " << SurfaceCapabilities.minImageExtent.height << ")";
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Max ImageAllocation Extent: (" << SurfaceCapabilities.maxImageExtent.width << ", " << SurfaceCapabilities.maxImageExtent.height << ")";
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Max ImageAllocation Array Layers: " << SurfaceCapabilities.maxImageArrayLayers;
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Supported Transforms: " << TransformFlagToString(static_cast<VkSurfaceTransformFlagBitsKHR>(SurfaceCapabilities.supportedTransforms));
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Current Transform: " << TransformFlagToString(SurfaceCapabilities.currentTransform);
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Supported Composite Alpha: " << CompositeAlphaFlagToString(SurfaceCapabilities.supportedCompositeAlpha);
    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Supported Usage Flags: " << ImageUsageFlagToString(static_cast<VkImageUsageFlagBits>(SurfaceCapabilities.supportedUsageFlags));
}

void ListAvailablePhysicalDeviceSurfaceFormats(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Listing available vulkan physical device surface formats...";

    for (auto const& [Format, ColorSpace]: GetAvailablePhysicalDeviceSurfaceFormats(VulkanSurface))
    {
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Format: " << SurfaceFormatToString(Format);
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Color Space: " << ColorSpaceModeToString(ColorSpace) << std::endl;
    }
}

void ListAvailablePhysicalDeviceSurfacePresentationModes(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Listing available vulkan physical device presentation modes...";

    for (VkPresentModeKHR const& FormatIter: GetAvailablePhysicalDeviceSurfacePresentationModes(VulkanSurface))
    {
        BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Mode: " << PresentationModeToString(FormatIter);
    }
}
#endif

bool IsPhysicalDeviceSuitable(VkPhysicalDevice const& Device)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (Device == VK_NULL_HANDLE)
    {
        return false;
    }

    VkPhysicalDeviceProperties DeviceProperties;
    vkGetPhysicalDeviceProperties(Device, &DeviceProperties);

    VkPhysicalDeviceFeatures SupportedFeatures;
    vkGetPhysicalDeviceFeatures(Device, &SupportedFeatures);

    return DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && SupportedFeatures.samplerAnisotropy != 0U;
}

void RenderCore::PickPhysicalDevice(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Picking a physical device";

    for (VkPhysicalDevice const& DeviceIter: GetAvailablePhysicalDevices())
    {
        if (g_PhysicalDevice == VK_NULL_HANDLE && IsPhysicalDeviceSuitable(DeviceIter))
        {
            g_PhysicalDevice = DeviceIter;
            break;
        }
    }

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("No suitable Vulkan physical device found.");
    }

#ifdef _DEBUG
    ListAvailablePhysicalDevices();
    ListAvailablePhysicalDeviceLayers();

    for (char const* const& RequiredLayerIter: g_RequiredDeviceLayers)
    {
        ListAvailablePhysicalDeviceLayerExtensions(RequiredLayerIter);
    }

    for (char const* const& DebugLayerIter: g_DebugDeviceLayers)
    {
        ListAvailablePhysicalDeviceLayerExtensions(DebugLayerIter);
    }

    ListAvailablePhysicalDeviceSurfaceCapabilities(VulkanSurface);
    ListAvailablePhysicalDeviceSurfaceFormats(VulkanSurface);
    ListAvailablePhysicalDeviceSurfacePresentationModes(VulkanSurface);
#endif
}

void RenderCore::CreateLogicalDevice(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    std::optional<std::uint8_t> GraphicsQueueFamilyIndex {std::nullopt};
    std::optional<std::uint8_t> PresentationQueueFamilyIndex {std::nullopt};
    std::optional<std::uint8_t> TransferQueueFamilyIndex {std::nullopt};

    if (!GetQueueFamilyIndices(VulkanSurface, GraphicsQueueFamilyIndex, PresentationQueueFamilyIndex, TransferQueueFamilyIndex))
    {
        throw std::runtime_error("Failed to get queue family indices.");
    }

    g_GraphicsQueue.first     = GraphicsQueueFamilyIndex.value();
    g_PresentationQueue.first = PresentationQueueFamilyIndex.value();
    g_TransferQueue.first     = TransferQueueFamilyIndex.value();

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Creating vulkan logical device";

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    std::vector Layers(std::cbegin(g_RequiredDeviceLayers), std::cend(g_RequiredDeviceLayers));
    std::vector Extensions(std::cbegin(g_RequiredDeviceExtensions), std::cend(g_RequiredDeviceExtensions));

#ifdef _DEBUG
    Layers.insert(std::cend(Layers), std::cbegin(g_DebugDeviceLayers), std::cend(g_DebugDeviceLayers));
    Extensions.insert(std::cend(Extensions), std::cbegin(g_DebugDeviceExtensions), std::cend(g_DebugDeviceExtensions));
#endif

    std::unordered_map<std::uint8_t, std::uint8_t> QueueFamilyIndices {{g_GraphicsQueue.first, 1U}};
    if (!QueueFamilyIndices.contains(g_PresentationQueue.first))
    {
        QueueFamilyIndices.emplace(g_PresentationQueue.first, 1U);
    }
    else
    {
        ++QueueFamilyIndices.at(g_PresentationQueue.first);
    }

    if (!QueueFamilyIndices.contains(g_TransferQueue.first))
    {
        QueueFamilyIndices.emplace(g_TransferQueue.first, 1U);
    }
    else
    {
        ++QueueFamilyIndices.at(g_TransferQueue.first);
    }

    g_UniqueQueueFamilyIndices.clear();
    g_UniqueQueueFamilyIndices.reserve(std::size(QueueFamilyIndices));

    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfo;
    QueueCreateInfo.reserve(std::size(QueueFamilyIndices));
    for (auto const& [Index, Count]: QueueFamilyIndices)
    {
        g_UniqueQueueFamilyIndices.push_back(Index);

        QueueCreateInfo.push_back(
                VkDeviceQueueCreateInfo {
                        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .queueFamilyIndex = Index,
                        .queueCount       = Count,
                        .pQueuePriorities = std::data(std::vector(Count, 1.0F))});
    }

    VkPhysicalDeviceRobustness2FeaturesEXT RobustnessFeatures {
            .sType          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
            .nullDescriptor = VK_TRUE};

    VkPhysicalDeviceFeatures2 DeviceFeatures {
            .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext    = &RobustnessFeatures,
            .features = VkPhysicalDeviceFeatures {.samplerAnisotropy = VK_TRUE}};

    VkDeviceCreateInfo const DeviceCreateInfo {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = &DeviceFeatures,
            .queueCreateInfoCount    = static_cast<std::uint32_t>(std::size(QueueCreateInfo)),
            .pQueueCreateInfos       = std::data(QueueCreateInfo),
            .enabledLayerCount       = static_cast<std::uint32_t>(std::size(Layers)),
            .ppEnabledLayerNames     = std::data(Layers),
            .enabledExtensionCount   = static_cast<std::uint32_t>(std::size(Extensions)),
            .ppEnabledExtensionNames = std::data(Extensions),
            .pEnabledFeatures        = nullptr};

    CheckVulkanResult(vkCreateDevice(g_PhysicalDevice, &DeviceCreateInfo, nullptr, &g_Device));

    if (vkGetDeviceQueue(g_Device, g_GraphicsQueue.first, 0U, &g_GraphicsQueue.second);
        g_GraphicsQueue.second == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to get graphics queue.");
    }

    if (vkGetDeviceQueue(g_Device, g_PresentationQueue.first, 0U, &g_PresentationQueue.second);
        g_PresentationQueue.second == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to get presentation queue.");
    }

    if (vkGetDeviceQueue(g_Device, g_TransferQueue.first, 0U, &g_TransferQueue.second);
        g_TransferQueue.second == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to get transfer queue.");
    }
}

bool RenderCore::UpdateDeviceProperties(GLFWwindow* const Window, VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    g_DeviceProperties.Capabilities = GetAvailablePhysicalDeviceSurfaceCapabilities(VulkanSurface);

    std::vector<VkSurfaceFormatKHR> const SupportedFormats = GetAvailablePhysicalDeviceSurfaceFormats(VulkanSurface);
    if (std::empty(SupportedFormats))
    {
        throw std::runtime_error("No supported surface formats found.");
    }

    std::vector<VkPresentModeKHR> const SupportedPresentationModes = GetAvailablePhysicalDeviceSurfacePresentationModes(VulkanSurface);
    if (std::empty(SupportedFormats))
    {
        throw std::runtime_error("No supported presentation modes found.");
    }

    if (g_DeviceProperties.Capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        g_DeviceProperties.Extent = g_DeviceProperties.Capabilities.currentExtent;
    }
    else
    {
        g_DeviceProperties.Extent = GetWindowExtent(Window, g_DeviceProperties.Capabilities);
    }

    g_DeviceProperties.Format = SupportedFormats.front();
    if (auto const MatchingFormat = std::ranges::find_if(
                SupportedFormats,
                [](VkSurfaceFormatKHR const& Iter) {
                    return Iter.format == VK_FORMAT_B8G8R8A8_SRGB && Iter.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                });
        MatchingFormat != std::cend(SupportedFormats))
    {
        g_DeviceProperties.Format = *MatchingFormat;
    }

    g_DeviceProperties.Mode = VK_PRESENT_MODE_FIFO_KHR;
    if (auto const MatchingMode = std::ranges::find_if(
                SupportedPresentationModes,
                [](VkPresentModeKHR const& Iter) {
                    return Iter == VK_PRESENT_MODE_MAILBOX_KHR;
                });
        MatchingMode != std::cend(SupportedPresentationModes))
    {
        g_DeviceProperties.Mode = *MatchingMode;
    }

    for (std::vector const PreferredDepthFormats = {
                 VK_FORMAT_D32_SFLOAT,
                 VK_FORMAT_D32_SFLOAT_S8_UINT,
                 VK_FORMAT_D24_UNORM_S8_UINT};
         VkFormat const& FormatIter: PreferredDepthFormats)
    {
        VkFormatProperties FormatProperties;
        vkGetPhysicalDeviceFormatProperties(g_PhysicalDevice, FormatIter, &FormatProperties);

        if ((FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0U)
        {
            g_DeviceProperties.DepthFormat = FormatIter;
            break;
        }
    }

    return g_DeviceProperties.IsValid();
}

DeviceProperties& RenderCore::GetDeviceProperties()
{
    return g_DeviceProperties;
}

VkDevice& RenderCore::GetLogicalDevice()
{
    return g_Device;
}

VkPhysicalDevice& RenderCore::GetPhysicalDevice()
{
    return g_PhysicalDevice;
}

std::pair<std::uint8_t, VkQueue>& RenderCore::GetGraphicsQueue()
{
    return g_GraphicsQueue;
}

std::pair<std::uint8_t, VkQueue>& RenderCore::GetPresentationQueue()
{
    return g_PresentationQueue;
}

std::pair<std::uint8_t, VkQueue>& RenderCore::GetTransferQueue()
{
    return g_TransferQueue;
}

std::vector<std::uint32_t> RenderCore::GetUniqueQueueFamilyIndicesU32()
{
    std::vector<std::uint32_t> QueueFamilyIndicesU32(std::size(g_UniqueQueueFamilyIndices));

    std::ranges::transform(
            g_UniqueQueueFamilyIndices,
            std::begin(QueueFamilyIndicesU32),
            [](std::uint8_t const& Index) {
                return static_cast<std::uint32_t>(Index);
            });

    return QueueFamilyIndicesU32;
}

std::uint32_t RenderCore::GetMinImageCount()
{
    bool const SupportsTripleBuffering = g_DeviceProperties.Capabilities.minImageCount < 3U && g_DeviceProperties.Capabilities.maxImageCount >= 3U;
    return SupportsTripleBuffering ? 3U : g_DeviceProperties.Capabilities.minImageCount;
}

void RenderCore::ReleaseDeviceResources()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    BOOST_LOG_TRIVIAL(debug) << "[" << __func__ << "]: Releasing vulkan device resources";

    vkDestroyDevice(g_Device, nullptr);
    g_Device = VK_NULL_HANDLE;

    g_PhysicalDevice           = VK_NULL_HANDLE;
    g_GraphicsQueue.second     = VK_NULL_HANDLE;
    g_PresentationQueue.second = VK_NULL_HANDLE;
    g_TransferQueue.second     = VK_NULL_HANDLE;
}

std::vector<VkPhysicalDevice> RenderCore::GetAvailablePhysicalDevices()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    VkInstance const& VulkanInstance = volkGetLoadedInstance();

    std::uint32_t DeviceCount = 0U;
    CheckVulkanResult(vkEnumeratePhysicalDevices(VulkanInstance, &DeviceCount, nullptr));

    std::vector<VkPhysicalDevice> Output(DeviceCount, VK_NULL_HANDLE);
    CheckVulkanResult(vkEnumeratePhysicalDevices(VulkanInstance, &DeviceCount, std::data(Output)));

    return Output;
}

std::vector<VkExtensionProperties> RenderCore::GetAvailablePhysicalDeviceExtensions()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    std::uint32_t ExtensionsCount = 0;
    CheckVulkanResult(vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &ExtensionsCount, nullptr));

    std::vector<VkExtensionProperties> Output(ExtensionsCount);
    CheckVulkanResult(vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &ExtensionsCount, std::data(Output)));

    return Output;
}

std::vector<VkLayerProperties> RenderCore::GetAvailablePhysicalDeviceLayers()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    std::uint32_t LayersCount = 0;
    CheckVulkanResult(vkEnumerateDeviceLayerProperties(g_PhysicalDevice, &LayersCount, nullptr));

    std::vector<VkLayerProperties> Output(LayersCount);
    CheckVulkanResult(vkEnumerateDeviceLayerProperties(g_PhysicalDevice, &LayersCount, std::data(Output)));

    return Output;
}

std::vector<VkExtensionProperties> RenderCore::GetAvailablePhysicalDeviceLayerExtensions(std::string_view const& LayerName)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    if (std::vector<std::string> const AvailableLayers = GetAvailablePhysicalDeviceLayersNames();
        std::ranges::find(AvailableLayers, LayerName) == std::cend(AvailableLayers))
    {
        return {};
    }

    std::uint32_t ExtensionsCount = 0;
    CheckVulkanResult(vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, std::data(LayerName), &ExtensionsCount, nullptr));

    std::vector<VkExtensionProperties> Output(ExtensionsCount);
    CheckVulkanResult(vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, std::data(LayerName), &ExtensionsCount, std::data(Output)));

    return Output;
}

std::vector<std::string> RenderCore::GetAvailablePhysicalDeviceExtensionsNames()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    std::vector<std::string> Output;
    for (VkExtensionProperties const& ExtensionIter: GetAvailablePhysicalDeviceExtensions())
    {
        Output.emplace_back(ExtensionIter.extensionName);
    }

    return Output;
}

std::vector<std::string> RenderCore::GetAvailablePhysicalDeviceLayerExtensionsNames(std::string_view const& LayerName)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    std::vector<std::string> Output;
    for (VkExtensionProperties const& ExtensionIter: GetAvailablePhysicalDeviceLayerExtensions(LayerName))
    {
        Output.emplace_back(ExtensionIter.extensionName);
    }

    return Output;
}

std::vector<std::string> RenderCore::GetAvailablePhysicalDeviceLayersNames()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    std::vector<std::string> Output;
    for (VkLayerProperties const& LayerIter: GetAvailablePhysicalDeviceLayers())
    {
        Output.emplace_back(LayerIter.layerName);
    }

    return Output;
}

VkSurfaceCapabilitiesKHR RenderCore::GetAvailablePhysicalDeviceSurfaceCapabilities(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    VkSurfaceCapabilitiesKHR Output;
    CheckVulkanResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_PhysicalDevice, VulkanSurface, &Output));

    return Output;
}

std::vector<VkSurfaceFormatKHR> RenderCore::GetAvailablePhysicalDeviceSurfaceFormats(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    std::uint32_t Count = 0U;
    CheckVulkanResult(vkGetPhysicalDeviceSurfaceFormatsKHR(g_PhysicalDevice, VulkanSurface, &Count, nullptr));

    std::vector Output(Count, VkSurfaceFormatKHR());
    CheckVulkanResult(vkGetPhysicalDeviceSurfaceFormatsKHR(g_PhysicalDevice, VulkanSurface, &Count, std::data(Output)));

    return Output;
}

std::vector<VkPresentModeKHR> RenderCore::GetAvailablePhysicalDeviceSurfacePresentationModes(VkSurfaceKHR const& VulkanSurface)
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    std::uint32_t Count = 0U;
    CheckVulkanResult(vkGetPhysicalDeviceSurfacePresentModesKHR(g_PhysicalDevice, VulkanSurface, &Count, nullptr));

    std::vector Output(Count, VkPresentModeKHR());
    CheckVulkanResult(vkGetPhysicalDeviceSurfacePresentModesKHR(g_PhysicalDevice, VulkanSurface, &Count, std::data(Output)));

    return Output;
}

VkDeviceSize RenderCore::GetMinUniformBufferOffsetAlignment()
{
    Timer::ScopedTimer TotalSceneAllocationTimer(__FUNCTION__);

    if (g_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Vulkan physical device is invalid.");
    }

    VkPhysicalDeviceProperties DeviceProperties;
    vkGetPhysicalDeviceProperties(g_PhysicalDevice, &DeviceProperties);

    return DeviceProperties.limits.minUniformBufferOffsetAlignment;
}