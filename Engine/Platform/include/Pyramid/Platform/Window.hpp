#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Platform/Input.hpp>
#include <Pyramid/Platform/Clipboard.hpp>

#include <functional>
#include <utility>

namespace Pyramid
{
    /**
     * @brief Native window state reported with a resize event.
     */
    enum class WindowResizeState : u8
    {
        Restored,
        Minimized,
        Maximized
    };

    /**
     * @brief Platform-neutral client-area resize notification.
     *
     * A minimized Win32 window reports a zero-sized client area. Rendering code
     * should defer viewport and render-target work until a later non-zero event.
     */
    struct WindowResizeEvent
    {
        int width = 0;
        int height = 0;
        WindowResizeState state = WindowResizeState::Restored;

        [[nodiscard]] bool HasRenderableArea() const
        {
            return width > 0 && height > 0 && state != WindowResizeState::Minimized;
        }
    };

    /**
     * @brief Platform-neutral window and graphics-context interface.
     */
    class Window
    {
    public:
        using ResizeCallback = std::function<void(const WindowResizeEvent&)>;

        virtual ~Window() = default;

        virtual bool Initialize(const char* title = "Pyramid Game", int width = 800, int height = 600) = 0;
        virtual void Present(bool vsync) = 0;
        virtual void MakeContextCurrent() = 0;
        virtual bool ProcessMessages() = 0;

        virtual void* GetHandle() const = 0;
        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;

        virtual bool ShouldClose() const = 0;
        virtual void SetTitle(const char* title) = 0;
        virtual void SetSize(int width, int height) = 0;
        virtual void SetPosition(int x, int y) = 0;
        virtual void SetVisible(bool visible) = 0;
        virtual bool IsMinimized() const = 0;
        virtual bool IsMaximized() const = 0;

        /**
         * @brief Current per-frame keyboard and mouse state for this window.
         */
        [[nodiscard]] virtual const InputState& GetInputState() const = 0;

        /** Unicode clipboard service owned by the native window backend. */
        [[nodiscard]] virtual Clipboard& GetClipboard() = 0;

        /**
         * @brief Replaces the callback invoked for client-area size changes.
         *
         * The callback runs on the window thread while ProcessMessages() is
         * dispatching native messages. Pass an empty callback to detach it.
         */
        void SetResizeCallback(ResizeCallback callback)
        {
            m_resizeCallback = std::move(callback);
        }

    protected:
        void DispatchResizeEvent(const WindowResizeEvent& event)
        {
            // Copy before invocation so a callback may safely replace or detach
            // itself while handling the current event.
            const ResizeCallback callback = m_resizeCallback;
            if (callback)
            {
                callback(event);
            }
        }

    private:
        ResizeCallback m_resizeCallback;
    };
} // namespace Pyramid
