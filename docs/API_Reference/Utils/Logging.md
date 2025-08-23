# Logging System API Reference

## Overview

The Pyramid Engine features a production-ready, thread-safe logging system with multiple output formats, configurable log levels, and automatic file rotation. The system provides both stream-style and printf-style logging interfaces for maximum flexibility.

## Key Features

- **Thread-Safe**: Mutex-protected logging operations for multi-threaded applications
- **Multiple Log Levels**: Trace, Debug, Info, Warn, Error, Critical
- **Dual Output**: Simultaneous console and file logging with independent level control
- **File Rotation**: Automatic log file rotation with size limits and file count management
- **Source Location**: Optional file, function, and line number tracking
- **Structured Logging**: Key-value pair logging for analytics and debugging
- **Performance Optimized**: Early exit filtering and efficient string handling

## Log Levels

```cpp
enum class LogLevel {
    Trace = 0,    // Detailed execution flow
    Debug = 1,    // Development debugging information
    Info = 2,     // General information
    Warn = 3,     // Warning conditions
    Error = 4,    // Error conditions
    Critical = 5, // Critical failures
    None = 6      // Disable logging
};
```

## Configuration

### LoggerConfig Structure

```cpp
struct LoggerConfig {
    bool enableConsole = true;
    bool enableFile = true;
    LogLevel consoleLevel = LogLevel::Info;
    LogLevel fileLevel = LogLevel::Debug;
    std::string logFilePath = "pyramid.log";
    size_t maxFileSize = 10 * 1024 * 1024; // 10MB
    size_t maxFiles = 5;
    bool enableTimestamp = true;
    bool enableSourceLocation = false;
    bool enableThreadId = false;
};
```

### Configuring the Logger

```cpp
#include <Pyramid/Util/Log.hpp>

// Configure with custom settings
Pyramid::Util::LoggerConfig config;
config.enableConsole = true;
config.enableFile = true;
config.consoleLevel = Pyramid::Util::LogLevel::Info;
config.fileLevel = Pyramid::Util::LogLevel::Debug;
config.logFilePath = "game.log";
config.maxFileSize = 20 * 1024 * 1024; // 20MB
config.maxFiles = 10;
config.enableTimestamp = true;
config.enableSourceLocation = true;
config.enableThreadId = false;

PYRAMID_CONFIGURE_LOGGER(config);
```

## Logging Macros

### Basic Logging Macros

```cpp
// Multi-argument logging (variadic templates)
PYRAMID_LOG_TRACE("Trace message: ", value1, ", ", value2);
PYRAMID_LOG_DEBUG("Debug info: ", debugValue);
PYRAMID_LOG_INFO("Application started with version: ", version);
PYRAMID_LOG_WARN("Warning: low memory: ", availableMemory, " MB");
PYRAMID_LOG_ERROR("Error loading file: ", filename);
PYRAMID_LOG_CRITICAL("Critical system failure: ", errorCode);

// Core system logging (internal engine use)
PYRAMID_CORE_TRACE("Engine trace message");
PYRAMID_CORE_DEBUG("Engine debug info");
PYRAMID_CORE_INFO("Engine initialized");
PYRAMID_CORE_WARN("Engine warning");
PYRAMID_CORE_ERROR("Engine error");
PYRAMID_CORE_CRITICAL("Engine critical error");
```

### Stream-Style Logging

```cpp
// Stream-style interface for complex formatting
PYRAMID_TRACE_STREAM() << "Player position: " << player.x << ", " << player.y;
PYRAMID_DEBUG_STREAM() << "Debug value: " << std::hex << debugValue;
PYRAMID_INFO_STREAM() << "Game state: " << gameState.ToString();
PYRAMID_WARN_STREAM() << "Performance warning: FPS = " << fps;
PYRAMID_ERROR_STREAM() << "Error in " << __FUNCTION__ << ": " << errorMessage;
PYRAMID_CRITICAL_STREAM() << "Critical failure in system: " << systemName;
```

### Structured Logging

```cpp
// Structured logging with key-value pairs
std::unordered_map<std::string, std::string> fields;
fields["user_id"] = "12345";
fields["level_name"] = "forest_level_1";
fields["score"] = std::to_string(playerScore);
fields["time_elapsed"] = std::to_string(gameTime);

PYRAMID_LOG_STRUCTURED(Pyramid::Util::LogLevel::Info, "Level completed", fields);

// Analytics logging
fields.clear();
fields["event_type"] = "item_collected";
fields["item_id"] = "health_potion";
fields["player_level"] = std::to_string(playerLevel);
fields["position_x"] = std::to_string(player.position.x);
fields["position_y"] = std::to_string(player.position.y);

PYRAMID_LOG_STRUCTURED(Pyramid::Util::LogLevel::Info, "Item collected", fields);
```

