// Author: Lucas Vilas-Boas
// Year : 2024
// Repo : https://github.com/lucoiso/vulkan-renderer

module;

#include <array>
#include <Volk/volk.h>

module RenderCore.Runtime.Synchronization;

import RenderCore.Utils.Helpers;
import RenderCore.Utils.Constants;
import RenderCore.Runtime.Device;
import RenderCore.Runtime.Command;

using namespace RenderCore;

std::array<VkSemaphore, g_ImageCount> g_ImageAvailableSemaphores {};
std::array<VkSemaphore, g_ImageCount> g_RenderFinishedSemaphores {};
std::array<VkFence, g_ImageCount>     g_Fences {};
std::array<bool, g_ImageCount>        g_FenceInUse {};

void RenderCore::WaitAndResetFence(std::uint32_t const Index)
{
    if (g_Fences.at(Index) == VK_NULL_HANDLE || !g_FenceInUse.at(Index))
    {
        return;
    }

    VkDevice const &LogicalDevice = GetLogicalDevice();
    CheckVulkanResult(vkWaitForFences(LogicalDevice, 1U, &g_Fences.at(Index), VK_FALSE, g_Timeout));
    CheckVulkanResult(vkResetFences(LogicalDevice, 1U, &g_Fences.at(Index)));
    g_FenceInUse.at(Index) = false;

    ResetCommandPool(Index);
}

void RenderCore::CreateSynchronizationObjects()
{
    VkDevice const &LogicalDevice = GetLogicalDevice();

    constexpr VkSemaphoreCreateInfo SemaphoreCreateInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    for (auto &Semaphore : g_ImageAvailableSemaphores)
    {
        CheckVulkanResult(vkCreateSemaphore(LogicalDevice, &SemaphoreCreateInfo, nullptr, &Semaphore));
    }

    for (auto &Semaphore : g_RenderFinishedSemaphores)
    {
        CheckVulkanResult(vkCreateSemaphore(LogicalDevice, &SemaphoreCreateInfo, nullptr, &Semaphore));
    }

    constexpr VkFenceCreateInfo FenceCreateInfo { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
    for (auto &Fence : g_Fences)
    {
        CheckVulkanResult(vkCreateFence(LogicalDevice, &FenceCreateInfo, nullptr, &Fence));
    }

    CheckVulkanResult(vkResetFences(LogicalDevice, static_cast<std::uint32_t>(std::size(g_Fences)), data(g_Fences)));
}

void RenderCore::ReleaseSynchronizationObjects()
{
    VkDevice const &LogicalDevice = GetLogicalDevice();
    vkDeviceWaitIdle(LogicalDevice);

    for (auto &Semaphore : g_ImageAvailableSemaphores)
    {
        if (Semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(LogicalDevice, Semaphore, nullptr);
            Semaphore = VK_NULL_HANDLE;
        }
    }

    for (auto &Semaphore : g_RenderFinishedSemaphores)
    {
        if (Semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(LogicalDevice, Semaphore, nullptr);
            Semaphore = VK_NULL_HANDLE;
        }
    }

    for (auto &Fence : g_Fences)
    {
        if (Fence != VK_NULL_HANDLE)
        {
            vkDestroyFence(LogicalDevice, Fence, nullptr);
            Fence = VK_NULL_HANDLE;
        }
    }

    ResetFenceStatus();
}

void RenderCore::ResetSemaphores()
{
    vkQueueWaitIdle(GetGraphicsQueue().second);

    VkDevice const &                LogicalDevice = GetLogicalDevice();
    constexpr VkSemaphoreCreateInfo SemaphoreCreateInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    for (auto &Semaphore : g_ImageAvailableSemaphores)
    {
        if (Semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(LogicalDevice, Semaphore, nullptr);
            CheckVulkanResult(vkCreateSemaphore(LogicalDevice, &SemaphoreCreateInfo, nullptr, &Semaphore));
        }
    }

    for (auto &Semaphore : g_RenderFinishedSemaphores)
    {
        if (Semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(LogicalDevice, Semaphore, nullptr);
            CheckVulkanResult(vkCreateSemaphore(LogicalDevice, &SemaphoreCreateInfo, nullptr, &Semaphore));
        }
    }
}

void RenderCore::ResetFenceStatus()
{
    for (auto &FenceInUse : g_FenceInUse)
    {
        FenceInUse = false;
    }
}

void RenderCore::SetFenceWaitStatus(std::uint32_t const Index, bool const Value)
{
    g_FenceInUse.at(Index) = Value;
}

bool const &RenderCore::GetFenceWaitStatus(std::uint32_t const Index)
{
    return g_FenceInUse.at(Index);
}

VkSemaphore const &RenderCore::GetImageAvailableSemaphore(std::uint32_t const Index)
{
    return g_ImageAvailableSemaphores.at(Index);
}

VkSemaphore const &RenderCore::GetRenderFinishedSemaphore(std::uint32_t const Index)
{
    return g_RenderFinishedSemaphores.at(Index);
}

VkFence const &RenderCore::GetFence(std::uint32_t const Index)
{
    return g_Fences.at(Index);
}
