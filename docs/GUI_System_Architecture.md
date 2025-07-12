# GUI System Architecture Design

## Overview
Design and implement a comprehensive immediate-mode GUI system optimized for game engines, providing modern UI capabilities with high performance and flexibility.

## Architecture Decision: Immediate Mode vs Retained Mode

### Selected: **Hybrid Immediate Mode GUI (IMGUI)**

**Advantages:**
- **Simple Integration** - Easy to integrate with existing rendering pipeline
- **Dynamic Content** - Excellent for game UIs that change frequently
- **Memory Efficient** - No persistent widget tree in memory
- **Debug-Friendly** - Perfect for development tools and debug interfaces
- **Performance** - Minimal overhead for simple UIs

**Implementation Strategy:**
- **Core IMGUI** for development tools, debug interfaces, and dynamic game UI
- **Retained Mode Layer** for complex, static UIs (menus, HUDs)
- **Hybrid Approach** combining both paradigms as needed

## Core GUI Architecture

### GUI System Components

```cpp
namespace Pyramid::GUI {

// Core GUI context and state management
class GUIContext {
public:
    static GUIContext& Get();
    
    // Frame management
    void BeginFrame();
    void EndFrame();
    void Render();
    
    // Input handling
    void ProcessInput(const InputEvent& event);
    bool WantCaptureMouse() const;
    bool WantCaptureKeyboard() const;
    
    // Style and theming
    void SetStyle(const GUIStyle& style);
    GUIStyle& GetStyle();
    
    // Font management
    bool LoadFont(const std::string& name, const std::string& filepath, f32 size);
    void SetDefaultFont(const std::string& name);
    
private:
    std::unique_ptr<GUIRenderer> m_renderer;
    std::unique_ptr<GUIInputManager> m_inputManager;
    std::unique_ptr<GUIStyleManager> m_styleManager;
    std::unique_ptr<GUIFontManager> m_fontManager;
};

// Core widget interface
class Widget {
public:
    virtual ~Widget() = default;
    virtual void Render() = 0;
    virtual bool HandleInput(const InputEvent& event) = 0;
    virtual Vec2 GetSize() const = 0;
    virtual void SetPosition(const Vec2& position) = 0;
    virtual bool IsVisible() const = 0;
    virtual void SetVisible(bool visible) = 0;
};

// Layout management
enum class LayoutType {
    None,           // Manual positioning
    Horizontal,     // Horizontal flow
    Vertical,       // Vertical flow
    Grid,           // Grid layout
    Flexbox,        // CSS-like flexbox
    Absolute        // Absolute positioning
};

class Layout {
public:
    virtual ~Layout() = default;
    virtual void CalculateLayout(const std::vector<Widget*>& widgets, const Vec2& availableSize) = 0;
    virtual Vec2 GetMinimumSize(const std::vector<Widget*>& widgets) const = 0;
};
```

### Immediate Mode GUI Interface

```cpp
// Core IMGUI functions
namespace GUI {
    // Window management
    bool Begin(const std::string& name, bool* open = nullptr, WindowFlags flags = 0);
    void End();
    
    // Basic widgets
    bool Button(const std::string& label, const Vec2& size = Vec2(0, 0));
    bool Checkbox(const std::string& label, bool* value);
    bool RadioButton(const std::string& label, bool active);
    
    // Input widgets
    bool InputText(const std::string& label, std::string& text, InputTextFlags flags = 0);
    bool InputFloat(const std::string& label, f32* value, f32 step = 0.0f, f32 step_fast = 0.0f);
    bool InputInt(const std::string& label, i32* value, i32 step = 1, i32 step_fast = 100);
    bool InputFloat3(const std::string& label, f32 value[3]);
    
    // Sliders
    bool SliderFloat(const std::string& label, f32* value, f32 min, f32 max);
    bool SliderInt(const std::string& label, i32* value, i32 min, i32 max);
    bool SliderFloat3(const std::string& label, f32 value[3], f32 min, f32 max);
    
    // Selection widgets
    bool Combo(const std::string& label, i32* current_item, const std::vector<std::string>& items);
    bool ListBox(const std::string& label, i32* current_item, const std::vector<std::string>& items);
    
    // Display widgets
    void Text(const std::string& text);
    void TextColored(const Color& color, const std::string& text);
    void TextWrapped(const std::string& text);
    void Image(ITexture2D* texture, const Vec2& size, const Vec2& uv0 = Vec2(0,0), const Vec2& uv1 = Vec2(1,1));
    
    // Layout and spacing
    void SameLine(f32 offset_from_start_x = 0.0f, f32 spacing = -1.0f);
    void Spacing();
    void Separator();
    void NewLine();
    void Indent(f32 indent_w = 0.0f);
    void Unindent(f32 indent_w = 0.0f);
    
    // Containers
    bool CollapsingHeader(const std::string& label, TreeNodeFlags flags = 0);
    bool TreeNode(const std::string& label);
    void TreePop();
    bool BeginTabBar(const std::string& str_id, TabBarFlags flags = 0);
    void EndTabBar();
    bool BeginTabItem(const std::string& label, bool* open = nullptr, TabItemFlags flags = 0);
    void EndTabItem();
    
    // Menus
    bool BeginMainMenuBar();
    void EndMainMenuBar();
    bool BeginMenuBar();
    void EndMenuBar();
    bool BeginMenu(const std::string& label, bool enabled = true);
    void EndMenu();
    bool MenuItem(const std::string& label, const std::string& shortcut = "", bool* selected = nullptr, bool enabled = true);
    
    // Popups and modals
    void OpenPopup(const std::string& str_id);
    bool BeginPopup(const std::string& str_id, WindowFlags flags = 0);
    bool BeginPopupModal(const std::string& name, bool* open = nullptr, WindowFlags flags = 0);
    void EndPopup();
    void CloseCurrentPopup();
}
```

