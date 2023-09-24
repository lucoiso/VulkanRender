// Author: Lucas Vilas-Boas
// Year : 2023
// Repo : https://github.com/lucoiso/VulkanLearning

#include <RenderCore/Window.h>
#include <memory>

int main(int Argc, char *Argv[])
{
    const std::unique_ptr<RenderCore::Window> Window = std::make_unique<RenderCore::Window>();
    if (Window->Initialize(600u, 600u, "Vulkan Project"))
    {
        while (Window->IsOpen())
        {
            Window->PollEvents();
        }

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
