#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
    using namespace Pyramid;

    Renderer::CommandBuffer commands;
    commands.Begin();
    commands.DrawIndexed(6, 2, PrimitiveTopology::Triangles);
    commands.DrawArrays(4, 0, 1, PrimitiveTopology::TriangleStrip);
    commands.End();

    const Renderer::CommandBufferDrawStats stats = commands.GetDrawStats();
    if (stats.drawCalls != 2 || stats.vertices != 16 || stats.triangles != 6)
    {
        std::cerr << "CommandBuffer draw statistics mismatch\n";
        return EXIT_FAILURE;
    }

    commands.Reset();
    const Renderer::CommandBufferDrawStats empty = commands.GetDrawStats();
    if (empty.drawCalls != 0 || empty.vertices != 0 || empty.triangles != 0)
    {
        std::cerr << "Reset command buffer retained draw statistics\n";
        return EXIT_FAILURE;
    }

    std::cout << "CommandBuffer statistics tests passed\n";
    return EXIT_SUCCESS;
}
