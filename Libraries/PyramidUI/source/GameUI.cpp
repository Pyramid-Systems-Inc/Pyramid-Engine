#include <Pyramid/UI/GameUI.hpp>

#include <algorithm>
#include <cmath>

namespace Pyramid::UI
{
    namespace
    {
        f32 NonNegative(f32 value)
        {
            return std::isfinite(value) ? (std::max)(0.0f, value) : 0.0f;
        }

        Rect ApplyMargin(const Rect& rect, const Insets& margin)
        {
            const f32 left = NonNegative(margin.left);
            const f32 top = NonNegative(margin.top);
            const f32 right = NonNegative(margin.right);
            const f32 bottom = NonNegative(margin.bottom);
            return {
                rect.x + left,
                rect.y + top,
                (std::max)(0.0f, rect.width - left - right),
                (std::max)(0.0f, rect.height - top - bottom)};
        }
    } // namespace

    Rect ResolveAnchoredRect(
        const Rect& parent,
        const Math::Vec2& requestedSize,
        Anchor anchor,
        const Insets& margin)
    {
        const Rect content = ApplyMargin(parent, margin);
        if (!content.IsValid())
        {
            return {};
        }
        if (anchor == Anchor::Stretch)
        {
            return content;
        }

        const f32 width = (std::min)(content.width, NonNegative(requestedSize.x));
        const f32 height = (std::min)(content.height, NonNegative(requestedSize.y));
        if (width <= 0.0f || height <= 0.0f)
        {
            return {};
        }

        f32 x = content.x;
        f32 y = content.y;
        switch (anchor)
        {
            case Anchor::TopCenter:
            case Anchor::Center:
            case Anchor::BottomCenter:
                x += (content.width - width) * 0.5f;
                break;
            case Anchor::TopRight:
            case Anchor::CenterRight:
            case Anchor::BottomRight:
                x += content.width - width;
                break;
            default:
                break;
        }

        switch (anchor)
        {
            case Anchor::CenterLeft:
            case Anchor::Center:
            case Anchor::CenterRight:
                y += (content.height - height) * 0.5f;
                break;
            case Anchor::BottomLeft:
            case Anchor::BottomCenter:
            case Anchor::BottomRight:
                y += content.height - height;
                break;
            default:
                break;
        }
        return {x, y, width, height};
    }

    Rect ResolveDockedRect(
        Rect& remaining,
        Dock dock,
        f32 requestedExtent,
        const Insets& margin)
    {
        if (!remaining.IsValid() || dock == Dock::None)
        {
            return {};
        }

        const f32 extent = NonNegative(requestedExtent);
        Rect allocated;
        switch (dock)
        {
            case Dock::Top:
            {
                const f32 height = (std::min)(remaining.height, extent);
                allocated = {remaining.x, remaining.y, remaining.width, height};
                remaining.y += height;
                remaining.height -= height;
                break;
            }
            case Dock::Bottom:
            {
                const f32 height = (std::min)(remaining.height, extent);
                allocated = {
                    remaining.x,
                    remaining.y + remaining.height - height,
                    remaining.width,
                    height};
                remaining.height -= height;
                break;
            }
            case Dock::Left:
            {
                const f32 width = (std::min)(remaining.width, extent);
                allocated = {remaining.x, remaining.y, width, remaining.height};
                remaining.x += width;
                remaining.width -= width;
                break;
            }
            case Dock::Right:
            {
                const f32 width = (std::min)(remaining.width, extent);
                allocated = {
                    remaining.x + remaining.width - width,
                    remaining.y,
                    width,
                    remaining.height};
                remaining.width -= width;
                break;
            }
            case Dock::Fill:
                allocated = remaining;
                remaining = {};
                break;
            case Dock::None:
                break;
        }
        return ApplyMargin(allocated, margin);
    }

    bool ScreenTransition::IsValid() const
    {
        if (type == ScreenTransitionType::None)
        {
            return duration == 0.0f;
        }
        return duration > 0.0f && std::isfinite(duration);
    }

    ScreenStack::~ScreenStack()
    {
        Clear();
    }

    bool ScreenStack::Push(
        std::shared_ptr<Screen> screen,
        const ScreenTransition& transition)
    {
        return QueueOrApply({OperationType::Push, std::move(screen), transition});
    }

    bool ScreenStack::Replace(
        std::shared_ptr<Screen> screen,
        const ScreenTransition& transition)
    {
        return QueueOrApply({OperationType::Replace, std::move(screen), transition});
    }

    bool ScreenStack::Pop(const ScreenTransition& transition)
    {
        return QueueOrApply({OperationType::Pop, {}, transition});
    }

    void ScreenStack::Clear()
    {
        (void)QueueOrApply({OperationType::Clear, {}, {}});
    }

