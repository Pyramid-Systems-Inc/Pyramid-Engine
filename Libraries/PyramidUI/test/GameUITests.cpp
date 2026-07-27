#include <Pyramid/UI/GameUI.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "Game UI test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    class TestScreen final : public Pyramid::UI::Screen
    {
    public:
        TestScreen(
            std::string name,
            Pyramid::UI::ScreenPresentation presentation,
            std::vector<std::string>& trace)
            : m_name(std::move(name))
            , m_presentation(presentation)
            , m_trace(trace)
        {
        }

        std::string_view GetName() const override { return m_name; }
        Pyramid::UI::ScreenPresentation GetPresentation() const override
        {
            return m_presentation;
        }

        void OnEnter() override { m_trace.push_back(m_name + ":enter"); }
        void OnExit() override { m_trace.push_back(m_name + ":exit"); }
        void Update(float) override { m_trace.push_back(m_name + ":update"); }
        void Build(Pyramid::UI::Context& context) override
        {
            m_trace.push_back(m_name + ":build");
            if (m_presentation == Pyramid::UI::ScreenPresentation::Modal)
            {
                context.Overlay(
                    m_name + "-overlay",
                    Pyramid::Color(0.0f, 0.0f, 0.0f, 0.6f),
                    true);
            }
            if (onBuild)
            {
                onBuild();
            }
        }

        std::function<void()> onBuild;

    private:
        std::string m_name;
        Pyramid::UI::ScreenPresentation m_presentation;
        std::vector<std::string>& m_trace;
    };
}

int main()
{
    using namespace Pyramid;
    using namespace Pyramid::UI;

    Rect parent{10.0f, 20.0f, 800.0f, 600.0f};
    const Rect centered = ResolveAnchoredRect(
        parent,
        Math::Vec2(200.0f, 100.0f),
        Anchor::Center,
        Insets{10.0f, 10.0f, 10.0f, 10.0f});
    Require(centered == Rect{310.0f, 270.0f, 200.0f, 100.0f},
        "center anchor produced the wrong rectangle");

    Rect remaining{0.0f, 0.0f, 640.0f, 480.0f};
    const Rect top = ResolveDockedRect(remaining, Dock::Top, 80.0f);
    const Rect right = ResolveDockedRect(remaining, Dock::Right, 120.0f);
    const Rect fill = ResolveDockedRect(remaining, Dock::Fill, 0.0f);
    Require(top == Rect{0.0f, 0.0f, 640.0f, 80.0f}, "top docking failed");
    Require(right == Rect{520.0f, 80.0f, 120.0f, 400.0f}, "right docking failed");
    Require(fill == Rect{0.0f, 80.0f, 520.0f, 400.0f}, "fill docking failed");
    Require(!remaining.IsValid(), "fill docking did not consume the remaining space");

    Signal<int> signal;
    int signalValue = 0;
    Signal<int>::Connection second = 0;
    const auto first = signal.Connect(
        [&](int value)
        {
            signalValue += value;
            (void)signal.Disconnect(second);
            (void)signal.Connect([&](int replacement) { signalValue += replacement * 100; });
        });
    second = signal.Connect([&](int value) { signalValue += value * 10; });
    Require(first != 0 && second != 0, "signal connection failed");
    signal.Emit(2);
    Require(signalValue == 2,
        "signal did not safely defer newly connected callbacks or honor disconnection");
    signal.Emit(1);
    Require(signalValue == 103, "signal snapshot behavior changed between emissions");

    DrawList nineSlice;
    nineSlice.AddNineSlice(
        {0.0f, 0.0f, 100.0f, 80.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {8.0f, 8.0f, 8.0f, 8.0f},
        {0.1f, 0.1f, 0.1f, 0.1f},
        Color::White,
        9,
        {0.0f, 0.0f, 200.0f, 200.0f});
    Require(nineSlice.GetVertices().size() == 36, "nine-slice vertex count is wrong");
    Require(nineSlice.GetIndices().size() == 54, "nine-slice index count is wrong");
    Require(nineSlice.GetBatches().size() == 1, "nine-slice did not batch compatible quads");

    std::vector<std::string> trace;
    ScreenStack stack;
    auto menu = std::make_shared<TestScreen>(
        "menu",
        ScreenPresentation::Opaque,
        trace);
    auto hud = std::make_shared<TestScreen>(
        "hud",
        ScreenPresentation::Transparent,
        trace);
    auto pause = std::make_shared<TestScreen>(
        "pause",
        ScreenPresentation::Modal,
        trace);

    Require(stack.Push(menu), "opaque screen push failed");
    Require(stack.BlocksGameplayInput(), "opaque screen did not block gameplay");
    Require(stack.Push(hud, {ScreenTransitionType::Fade, 0.25f}),
        "transparent screen push failed");
    Require(stack.IsTransitioning(), "screen transition did not start");
    stack.Update(0.125f);
    Require(stack.GetTransitionProgress() > 0.49f && stack.GetTransitionProgress() < 0.51f,
        "transition progress is not delta-time based");
    stack.Update(0.125f);
    Require(!stack.IsTransitioning(), "screen transition did not complete");
    Require(!stack.BlocksGameplayInput(), "transparent HUD blocked gameplay");

    InputState input;
    input.SetFocused(true);
    input.ProcessMouseMove(5.0f, 5.0f);
    Context context;
    Require(context.BeginFrame({640.0f, 480.0f, 1.0f, 1.0f / 60.0f}, input),
        "screen build frame rejected");
    stack.Build(context);
    (void)context.EndFrame();
    Require(trace.end() != std::find(trace.begin(), trace.end(), "menu:build"),
        "opaque base screen was not built below transparent HUD");
    Require(trace.end() != std::find(trace.begin(), trace.end(), "hud:build"),
        "transparent HUD was not built");

    Require(stack.Push(pause), "modal screen push failed");
    Require(stack.BlocksGameplayInput(), "modal did not block gameplay");
    Require(context.BeginFrame({640.0f, 480.0f, 1.0f, 1.0f / 60.0f}, input),
        "modal frame rejected");
    stack.Build(context);
    (void)context.EndFrame();
    Require(context.PrepareInput(input).HasAnyMouseConsumption(),
        "modal overlay did not reserve pointer input");

    auto replacement = std::make_shared<TestScreen>(
        "replacement",
        ScreenPresentation::Opaque,
        trace);
    pause->onBuild = [&]() { (void)stack.Replace(replacement); };
    Require(context.BeginFrame({640.0f, 480.0f, 1.0f, 1.0f / 60.0f}, input),
        "deferred replacement frame rejected");
    stack.Build(context);
    (void)context.EndFrame();
    Require(stack.GetTopName() == "replacement",
        "screen replacement requested during Build was not deferred safely");
    Require(trace.end() != std::find(trace.begin(), trace.end(), "pause:exit"),
        "replaced screen did not receive OnExit");
    Require(trace.end() != std::find(trace.begin(), trace.end(), "replacement:enter"),
        "replacement screen did not receive OnEnter");

    stack.Clear();
    Require(stack.Empty(), "screen clear retained entries");

    std::cout << "Game UI tests passed\n";
    return EXIT_SUCCESS;
}
