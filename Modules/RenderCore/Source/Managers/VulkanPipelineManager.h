// Author: Lucas Vilas-Boas
// Year : 2023
// Repo : https://github.com/lucoiso/VulkanLearning

#pragma once

#include "RenderCoreModule.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace RenderCore
{
    class VulkanPipelineManager
    {
    public:
        VulkanPipelineManager() = delete;
        VulkanPipelineManager(const VulkanPipelineManager &) = delete;
        VulkanPipelineManager &operator=(const VulkanPipelineManager &) = delete;

        VulkanPipelineManager(const VkInstance &Instance, const VkDevice &Device);
        ~VulkanPipelineManager();

        void CreateRenderPass(const VkFormat &Format);
        void CreateGraphicsPipeline(const std::vector<VkPipelineShaderStageCreateInfo> &ShaderStages, const VkExtent2D &Extent);
        void CreateDescriptorsAndPipelineCache(const std::vector<VkBuffer>& VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR);
        void UpdateExtent(const VkExtent2D &Extent);

        void Shutdown();

        bool IsInitialized() const;
        [[nodiscard]] const VkRenderPass &GetRenderPass() const;
        [[nodiscard]] const VkPipeline &GetPipeline() const;
        [[nodiscard]] const VkPipelineLayout &GetPipelineLayout() const;
        [[nodiscard]] const VkPipelineCache &GetPipelineCache() const;
        [[nodiscard]] const VkDescriptorSetLayout &GetDescriptorSetLayout() const;
        [[nodiscard]] const VkDescriptorPool &GetDescriptorPool() const;
        [[nodiscard]] const std::vector<VkDescriptorSet> &GetDescriptorSets() const;
        [[nodiscard]] const std::vector<VkViewport> &GetViewports() const;
        [[nodiscard]] const std::vector<VkRect2D> &GetScissors() const;

    private:
        const VkInstance &m_Instance;
        const VkDevice &m_Device;

        VkRenderPass m_RenderPass;
        VkPipeline m_Pipeline;
        VkPipelineLayout m_PipelineLayout;
        VkPipelineCache m_PipelineCache;
        VkDescriptorSetLayout m_DescriptorSetLayout;
        VkDescriptorPool m_DescriptorPool;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        std::vector<VkViewport> m_Viewports;
        std::vector<VkRect2D> m_Scissors;
    };
}
