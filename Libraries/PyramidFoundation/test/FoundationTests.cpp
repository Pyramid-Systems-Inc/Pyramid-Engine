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
    config.enableHistory = true;
    config.historyLevel = Util::LogLevel::Info;
    config.historyCapacity = 3;
    auto& logger = Util::Logger::GetInstance();
    logger.Configure(config);
    logger.ClearHistory();
    logger.Log(Util::LogLevel::Debug, "ignored debug entry");
    logger.Log(Util::LogLevel::Info, "first info entry");
    logger.Log(Util::LogLevel::Warn, "warning entry");
    logger.Log(Util::LogLevel::Error, "error entry");
    logger.Log(Util::LogLevel::Info, "latest info entry");

    const auto history = logger.GetRecentEntries();
    passed &= Expect(history.size() == 3,
        "logger history must remain bounded");
    passed &= Expect(history.size() == 3 &&
            history[0].message == "warning entry" &&
            history[1].message == "error entry" &&
            history[2].message == "latest info entry",
        "logger history must retain chronological newest entries");

    const auto recentWarnings = logger.GetRecentEntries(2, Util::LogLevel::Warn);
    passed &= Expect(recentWarnings.size() == 2 &&
            recentWarnings[0].message == "warning entry" &&
            recentWarnings[1].message == "error entry",
        "filtered recent history must return the newest matching entries chronologically");

    logger.SetHistoryCapacity(1);
    const auto trimmed = logger.GetRecentEntries();
    passed &= Expect(trimmed.size() == 1 &&
            trimmed.front().message == "latest info entry",
        "reducing history capacity must discard the oldest entries");
    logger.ClearHistory();
    passed &= Expect(logger.GetRecentEntries().empty(),
        "clearing logger history must remove every retained entry");
    logger.Flush();

    return passed ? 0 : 1;
}
