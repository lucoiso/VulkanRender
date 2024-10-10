// Author: Lucas Vilas-Boas
// Year : 2024
// Repo : https://github.com/lucoiso/vulkan-renderer

module;

#include <meshoptimizer.h>

module RenderCore.Types.Mesh;

import RenderCore.Runtime.Memory;
import RenderCore.Runtime.Pipeline;
import RenderCore.Runtime.Scene;
import RenderCore.Runtime.Device;
import RenderCore.Types.Material;
import RenderCore.Types.Texture;
import RenderCore.Types.UniformBufferObject;
import RenderCore.Utils.Helpers;
import RenderCore.Utils.Constants;

using namespace RenderCore;

Mesh::Mesh(std::uint32_t const ID, strzilla::string_view const Path)
    : Resource(ID, Path)
{
}

Mesh::Mesh(std::uint32_t const ID, strzilla::string_view const Path, strzilla::string_view const Name)
    : Resource(ID, Path, Name)
{
}

void Mesh::SetupMeshlets(std::vector<Vertex> &&Vertices, std::vector<std::uint32_t> &&Indices)
{
    std::size_t VertexCount = std::size(Vertices);
    std::size_t IndexCount  = std::size(Indices);

    std::vector<std::uint32_t> Remap(IndexCount);
    VertexCount = meshopt_generateVertexRemap(std::data(Remap),
                                              std::data(Indices),
                                              IndexCount,
                                              std::data(Vertices),
                                              VertexCount,
                                              sizeof(Vertex));

    std::vector<Vertex>    NewVertices(VertexCount);
    std::vector<glm::uint> NewIndices(IndexCount);

    meshopt_remapIndexBuffer(std::data(NewIndices), std::data(Indices), IndexCount, std::data(Remap));
    meshopt_remapVertexBuffer(std::data(NewVertices), std::data(Vertices), VertexCount, sizeof(Vertex), std::data(Remap));

    Vertices = std::move(NewVertices);
    Indices  = std::move(NewIndices);

    meshopt_optimizeVertexCache(std::data(Indices), std::data(Indices), IndexCount, std::size(Vertices));

    meshopt_optimizeOverdraw(std::data(Indices),
                             std::data(Indices),
                             IndexCount,
                             &Vertices.at(0).Position.x,
                             std::size(Vertices),
                             sizeof(Vertex),
                             1.05f);

    meshopt_optimizeVertexFetch(std::data(Vertices),
                                std::data(Indices),
                                IndexCount,
                                std::data(Vertices),
                                std::size(Vertices),
                                sizeof(Vertex));

    VertexCount = std::size(Vertices);
    IndexCount  = std::size(Indices);

    m_Vertices = std::move(Vertices);
    m_Indices  = std::move(Indices);

    std::size_t const MaxMeshlets = meshopt_buildMeshletsBound(IndexCount, g_MaxMeshletVertices, g_MaxMeshletPrimitives);

    std::vector<meshopt_Meshlet> OptimizerMeshlets(MaxMeshlets);
    std::vector<std::uint32_t> MeshletVertices(MaxMeshlets * g_MaxMeshletVertices);
    std::vector<std::uint8_t> MeshletTriangles(MaxMeshlets * g_MaxMeshletIndices);

    std::size_t const NumMeshlets = meshopt_buildMeshlets(std::data(OptimizerMeshlets),
                                                          std::data(MeshletVertices),
                                                          std::data(MeshletTriangles),
                                                          std::data(m_Indices),
                                                          IndexCount,
                                                          &m_Vertices.at(0).Position.x,
                                                          VertexCount,
                                                          sizeof(Vertex),
                                                          g_MaxMeshletVertices,
                                                          g_MaxMeshletPrimitives,
                                                          0.F);

    auto const &[MeshletVertexOffset,
                 MeshletTriangleOffset,
                 MeshletVertexCount,
                 MeshletTriangleCount] = OptimizerMeshlets.at(NumMeshlets - 1U);

    std::size_t NumMeshletVertices = MeshletVertexOffset + MeshletVertexCount;
    MeshletVertices.resize(NumMeshletVertices);

    std::size_t const NumMeshletTriangles = MeshletTriangleOffset + (MeshletTriangleCount * 3 + 3 & ~3);
    MeshletTriangles.resize(NumMeshletTriangles);
    OptimizerMeshlets.resize(NumMeshlets);

    meshopt_optimizeMeshlet(&MeshletVertices.at(MeshletVertexOffset), &MeshletTriangles.at(MeshletTriangleOffset), MeshletTriangleCount, MeshletVertexCount);

    m_Meshlets.resize(NumMeshlets);

    for (std::size_t MeshletIt = 0; MeshletIt < NumMeshlets; ++MeshletIt)
    {
        auto const &[LocalMeshletVertexOffset,
                     LocalMeshletTriangleOffset,
                     LocalMeshletVertexCount,
                     LocalMeshletTriangleCount] = OptimizerMeshlets[MeshletIt];

        Meshlet &NewMeshlet = m_Meshlets.at(MeshletIt);

        NewMeshlet.IndexCount   = LocalMeshletTriangleCount * 3U;
        NewMeshlet.VertexCount  = LocalMeshletVertexCount;
        NewMeshlet.IndexOffset  = LocalMeshletTriangleOffset;
        NewMeshlet.VertexOffset = LocalMeshletVertexOffset;
    }
}