### Assertion Macros

```cpp
// Enhanced assertions with logging
PYRAMID_ASSERT(condition, "Assertion message with ", variable);
PYRAMID_CORE_ASSERT(condition, "Core assertion message");

// Conditional assertions (only in debug builds)
#ifdef PYRAMID_DEBUG
    PYRAMID_DEBUG_ASSERT(condition, "Debug-only assertion");
#endif
```

## Usage Examples

### Game Application Logging

```cpp
#include <Pyramid/Util/Log.hpp>

class MyGame : public Pyramid::Game {
public:
    void onCreate() override {
        // Configure logging for the game
        Pyramid::Util::LoggerConfig config;
        config.enableConsole = true;
        config.enableFile = true;
        config.consoleLevel = Pyramid::Util::LogLevel::Info;
        config.fileLevel = Pyramid::Util::LogLevel::Debug;
        config.logFilePath = "mygame.log";
        config.enableSourceLocation = true;
        PYRAMID_CONFIGURE_LOGGER(config);
        
        PYRAMID_LOG_INFO("MyGame starting up...");
        
        // Initialize game systems
        if (!InitializeRenderer()) {
            PYRAMID_LOG_ERROR("Failed to initialize renderer");
            return;
        }
        
        if (!LoadAssets()) {
            PYRAMID_LOG_ERROR("Failed to load game assets");
            return;
        }
        
        PYRAMID_LOG_INFO("MyGame initialized successfully");
    }
    
    void onUpdate(float deltaTime) override {
        PYRAMID_LOG_TRACE("Frame update: deltaTime = ", deltaTime);
        
        // Performance monitoring
        if (deltaTime > 0.033f) {  // > 30 FPS
            PYRAMID_LOG_WARN("Frame time spike: ", deltaTime * 1000.0f, " ms");
        }
        
        // Update game systems
        UpdatePlayer(deltaTime);
        UpdateEnemies(deltaTime);
        UpdatePhysics(deltaTime);
    }
    
private:
    bool InitializeRenderer() {
        PYRAMID_LOG_DEBUG("Initializing renderer...");
        auto* device = GetGraphicsDevice();
        
        if (!device) {
            PYRAMID_LOG_ERROR("Graphics device is null");
            return false;
        }
        
        PYRAMID_LOG_INFO("Graphics device initialized: OpenGL ", device->GetVersion());
        return true;
    }
    
    void UpdatePlayer(float deltaTime) {
        if (player.health <= 0) {
            PYRAMID_LOG_WARN("Player health critical: ", player.health);
            
            // Log structured data for analytics
            std::unordered_map<std::string, std::string> deathData;
            deathData["cause"] = lastDamageSource;
            deathData["level"] = currentLevel;
            deathData["playtime"] = std::to_string(totalPlayTime);
            deathData["position"] = player.position.ToString();
            
            PYRAMID_LOG_STRUCTURED(Pyramid::Util::LogLevel::Info, "Player death", deathData);
        }
    }
};
```

### System Component Logging

```cpp
class ResourceManager {
public:
    bool LoadTexture(const std::string& path) {
        PYRAMID_LOG_DEBUG("Loading texture: ", path);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Load texture
        auto texture = TextureLoader::Load(path);
        if (!texture) {
            PYRAMID_LOG_ERROR("Failed to load texture: ", path);
            return false;
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto loadTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        
        PYRAMID_LOG_INFO("Texture loaded: ", path, " (", loadTime, " ms)");
        
        // Log structured performance data
        std::unordered_map<std::string, std::string> perfData;
        perfData["resource_type"] = "texture";
        perfData["file_path"] = path;
        perfData["load_time_ms"] = std::to_string(loadTime);
        perfData["file_size"] = std::to_string(texture->GetSizeInBytes());
        
        PYRAMID_LOG_STRUCTURED(Pyramid::Util::LogLevel::Debug, "Resource loaded", perfData);
        
        return true;
    }
};
```

### Error Handling with Logging