### Rendering System

```cpp
// GUI-specific rendering interface
class GUIRenderer {
public:
    virtual ~GUIRenderer() = default;
    
    // Rendering commands
    virtual void DrawRect(const Vec2& min, const Vec2& max, const Color& color) = 0;
    virtual void DrawRectFilled(const Vec2& min, const Vec2& max, const Color& color) = 0;
    virtual void DrawText(const Vec2& pos, const Color& color, const std::string& text) = 0;
    virtual void DrawImage(const Vec2& min, const Vec2& max, ITexture2D* texture, const Vec2& uv0, const Vec2& uv1) = 0;
    virtual void DrawLine(const Vec2& p1, const Vec2& p2, const Color& color, f32 thickness = 1.0f) = 0;
    virtual void DrawCircle(const Vec2& center, f32 radius, const Color& color, i32 segments = 0) = 0;
    
    // Clipping
    virtual void PushClipRect(const Vec2& min, const Vec2& max) = 0;
    virtual void PopClipRect() = 0;
    
    // State management
    virtual void SetViewport(const Vec2& size) = 0;
    virtual void Clear() = 0;
    virtual void Present() = 0;
};

// OpenGL implementation
class OpenGLGUIRenderer : public GUIRenderer {
private:
    struct GUIVertex {
        Vec2 position;
        Vec2 texCoord;
        Color color;
    };
    
    std::shared_ptr<IShader> m_shader;
    std::shared_ptr<IVertexBuffer> m_vertexBuffer;
    std::shared_ptr<IIndexBuffer> m_indexBuffer;
    std::shared_ptr<IVertexArray> m_vertexArray;
    
    std::vector<GUIVertex> m_vertices;
    std::vector<u32> m_indices;
    
    void FlushDrawData();
    void SetupRenderState();
};
```

### Font and Text Rendering

```cpp
// Font management system
struct FontGlyph {
    u32 codepoint;
    f32 advanceX;
    f32 x0, y0, x1, y1;  // Glyph bounds
    f32 u0, v0, u1, v1;  // Texture coordinates
};

class Font {
public:
    bool LoadFromFile(const std::string& filepath, f32 size);
    bool LoadFromMemory(const void* data, size_t size, f32 size);
    
    const FontGlyph* GetGlyph(u32 codepoint) const;
    f32 GetStringWidth(const std::string& text) const;
    f32 GetLineHeight() const;
    ITexture2D* GetTexture() const;
    
private:
    std::unordered_map<u32, FontGlyph> m_glyphs;
    std::shared_ptr<ITexture2D> m_texture;
    f32 m_size;
    f32 m_lineHeight;
};

class FontManager {
public:
    bool LoadFont(const std::string& name, const std::string& filepath, f32 size);
    Font* GetFont(const std::string& name);
    void SetDefaultFont(const std::string& name);
    Font* GetDefaultFont();
    
private:
    std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;
    Font* m_defaultFont = nullptr;
};
```

### Styling and Theming

