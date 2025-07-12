#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <algorithm>

namespace Pyramid {

void UniformBufferLayout::AddElement(const std::string& name, u32 size, u32 arraySize)
{
    // Calculate aligned offset for std140 layout
    u32 alignedOffset = CalculateAlignedOffset(size, m_size);
    
    Element element;
    element.name = name;
    element.offset = alignedOffset;
    element.size = size;
    element.arraySize = arraySize;
    
    m_elements.push_back(element);
    
    // Update total size (including array elements)
    m_size = alignedOffset + (size * arraySize);
    
    // Ensure the total size is aligned to 16 bytes (std140 requirement)
    m_size = (m_size + 15) & ~15;
}

u32 UniformBufferLayout::GetOffset(const std::string& name) const
{
    auto it = std::find_if(m_elements.begin(), m_elements.end(),
        [&name](const Element& element) {
            return element.name == name;
        });
    
    if (it != m_elements.end())
    {
        return it->offset;
    }
    
    return UINT32_MAX; // Not found
}

u32 UniformBufferLayout::CalculateAlignedOffset(u32 size, u32 currentOffset) const
{
    // std140 alignment rules:
    // - scalars (4 bytes): 4-byte aligned
    // - vec2 (8 bytes): 8-byte aligned
    // - vec3/vec4 (12/16 bytes): 16-byte aligned
    // - mat3 (36 bytes): 16-byte aligned (treated as 3 vec4s)
    // - mat4 (64 bytes): 16-byte aligned
    // - arrays: each element aligned to 16 bytes
    
    u32 alignment;
    
    if (size <= 4)
    {
        alignment = 4;  // scalar
    }
    else if (size <= 8)
    {
        alignment = 8;  // vec2
    }
    else
    {
        alignment = 16; // vec3, vec4, matrices, arrays
    }
    
    // Calculate aligned offset
    return (currentOffset + alignment - 1) & ~(alignment - 1);
}

} // namespace Pyramid
