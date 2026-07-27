#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Util/Log.hpp>

#include <iostream>
#include <type_traits>

namespace
{
    bool Expect(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "Foundation test failed: " << message << '\n';
        }
        return condition;
    }
}

int main()
{
    using namespace Pyramid;

    bool passed = true;
    passed &= Expect(sizeof(u8) == 1 && sizeof(u16) == 2 && sizeof(u32) == 4 && sizeof(u64) == 8,
        "fixed-width unsigned aliases must preserve their widths");
    passed &= Expect(std::is_same_v<f32, float> && std::is_same_v<f64, double>,
        "floating-point aliases must preserve the public ABI");
    passed &= Expect(Color::White == Color(1.0f, 1.0f, 1.0f, 1.0f),
        "owned color constants must remain linked through PyramidFoundation");
    passed &= Expect(Color::Clear != Color::Black,
        "color comparison must include alpha");
    passed &= Expect(Util::StringToLogLevel("warning") == Util::LogLevel::Info,
        "unknown log levels must retain the documented Info fallback");
    passed &= Expect(Util::StringToLogLevel("CrItIcAl") == Util::LogLevel::Critical,
        "log-level parsing must remain case-insensitive");
    passed &= Expect(std::string(Util::LogLevelToString(Util::LogLevel::Warn)) == "WARN",
        "log-level formatting must remain stable");

    Util::LoggerConfig config;
    config.enableConsole = false;
    config.enableFile = false;
    Util::Logger::GetInstance().Configure(config);
    Util::Logger::GetInstance().Log(Util::LogLevel::Info, "foundation smoke test");
    Util::Logger::GetInstance().Flush();

    return passed ? 0 : 1;
}