```cpp
class NetworkManager {
public:
    bool ConnectToServer(const std::string& address, int port) {
        PYRAMID_LOG_INFO("Attempting to connect to server: ", address, ":", port);
        
        try {
            socket.connect(address, port);
            PYRAMID_LOG_INFO("Successfully connected to server");
            return true;
        }
        catch (const NetworkException& e) {
            PYRAMID_LOG_ERROR("Network connection failed: ", e.what());
            
            // Log detailed error information
            std::unordered_map<std::string, std::string> errorData;
            errorData["error_type"] = "network_connection";
            errorData["server_address"] = address;
            errorData["server_port"] = std::to_string(port);
            errorData["error_code"] = std::to_string(e.getErrorCode());
            errorData["error_message"] = e.what();
            
            PYRAMID_LOG_STRUCTURED(Pyramid::Util::LogLevel::Error, "Connection failed", errorData);
            return false;
        }
    }
};
```

## Performance Considerations

### Early Exit Filtering

The logging system implements early exit filtering to minimize performance impact:

```cpp
// If the log level is below the threshold, the entire expression is skipped
PYRAMID_LOG_DEBUG("Expensive operation: ", ExpensiveFunction());
// ExpensiveFunction() is only called if Debug level is enabled
```

### String Construction Optimization

```cpp
// Efficient string building with minimal allocations
PYRAMID_INFO_STREAM() << "Player " << playerId 
                      << " scored " << score 
                      << " points in " << gameTime << " seconds";
```

### Thread Safety

All logging operations are thread-safe:

```cpp
// Multiple threads can log simultaneously
std::thread t1([]() {
    PYRAMID_LOG_INFO("Thread 1 message");
});

std::thread t2([]() {
    PYRAMID_LOG_INFO("Thread 2 message");
});
```

## File Rotation

The logging system automatically rotates log files:

```cpp
// Configuration for file rotation
config.maxFileSize = 10 * 1024 * 1024; // 10MB per file
config.maxFiles = 5; // Keep 5 files total

// Results in files:
// - game.log (current)
// - game.log.1 (previous)
// - game.log.2
// - game.log.3
// - game.log.4 (oldest, deleted when new file created)
```

## Log Output Format

### Console Output Format
```
[2024-01-15 14:30:25.123] [INFO] Application started with version: 1.0.0
[2024-01-15 14:30:25.124] [WARN] Low memory warning: 512 MB remaining
[2024-01-15 14:30:25.125] [ERROR] Failed to load texture: player.png
```

### File Output Format with Source Location
```
[2024-01-15 14:30:25.123] [INFO] [Game.cpp:45:onCreate] Application started with version: 1.0.0
[2024-01-15 14:30:25.124] [WARN] [ResourceManager.cpp:123:LoadTexture] Low memory warning: 512 MB remaining
[2024-01-15 14:30:25.125] [ERROR] [TextureLoader.cpp:67:Load] Failed to load texture: player.png
```

### Structured Log Format
```json
{
    "timestamp": "2024-01-15T14:30:25.123Z",
    "level": "INFO",
    "message": "Level completed",
    "fields": {
        "user_id": "12345",
        "level_name": "forest_level_1",
        "score": "1500",
        "time_elapsed": "180.5"
    }
}
```

## Integration with Development Tools

### Debug Builds
```cpp
#ifdef PYRAMID_DEBUG
    PYRAMID_LOG_DEBUG("Debug build active - additional logging enabled");
    PYRAMID_DEBUG_ASSERT(condition, "Debug assertion with detailed info");
#endif
```

### Profiling Integration
```cpp
class ProfiledFunction {
public:
    ProfiledFunction(const std::string& name) : m_name(name) {
        m_startTime = std::chrono::high_resolution_clock::now();
        PYRAMID_LOG_TRACE("Entering function: ", m_name);
    }
    
    ~ProfiledFunction() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float, std::milli>(endTime - m_startTime).count();
        PYRAMID_LOG_TRACE("Exiting function: ", m_name, " (", duration, " ms)");
    }
    
private:
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};

#define PYRAMID_PROFILE_FUNCTION() ProfiledFunction _prof(__FUNCTION__)
```

## See Also

- [Core Game Class](../Core/Game.md) - Main game loop and initialization
- [Error Handling](../Core/ErrorHandling.md) - Exception handling and error reporting
- [Performance Profiling](../Utils/Profiling.md) - Performance monitoring and optimization
- [Configuration System](../Utils/Configuration.md) - Application configuration management