void Mesh::SetupUniformDescriptor()
{
    m_MappedData = GetAllocationMappedData();

    UpdateUniformBuffers();
    UpdatePrimitivesBuffers();
}

void Mesh::UpdateUniformBuffers() const
{
    if (!m_MappedData || !IsRenderDirty())
    {
        return;
    }

    ModelUniformData const UpdatedModelUBO {.ProjectionView = GetCamera().GetProjectionMatrix(),
                                            .Model          = m_Transform.GetMatrix() * m_Transform.GetMatrix()};

    auto const UniformData = static_cast<char*>(m_MappedData) + GetUniformOffset();
    std::memcpy(UniformData, &UpdatedModelUBO, sizeof(ModelUniformData));

    auto const MaterialData = UniformData + sizeof(ModelUniformData);
    std::memcpy(MaterialData, &m_MaterialData, sizeof(MaterialData));

    SetRenderDirty(false);
}

void RenderCore::Mesh::UpdatePrimitivesBuffers() const
{
    if (!m_MappedData)
    {
        return;
    }

    auto const MeshletsData = static_cast<char*>(m_MappedData) + GetMeshletsOffset();
    std::memcpy(MeshletsData, std::data(m_Meshlets), std::size(m_Meshlets) * sizeof(Meshlet));

    auto const IndicesData = static_cast<char*>(m_MappedData) + GetIndicesOffset();
    std::memcpy(IndicesData, std::data(m_Indices), std::size(m_Indices) * sizeof(glm::uint));

    auto const VerticesData = static_cast<char*>(m_MappedData) + GetVerticesOffset();
    std::memcpy(VerticesData, std::data(m_Vertices), std::size(m_Vertices) * sizeof(Vertex));
}

void Mesh::DrawObject(VkCommandBuffer const &CommandBuffer, VkPipelineLayout const &PipelineLayout, std::uint32_t const ObjectIndex) const
{
    auto const &[SceneData,
                 ModelData,
                 MaterialData,
                 MeshletsData,
                 IndicesData,
                 VerticesData,
                 TextureData] = GetPipelineDescriptorData();

    std::array const BufferBindingInfos {
            VkDescriptorBufferBindingInfoEXT
            {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                    .address = SceneData.BufferDeviceAddress.deviceAddress,
                    .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            },
            VkDescriptorBufferBindingInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                    .address = ModelData.BufferDeviceAddress.deviceAddress,
                    .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            },
            VkDescriptorBufferBindingInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                    .address = MaterialData.BufferDeviceAddress.deviceAddress,
                    .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            },
            VkDescriptorBufferBindingInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                    .address = MeshletsData.BufferDeviceAddress.deviceAddress,
                    .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            },
            VkDescriptorBufferBindingInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                    .address = IndicesData.BufferDeviceAddress.deviceAddress,
                    .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            },
            VkDescriptorBufferBindingInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                    .address = VerticesData.BufferDeviceAddress.deviceAddress,
                    .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            },
            VkDescriptorBufferBindingInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                    .address = TextureData.BufferDeviceAddress.deviceAddress,
                    .usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            }
    };

    vkCmdBindDescriptorBuffersEXT(CommandBuffer, static_cast<std::uint32_t>(std::size(BufferBindingInfos)), std::data(BufferBindingInfos));

    constexpr std::array BufferIndices { 0U, 1U, 2U, 3U, 4U, 5U, 6U };
    constexpr auto NumTextures = static_cast<std::uint8_t>(TextureType::Count);

    std::array const BufferOffsets {
            SceneData.LayoutOffset,
            ObjectIndex * ModelData.LayoutSize    + ModelData.LayoutOffset,
            ObjectIndex * MaterialData.LayoutSize + MaterialData.LayoutOffset,
            ObjectIndex * MeshletsData.LayoutSize + MeshletsData.LayoutOffset,
            ObjectIndex * IndicesData.LayoutSize  + IndicesData.LayoutOffset,
            ObjectIndex * VerticesData.LayoutSize + VerticesData.LayoutOffset,
            ObjectIndex * NumTextures * TextureData.LayoutSize + TextureData.LayoutOffset
    };

    vkCmdSetDescriptorBufferOffsetsEXT(CommandBuffer,
                                       VK_PIPELINE_BIND_POINT_GRAPHICS,
                                       PipelineLayout,
                                       0U,
                                       static_cast<std::uint32_t>(std::size(BufferBindingInfos)),
                                       std::data(BufferIndices),
                                       std::data(BufferOffsets));

    std::uint32_t const NumMeshlets = GetNumMeshlets();
    std::uint32_t const NumTasks = std::ceil(NumMeshlets + g_MaxMeshTasks - 1U) / g_MaxMeshTasks;

    vkCmdDrawMeshTasksEXT(CommandBuffer, NumTasks, 1U, 1U);
}
