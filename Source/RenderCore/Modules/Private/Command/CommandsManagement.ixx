// Author: Lucas Vilas-Boas
// Year : 2023
// Repo : https://github.com/lucoiso/VulkanRenderer

module;

#include <volk.h>

export module RenderCore.Management.CommandsManagement;

import <optional>;
import <cstdint>;
import <memory>;
import <vector>;

import RenderCore.Management.BufferManagement;
import RenderCore.Management.PipelineManagement;
import RenderCore.Types.Camera;
import RenderCore.Types.Object;

namespace RenderCore
{
    export void ReleaseCommandsResources();
    export [[nodiscard]] VkCommandPool CreateCommandPool(std::uint8_t);

    export void CreateCommandsSynchronizationObjects();
    export void DestroyCommandsSynchronizationObjects();

    export [[nodiscard]] std::optional<std::int32_t> RequestSwapChainImage(VkSwapchainKHR const&);

    export void RecordCommandBuffers(std::uint32_t, std::uint32_t, Camera const&, BufferManager const&, PipelineManager&, std::vector<std::shared_ptr<Object>> const&);
    export void SubmitCommandBuffers(VkQueue const&);

    export void PresentFrame(VkQueue const&, std::uint32_t, VkSwapchainKHR const&);

    export void InitializeSingleCommandQueue(VkCommandPool&, std::vector<VkCommandBuffer>&, std::uint8_t);
    export void FinishSingleCommandQueue(VkQueue const&, VkCommandPool const&, std::vector<VkCommandBuffer>&);
}// namespace RenderCore