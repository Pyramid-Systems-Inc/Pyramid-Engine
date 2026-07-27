#pragma once

#include <Pyramid/UI/UI.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Pyramid::UI
{
    enum class Anchor : u8
    {
        TopLeft = 0,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        Stretch
    };

    enum class Dock : u8
    {
        None = 0,
        Top,
        Bottom,
        Left,
        Right,
        Fill
    };

    [[nodiscard]] Rect ResolveAnchoredRect(
        const Rect& parent,
        const Math::Vec2& size,
        Anchor anchor,
        const Insets& margin = {});

    /**
     * Resolve one docked region and shrink the caller-owned remaining rectangle.
     * Fill consumes the complete remaining rectangle.
     */
    [[nodiscard]] Rect ResolveDockedRect(
        Rect& remaining,
        Dock dock,
        f32 extent,
        const Insets& margin = {});

    template<typename... Arguments>
    class Signal final
    {
    public:
        using Connection = u64;
        using Callback = std::function<void(Arguments...)>;

        [[nodiscard]] Connection Connect(Callback callback)
        {
            if (!callback)
            {
                return 0;
            }
            const Connection id = ++m_nextConnection;
            m_callbacks.emplace(id, std::move(callback));
            return id;
        }

        bool Disconnect(Connection connection)
        {
            return connection != 0 && m_callbacks.erase(connection) != 0;
        }

        void Clear()
        {
            m_callbacks.clear();
        }

        [[nodiscard]] bool Empty() const
        {
            return m_callbacks.empty();
        }

        void Emit(Arguments... arguments)
        {
            std::vector<Connection> snapshot;
            snapshot.reserve(m_callbacks.size());
            for (const auto& entry : m_callbacks)
            {
                snapshot.push_back(entry.first);
            }
            std::sort(snapshot.begin(), snapshot.end());

            for (const Connection connection : snapshot)
            {
                const auto found = m_callbacks.find(connection);
                if (found != m_callbacks.end())
                {
                    found->second(arguments...);
                }
            }
        }

    private:
        std::unordered_map<Connection, Callback> m_callbacks;
        Connection m_nextConnection = 0;
    };

    enum class ScreenPresentation : u8
    {
        Opaque = 0,
        Transparent,
        Modal
    };

    enum class ScreenTransitionType : u8
    {
        None = 0,
        Fade
    };

    struct ScreenTransition
    {
        ScreenTransitionType type = ScreenTransitionType::None;
        f32 duration = 0.0f;

        [[nodiscard]] bool IsValid() const;
    };

    class Screen
    {
    public:
        virtual ~Screen() = default;

        [[nodiscard]] virtual std::string_view GetName() const = 0;
        [[nodiscard]] virtual ScreenPresentation GetPresentation() const
        {
            return ScreenPresentation::Transparent;
        }

        virtual void OnEnter() {}
        virtual void OnExit() {}
        virtual void Update(f32 deltaTime) { (void)deltaTime; }
        virtual void Build(Context& context) = 0;
    };

    class ScreenStack final
    {
    public:
        ~ScreenStack();

        bool Push(
            std::shared_ptr<Screen> screen,
            const ScreenTransition& transition = {});
        bool Replace(
            std::shared_ptr<Screen> screen,
            const ScreenTransition& transition = {});
        bool Pop(const ScreenTransition& transition = {});
        void Clear();

        void Update(f32 deltaTime);
        void Build(Context& context);

        [[nodiscard]] std::size_t GetCount() const { return m_screens.size(); }
        [[nodiscard]] bool Empty() const { return m_screens.empty(); }
        [[nodiscard]] std::shared_ptr<Screen> GetTop() const;
        [[nodiscard]] std::string_view GetTopName() const;
        [[nodiscard]] bool BlocksGameplayInput() const;
        [[nodiscard]] bool IsTransitioning() const { return m_transitionActive; }
        [[nodiscard]] f32 GetTransitionProgress() const;

    private:
        enum class OperationType : u8
        {
            Push,
            Replace,
            Pop,
            Clear
        };

        struct PendingOperation
        {
            OperationType type = OperationType::Push;
            std::shared_ptr<Screen> screen;
            ScreenTransition transition;
        };

        bool QueueOrApply(PendingOperation operation);
        bool Apply(const PendingOperation& operation);
        void ApplyPending();
        void BeginTransition(const ScreenTransition& transition);
        [[nodiscard]] std::size_t FirstVisibleScreen() const;

        std::vector<std::shared_ptr<Screen>> m_screens;
        std::vector<PendingOperation> m_pending;
        ScreenTransition m_transition;
        f32 m_transitionElapsed = 0.0f;
        bool m_transitionActive = false;
        bool m_dispatching = false;
    };
} // namespace Pyramid::UI
