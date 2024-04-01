// Author: Lucas Vilas-Boas
// Year : 2024
// Repo : https://github.com/lucoiso/VulkanRenderer

module;
#include <Volk/volk.h>
#include <algorithm>
#include <vector>

#ifdef GLFW_INCLUDE_VULKAN
    #undef GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

module RenderCore.Runtime.SwapChain;

import RenderCore.Runtime.Device;
import RenderCore.Runtime.Synchronization;
import RenderCore.Runtime.Memory;
import RenderCore.Utils.Helpers;
import RenderCore.Utils.Constants;

using namespace RenderCore;

VkSurfaceKHR                 g_Surface {VK_NULL_HANDLE};
VkSwapchainKHR               g_SwapChain {VK_NULL_HANDLE};
VkSwapchainKHR               g_OldSwapChain {VK_NULL_HANDLE};
VkFormat                     g_SwapChainImageFormat {VK_FORMAT_UNDEFINED};
VkExtent2D                   g_SwapChainExtent {0U, 0U};
std::vector<ImageAllocation> g_SwapChainImages {};

void RenderCore::CreateVulkanSurface(GLFWwindow *const Window)
{
    CheckVulkanResult(glfwCreateWindowSurface(volkGetLoadedInstance(), Window, nullptr, &g_Surface));
}

void RenderCore::CreateSwapChain(SurfaceProperties const &SurfaceProperties, VkSurfaceCapabilitiesKHR const &Capabilities)
{
    auto const QueueFamilyIndices      = GetUniqueQueueFamilyIndicesU32();
    auto const QueueFamilyIndicesCount = static_cast<std::uint32_t>(std::size(QueueFamilyIndices));

    g_OldSwapChain         = g_SwapChain;
    g_SwapChainExtent      = SurfaceProperties.Extent;
    g_SwapChainImageFormat = SurfaceProperties.Format.format;

    VkSwapchainCreateInfoKHR const SwapChainCreateInfo {.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                                        .surface          = GetSurface(),
                                                        .minImageCount    = g_MinImageCount,
                                                        .imageFormat      = g_SwapChainImageFormat,
                                                        .imageColorSpace  = SurfaceProperties.Format.colorSpace,
                                                        .imageExtent      = g_SwapChainExtent,
                                                        .imageArrayLayers = 1U,
                                                        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                        .imageSharingMode
                                                        = QueueFamilyIndicesCount > 1U ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
                                                        .queueFamilyIndexCount = QueueFamilyIndicesCount,
                                                        .pQueueFamilyIndices   = std::data(QueueFamilyIndices),
                                                        .preTransform          = Capabilities.currentTransform,
                                                        .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                                        .presentMode           = SurfaceProperties.Mode,
                                                        .clipped               = VK_TRUE,
                                                        .oldSwapchain          = g_OldSwapChain};

    CheckVulkanResult(vkCreateSwapchainKHR(volkGetLoadedDevice(), &SwapChainCreateInfo, nullptr, &g_SwapChain));

    if (g_OldSwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(volkGetLoadedDevice(), g_OldSwapChain, nullptr);
        g_OldSwapChain = VK_NULL_HANDLE;
    }

    std::uint32_t Count = 0U;
    CheckVulkanResult(vkGetSwapchainImagesKHR(volkGetLoadedDevice(), g_SwapChain, &Count, nullptr));

    std::vector<VkImage> SwapChainImages(Count, VK_NULL_HANDLE);
    CheckVulkanResult(vkGetSwapchainImagesKHR(volkGetLoadedDevice(), g_SwapChain, &Count, std::data(SwapChainImages)));

    g_SwapChainImages.resize(Count, ImageAllocation());
    std::ranges::transform(SwapChainImages,
                           std::begin(g_SwapChainImages),
                           [](VkImage const &Image)
                           {
                               return ImageAllocation {.Image = Image};
                           });

    CreateSwapChainImageViews(g_SwapChainImages, SurfaceProperties.Format.format);
}

std::optional<std::int32_t> RenderCore::RequestSwapChainImage()
{
    std::uint32_t  Output          = 0U;
    VkResult const OperationResult = vkAcquireNextImageKHR(volkGetLoadedDevice(), g_SwapChain, g_Timeout, GetImageAvailableSemaphore(), GetFence(), &Output);
    WaitAndResetFences();

    if (OperationResult != VK_SUCCESS)
    {
        return std::nullopt;
    }

    return std::optional {static_cast<std::int32_t>(Output)};
}

void RenderCore::CreateSwapChainImageViews(std::vector<ImageAllocation> &Images, VkFormat const ImageFormat)
{
    std::ranges::for_each(Images,
                          [&](ImageAllocation &ImageIter)
                          {
                              CreateImageView(ImageIter.Image, ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, ImageIter.View);
                          });
}

void RenderCore::PresentFrame(std::uint32_t const ImageIndice)
{
    VkPresentInfoKHR const PresentInfo {.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                        .waitSemaphoreCount = 1U,
                                        .pWaitSemaphores    = &GetRenderFinishedSemaphore(),
                                        .swapchainCount     = 1U,
                                        .pSwapchains        = &g_SwapChain,
                                        .pImageIndices      = &ImageIndice,
                                        .pResults           = nullptr};

    CheckVulkanResult(vkQueuePresentKHR(GetPresentationQueue().second, &PresentInfo));
}

void RenderCore::ReleaseSwapChainResources()
{
    DestroySwapChainImages();

    if (g_SwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(volkGetLoadedDevice(), g_SwapChain, nullptr);
        g_SwapChain = VK_NULL_HANDLE;
    }

    if (g_OldSwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(volkGetLoadedDevice(), g_OldSwapChain, nullptr);
        g_OldSwapChain = VK_NULL_HANDLE;
    }

    vkDestroySurfaceKHR(volkGetLoadedInstance(), g_Surface, nullptr);
    g_Surface = VK_NULL_HANDLE;
}

void RenderCore::DestroySwapChainImages()
{
    std::ranges::for_each(g_SwapChainImages,
                          [&](ImageAllocation &ImageIter)
                          {
                              ImageIter.DestroyResources(GetAllocator());
                          });
    g_SwapChainImages.clear();
}

VkSurfaceKHR const &RenderCore::GetSurface()
{
    return g_Surface;
}

VkSwapchainKHR const &RenderCore::GetSwapChain()
{
    return g_SwapChain;
}

VkExtent2D const &RenderCore::GetSwapChainExtent()
{
    return g_SwapChainExtent;
}

VkFormat const &RenderCore::GetSwapChainImageFormat()
{
    return g_SwapChainImageFormat;
}

std::vector<ImageAllocation> const &RenderCore::GetSwapChainImages()
{
    return g_SwapChainImages;
}