```cpp
// GUI styling system
struct GUIStyle {
    // Colors
    Color windowBg = Color(0.06f, 0.06f, 0.06f, 0.94f);
    Color childBg = Color(0.00f, 0.00f, 0.00f, 0.00f);
    Color popupBg = Color(0.08f, 0.08f, 0.08f, 0.94f);
    Color border = Color(0.43f, 0.43f, 0.50f, 0.50f);
    Color frameBg = Color(0.16f, 0.29f, 0.48f, 0.54f);
    Color frameBgHovered = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Color frameBgActive = Color(0.26f, 0.59f, 0.98f, 0.67f);
    Color titleBg = Color(0.04f, 0.04f, 0.04f, 1.00f);
    Color titleBgActive = Color(0.16f, 0.29f, 0.48f, 1.00f);
    Color titleBgCollapsed = Color(0.00f, 0.00f, 0.00f, 0.51f);
    Color button = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Color buttonHovered = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Color buttonActive = Color(0.06f, 0.53f, 0.98f, 1.00f);
    Color text = Color(1.00f, 1.00f, 1.00f, 1.00f);
    Color textDisabled = Color(0.50f, 0.50f, 0.50f, 1.00f);
    
    // Sizes
    f32 windowPadding = 8.0f;
    f32 windowRounding = 0.0f;
    f32 windowBorderSize = 1.0f;
    f32 childRounding = 0.0f;
    f32 childBorderSize = 1.0f;
    f32 popupRounding = 0.0f;
    f32 popupBorderSize = 1.0f;
    f32 framePadding = 4.0f;
    f32 frameRounding = 0.0f;
    f32 frameBorderSize = 0.0f;
    f32 itemSpacing = 8.0f;
    f32 itemInnerSpacing = 4.0f;
    f32 indentSpacing = 21.0f;
    f32 scrollbarSize = 14.0f;
    f32 scrollbarRounding = 9.0f;
    f32 grabMinSize = 10.0f;
    f32 grabRounding = 0.0f;
    
    // Predefined themes
    static GUIStyle CreateDarkTheme();
    static GUIStyle CreateLightTheme();
    static GUIStyle CreateClassicTheme();
};
```

## Implementation Strategy

### Phase 1: Core Infrastructure (Week 1-2)
1. **Basic IMGUI Framework**
   - Implement core GUIContext and rendering pipeline
   - Basic widget rendering (buttons, text, rectangles)
   - Input event processing and state management

2. **Font and Text System**
   - Font loading using FreeType or stb_truetype
   - Glyph atlas generation and management
   - Basic text rendering with proper metrics

### Phase 2: Essential Widgets (Week 3-4)
1. **Input Widgets**
   - Text input fields with cursor and selection
   - Numeric input with validation
   - Checkboxes and radio buttons

2. **Display Widgets**
   - Text rendering with formatting
   - Image display with UV mapping
   - Progress bars and status indicators

### Phase 3: Advanced Widgets (Week 5-6)
1. **Container Widgets**
   - Windows with title bars and resizing
   - Collapsible sections and tree views
   - Tab controls and panels

2. **Selection Widgets**
   - Dropdown menus and combo boxes
   - List boxes with scrolling
   - Menu bars and context menus

### Phase 4: Polish and Integration (Week 7-8)
1. **Styling and Theming**
   - Complete styling system implementation
   - Multiple built-in themes
   - Custom theme creation tools

2. **Performance Optimization**
   - Efficient draw call batching
   - Texture atlas optimization
   - Memory usage minimization

## Integration with Engine

### Rendering Pipeline Integration
```cpp
// In main render loop
void Game::onRender() {
    // 3D scene rendering
    m_graphicsDevice->Clear(Color::Black);
    RenderScene();
    
    // GUI rendering (overlay)
    GUI::GUIContext::Get().BeginFrame();
    RenderGUI();  // User GUI code
    GUI::GUIContext::Get().EndFrame();
    GUI::GUIContext::Get().Render();
    
    m_graphicsDevice->Present(true);
}
```

### Input System Integration
```cpp
// In input processing
void Game::ProcessInput(const InputEvent& event) {
    // Let GUI handle input first
    GUI::GUIContext::Get().ProcessInput(event);
    
    // Only process game input if GUI doesn't want it
    if (!GUI::GUIContext::Get().WantCaptureKeyboard() && 
        !GUI::GUIContext::Get().WantCaptureMouse()) {
        ProcessGameInput(event);
    }
}
```

## Performance Considerations
1. **Efficient Rendering** - Batch draw calls and minimize state changes
2. **Memory Management** - Pool allocations and minimize garbage collection
3. **Input Latency** - Immediate response to user interactions
4. **Scalability** - Support for complex UIs with hundreds of widgets

## Testing Strategy
1. **Widget Functionality** - Comprehensive widget behavior testing
2. **Performance Testing** - Frame rate impact and memory usage
3. **Input Validation** - Edge cases and malformed input handling
4. **Visual Testing** - Rendering correctness across different themes