    void ScreenStack::Update(f32 deltaTime)
    {
        const f32 safeDelta = std::isfinite(deltaTime) ? (std::max)(0.0f, deltaTime) : 0.0f;
        if (m_transitionActive)
        {
            m_transitionElapsed += safeDelta;
            if (m_transitionElapsed >= m_transition.duration)
            {
                m_transitionElapsed = m_transition.duration;
                m_transitionActive = false;
            }
        }

        if (m_screens.empty())
        {
            ApplyPending();
            return;
        }

        const std::size_t first = FirstVisibleScreen();
        m_dispatching = true;
        for (std::size_t index = first; index < m_screens.size(); ++index)
        {
            if (m_screens[index])
            {
                m_screens[index]->Update(safeDelta);
            }
        }
        m_dispatching = false;
        ApplyPending();
    }

    void ScreenStack::Build(Context& context)
    {
        if (m_screens.empty())
        {
            ApplyPending();
            return;
        }

        const std::size_t first = FirstVisibleScreen();
        m_dispatching = true;
        for (std::size_t index = first; index < m_screens.size(); ++index)
        {
            if (m_screens[index])
            {
                m_screens[index]->Build(context);
            }
        }
        m_dispatching = false;
        ApplyPending();
    }

    std::shared_ptr<Screen> ScreenStack::GetTop() const
    {
        return m_screens.empty() ? std::shared_ptr<Screen>{} : m_screens.back();
    }

    std::string_view ScreenStack::GetTopName() const
    {
        const auto top = GetTop();
        return top ? top->GetName() : std::string_view{};
    }

    bool ScreenStack::BlocksGameplayInput() const
    {
        const auto top = GetTop();
        if (!top)
        {
            return false;
        }
        const ScreenPresentation presentation = top->GetPresentation();
        return presentation == ScreenPresentation::Opaque ||
            presentation == ScreenPresentation::Modal;
    }

    f32 ScreenStack::GetTransitionProgress() const
    {
        if (!m_transitionActive || m_transition.duration <= 0.0f)
        {
            return 1.0f;
        }
        return (std::max)(0.0f, (std::min)(1.0f, m_transitionElapsed / m_transition.duration));
    }

    bool ScreenStack::QueueOrApply(PendingOperation operation)
    {
        if (!operation.transition.IsValid())
        {
            return false;
        }
        if ((operation.type == OperationType::Push ||
             operation.type == OperationType::Replace) && !operation.screen)
        {
            return false;
        }

        if (m_dispatching)
        {
            m_pending.push_back(std::move(operation));
            return true;
        }

        m_dispatching = true;
        const bool result = Apply(operation);
        m_dispatching = false;
        ApplyPending();
        return result;
    }

    bool ScreenStack::Apply(const PendingOperation& operation)
    {
        switch (operation.type)
        {
            case OperationType::Push:
                m_screens.push_back(operation.screen);
                operation.screen->OnEnter();
                BeginTransition(operation.transition);
                return true;

            case OperationType::Replace:
                if (!m_screens.empty())
                {
                    m_screens.back()->OnExit();
                    m_screens.pop_back();
                }
                m_screens.push_back(operation.screen);
                operation.screen->OnEnter();
                BeginTransition(operation.transition);
                return true;

            case OperationType::Pop:
                if (m_screens.empty())
                {
                    return false;
                }
                m_screens.back()->OnExit();
                m_screens.pop_back();
                BeginTransition(operation.transition);
                return true;

            case OperationType::Clear:
                for (auto iterator = m_screens.rbegin(); iterator != m_screens.rend(); ++iterator)
                {
                    if (*iterator)
                    {
                        (*iterator)->OnExit();
                    }
                }
                m_screens.clear();
                m_transitionActive = false;
                m_transitionElapsed = 0.0f;
                return true;
        }
        return false;
    }

    void ScreenStack::ApplyPending()
    {
        if (m_dispatching || m_pending.empty())
        {
            return;
        }
        while (!m_pending.empty())
        {
            std::vector<PendingOperation> pending;
            pending.swap(m_pending);
            m_dispatching = true;
            for (const auto& operation : pending)
            {
                (void)Apply(operation);
            }
            m_dispatching = false;
        }
    }

    void ScreenStack::BeginTransition(const ScreenTransition& transition)
    {
        m_transition = transition;
        m_transitionElapsed = 0.0f;
        m_transitionActive = transition.type != ScreenTransitionType::None &&
            transition.duration > 0.0f;
    }

    std::size_t ScreenStack::FirstVisibleScreen() const
    {
        if (m_screens.empty())
        {
            return 0;
        }

        for (std::size_t reverse = m_screens.size(); reverse > 0; --reverse)
        {
            const std::size_t index = reverse - 1;
            if (!m_screens[index])
            {
                continue;
            }
            const ScreenPresentation presentation = m_screens[index]->GetPresentation();
            if (presentation == ScreenPresentation::Opaque)
            {
                return index;
            }
        }
        return 0;
    }
} // namespace Pyramid::UI
