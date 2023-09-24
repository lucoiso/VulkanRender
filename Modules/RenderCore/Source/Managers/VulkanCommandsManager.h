// Author: Lucas Vilas-Boas
// Year : 2023
// Repo : https://github.com/lucoiso/VulkanLearning

#pragma once

#include "Types/BufferRecordParameters.h"
#include <volk.h>
#include <optional>

namespace RenderCore
{
    constexpr std::uint32_t Timeout = std::numeric_limits<std::uint32_t>::max();

    class VulkanCommandsManager
    {
    public:
        VulkanCommandsManager(const VulkanCommandsManager &) = delete;
        VulkanCommandsManager &operator=(const VulkanCommandsManager &) = delete;

        VulkanCommandsManager();
        ~VulkanCommandsManager();

        static VulkanCommandsManager &Get();

        void Shutdown();

        [[nodiscard]] VkCommandPool CreateCommandPool(const std::uint8_t FamilyQueueIndex);

        void CreateSynchronizationObjects();
        void DestroySynchronizationObjects();

        std::optional<std::int32_t> DrawFrame(const VkSwapchainKHR &SwapChain);

        void RecordCommandBuffers(const VulkanBufferRecordParameters &Parameters);
        void SubmitCommandBuffers(const VkQueue &Queue);
        void PresentFrame(const VkQueue &Queue, const VkSwapchainKHR &SwapChain, const std::uint32_t ImageIndice);

    private:
        void CreateGraphicsCommandPool();
        void AllocateCommandBuffer();
        void WaitAndResetFences();

        static VulkanCommandsManager g_Instance;

        VkCommandPool m_CommandPool;
        VkCommandBuffer m_CommandBuffer;
        VkSemaphore m_ImageAvailableSemaphore;
        VkSemaphore m_RenderFinishedSemaphore;
        VkFence m_Fence;
        bool m_SynchronizationObjectsCreated;

        std::uint8_t m_FrameIndex;
    };
}
