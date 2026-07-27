#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Input/InputActions.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Platform/Input.hpp>
#include <Pyramid/Util/Log.hpp>

#include <iostream>

int main()
{
    Pyramid::Util::LoggerConfig logging;
    logging.enableConsole = false;
    logging.enableFile = false;
    Pyramid::Util::Logger::GetInstance().Configure(logging);

    Pyramid::InputState input;
    input.SetFocused(true);
    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::W, true);

    Pyramid::InputActionSystem actions;
    Pyramid::InputContext* context = actions.CreateContext("consumer");
    const bool configured = context &&
        context->AddAction("Move", Pyramid::InputActionType::Axis1D) &&
        context->AddBinding("Move", Pyramid::InputBinding::KeyBinding(Pyramid::Key::W));
    actions.Update(input);

    const Pyramid::Math::Vec3 vector(3.0f, 4.0f, 0.0f);
    const bool valid = configured &&
        Pyramid::Color::White == Pyramid::Color(1.0f, 1.0f, 1.0f, 1.0f) &&
        vector.LengthSquared() == 25.0f &&
        actions.IsActionDown("consumer", "Move");

    std::cout << "Pyramid foundation, math, and input packages are operational\n";
    return valid ? 0 : 1;
}
