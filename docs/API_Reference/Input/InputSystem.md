# Input System API Reference

## Overview

The Pyramid Engine's Input System provides comprehensive input handling for keyboard, mouse, and gamepad devices. The system is designed to be responsive, configurable, and easy to integrate with game logic.

**Note**: The Input System is currently in development. This documentation represents the planned API and architecture.

## Planned Features

### Core Input Features
- **Multi-Device Support**: Keyboard, mouse, and multiple gamepad support
- **Event-Based Input**: Real-time input event processing and queuing
- **Polling Interface**: Direct input state querying for game loops
- **Input Mapping**: Customizable input mapping and key binding system
- **Action System**: High-level action-based input handling
- **Dead Zone Configuration**: Configurable analog input dead zones and curves

### Advanced Features
- **Input Recording**: Record and playback input sequences for testing and demos
- **Accessibility Support**: Alternative input methods and accessibility features
- **Multiple Player Support**: Multiple gamepad management for local multiplayer
- **Platform Abstraction**: Unified input handling across different platforms
- **Hot-Plugging**: Dynamic gamepad connection and disconnection handling

## Planned API Structure

### InputManager Class

The main input management system:

```cpp
class InputManager {
public:
    static InputManager& GetInstance();
    
    // System management
    bool Initialize(const InputConfig& config);
    void Shutdown();
    void Update();
    
    // Event processing
    void ProcessEvents();
    void RegisterEventHandler(InputEventHandler* handler);
    void UnregisterEventHandler(InputEventHandler* handler);
    
    // Keyboard input
    bool IsKeyPressed(KeyCode key) const;
    bool IsKeyJustPressed(KeyCode key) const;
    bool IsKeyJustReleased(KeyCode key) const;
    
    // Mouse input
    bool IsMouseButtonPressed(MouseButton button) const;
    bool IsMouseButtonJustPressed(MouseButton button) const;
    bool IsMouseButtonJustReleased(MouseButton button) const;
    Math::Vec2 GetMousePosition() const;
    Math::Vec2 GetMouseDelta() const;
    float GetMouseWheelDelta() const;
    
    // Gamepad input
    u32 GetConnectedGamepadCount() const;
    bool IsGamepadConnected(u32 gamepadId) const;
    bool IsGamepadButtonPressed(u32 gamepadId, GamepadButton button) const;
    bool IsGamepadButtonJustPressed(u32 gamepadId, GamepadButton button) const;
    bool IsGamepadButtonJustReleased(u32 gamepadId, GamepadButton button) const;
    float GetGamepadAxis(u32 gamepadId, GamepadAxis axis) const;
    Math::Vec2 GetGamepadStick(u32 gamepadId, GamepadStick stick) const;
    
    // Action system
    void DefineAction(const std::string& actionName, const InputBinding& binding);
    bool IsActionPressed(const std::string& actionName) const;
    bool IsActionJustPressed(const std::string& actionName) const;
    bool IsActionJustReleased(const std::string& actionName) const;
    float GetActionValue(const std::string& actionName) const;
    
    // Input mapping
    void SetKeyBinding(const std::string& actionName, KeyCode key);
    void SetMouseBinding(const std::string& actionName, MouseButton button);
    void SetGamepadBinding(const std::string& actionName, u32 gamepadId, GamepadButton button);
    void LoadInputMap(const std::string& filePath);
    void SaveInputMap(const std::string& filePath) const;
    
    // Configuration
    void SetMouseSensitivity(float sensitivity);
    void SetGamepadDeadZone(u32 gamepadId, GamepadStick stick, float deadZone);
    void SetGamepadAxisCurve(u32 gamepadId, GamepadAxis axis, const InputCurve& curve);
    
private:
    InputManager() = default;
    ~InputManager() = default;
};
```

### Input Event System

Event-based input handling:

```cpp
// Base input event
struct InputEvent {
    InputEventType type;
    double timestamp;
};

// Keyboard events
struct KeyboardEvent : public InputEvent {
    KeyCode key;
    bool pressed;
    bool repeat;
    u32 modifiers;  // Shift, Ctrl, Alt, etc.
};

// Mouse events
struct MouseButtonEvent : public InputEvent {
    MouseButton button;
    bool pressed;
    Math::Vec2 position;
};

struct MouseMoveEvent : public InputEvent {
    Math::Vec2 position;
    Math::Vec2 delta;
};

struct MouseWheelEvent : public InputEvent {
    float deltaX;
    float deltaY;
};

// Gamepad events
struct GamepadConnectionEvent : public InputEvent {
    u32 gamepadId;
    bool connected;
    std::string deviceName;
};

struct GamepadButtonEvent : public InputEvent {
    u32 gamepadId;
    GamepadButton button;
    bool pressed;
};

struct GamepadAxisEvent : public InputEvent {
    u32 gamepadId;
    GamepadAxis axis;
    float value;
};

// Event handler interface
class InputEventHandler {
public:
    virtual ~InputEventHandler() = default;
    
    virtual void OnKeyboard(const KeyboardEvent& event) {}
    virtual void OnMouseButton(const MouseButtonEvent& event) {}
    virtual void OnMouseMove(const MouseMoveEvent& event) {}
    virtual void OnMouseWheel(const MouseWheelEvent& event) {}
    virtual void OnGamepadConnection(const GamepadConnectionEvent& event) {}
    virtual void OnGamepadButton(const GamepadButtonEvent& event) {}
    virtual void OnGamepadAxis(const GamepadAxisEvent& event) {}
};
```

### Input Enumerations

```cpp
enum class KeyCode {
    // Letters
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    // Function keys
    F1 = 112, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // Arrow keys
    Left = 37, Up, Right, Down,
    
    // Common keys
    Space = 32,
    Enter = 13,
    Escape = 27,
    Tab = 9,
    Backspace = 8,
    Delete = 46,
    Insert = 45,
    Home = 36,
    End = 35,
    PageUp = 33,
    PageDown = 34,
    
    // Modifiers
    LeftShift = 160,
    RightShift = 161,
    LeftControl = 162,
    RightControl = 163,
    LeftAlt = 164,
    RightAlt = 165,
    
    // Special keys
    CapsLock = 20,
    NumLock = 144,
    ScrollLock = 145,
    PrintScreen = 44,
    Pause = 19
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,      // Additional mouse buttons
    X2 = 4
};

enum class GamepadButton {
    A = 0,           // Xbox A / PlayStation X
    B = 1,           // Xbox B / PlayStation Circle
    X = 2,           // Xbox X / PlayStation Square
    Y = 3,           // Xbox Y / PlayStation Triangle
    LeftBumper = 4,  // L1
    RightBumper = 5, // R1
    Back = 6,        // Select/Share
    Start = 7,       // Start/Options
    Guide = 8,       // Xbox/PlayStation button
    LeftStick = 9,   // L3
    RightStick = 10, // R3
    DPadUp = 11,
    DPadRight = 12,
    DPadDown = 13,
    DPadLeft = 14
};

enum class GamepadAxis {
    LeftStickX = 0,
    LeftStickY = 1,
    RightStickX = 2,
    RightStickY = 3,
    LeftTrigger = 4,
    RightTrigger = 5
};

enum class GamepadStick {
    Left = 0,
    Right = 1
};
```

### Action System

High-level action-based input:

```cpp
// Input binding description
struct InputBinding {
    enum class Type {
        Key,
        MouseButton,
        GamepadButton,
        GamepadAxis
    };
    
    Type type;
    union {
        KeyCode key;
        MouseButton mouseButton;
        struct {
            u32 gamepadId;
            GamepadButton button;
        } gamepadButton;
        struct {
            u32 gamepadId;
            GamepadAxis axis;
            float threshold;  // For axis-to-button conversion
        } gamepadAxis;
    };
};

// Action definition
struct InputAction {
    std::string name;
    std::vector<InputBinding> bindings;
    bool isAxis;  // True for analog values, false for digital
    float deadZone;
    InputCurve curve;
};

// Input curve for analog input processing
struct InputCurve {
    enum class Type {
        Linear,
        Quadratic,
        Cubic,
        Custom
    };
    
    Type type = Type::Linear;
    float sensitivity = 1.0f;
    float deadZone = 0.1f;
    std::vector<Math::Vec2> customPoints;  // For custom curves
};
```

## Planned Configuration

### InputConfig Structure

```cpp
struct InputConfig {
    // General settings
    bool enableKeyboard = true;
    bool enableMouse = true;
    bool enableGamepads = true;
    u32 maxGamepads = 4;
    
    // Mouse settings
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    bool rawMouseInput = true;
    
    // Gamepad settings
    float gamepadDeadZone = 0.15f;
    float triggerDeadZone = 0.1f;
    bool gamepadVibration = true;
    
    // Event settings
    u32 maxEventQueueSize = 1000;
    bool enableEventRecording = false;
    std::string recordingFilePath = "input_recording.dat";
    
    // Platform-specific settings
    bool enableXInput = true;      // Windows XInput
    bool enableDirectInput = true; // Windows DirectInput
    bool enableRawInput = true;    // Windows Raw Input
};
```

## Planned Usage Examples

### Basic Input Handling

```cpp
#include <Pyramid/Input/InputManager.hpp>

class GameInput : public InputEventHandler {
public:
    void Initialize() {
        auto& inputManager = InputManager::GetInstance();
        
        // Initialize input system
        InputConfig config;
        config.mouseSensitivity = 1.5f;
        config.gamepadDeadZone = 0.2f;
        
        if (!inputManager.Initialize(config)) {
            PYRAMID_LOG_ERROR("Failed to initialize input system");
            return;
        }
        
        // Register for input events
        inputManager.RegisterEventHandler(this);
        
        // Define actions
        DefineGameActions();
    }
    
    void Update() {
        auto& inputManager = InputManager::GetInstance();
        inputManager.Update();
        
        // Check action states
        if (inputManager.IsActionJustPressed("Jump")) {
            player->Jump();
        }
        
        if (inputManager.IsActionPressed("Fire")) {
            player->Fire();
        }
        
        // Get movement input
        float moveX = inputManager.GetActionValue("MoveHorizontal");
        float moveY = inputManager.GetActionValue("MoveVertical");
        
        if (Math::Abs(moveX) > 0.1f || Math::Abs(moveY) > 0.1f) {
            player->Move(Math::Vec2(moveX, moveY));
        }
        
        // Mouse look
        Math::Vec2 mouseDelta = inputManager.GetMouseDelta();
        if (mouseDelta.Length() > 0.01f) {
            camera->Rotate(mouseDelta.x, mouseDelta.y);
        }
    }
    
    // Event handlers
    void OnKeyboard(const KeyboardEvent& event) override {
        if (event.key == KeyCode::Escape && event.pressed) {
            // Open pause menu
            gameStateManager->PushState(GameState::Paused);
        }
    }
    
    void OnGamepadConnection(const GamepadConnectionEvent& event) override {
        if (event.connected) {
            PYRAMID_LOG_INFO("Gamepad connected: ", event.deviceName);
            // Update UI to show gamepad prompts
            uiManager->SetInputMode(InputMode::Gamepad);
        } else {
            PYRAMID_LOG_INFO("Gamepad disconnected: ", event.gamepadId);
            // Switch back to keyboard/mouse
            uiManager->SetInputMode(InputMode::KeyboardMouse);
        }
    }
    
private:
    void DefineGameActions() {
        auto& inputManager = InputManager::GetInstance();
        
        // Movement actions
        InputBinding moveLeft;
        moveLeft.type = InputBinding::Type::Key;
        moveLeft.key = KeyCode::A;
        inputManager.DefineAction("MoveLeft", moveLeft);
        
        InputBinding moveRight;
        moveRight.type = InputBinding::Type::Key;
        moveRight.key = KeyCode::D;
        inputManager.DefineAction("MoveRight", moveRight);
        
        // Jump action (multiple bindings)
        InputBinding jumpKey;
        jumpKey.type = InputBinding::Type::Key;
        jumpKey.key = KeyCode::Space;
        
        InputBinding jumpGamepad;
        jumpGamepad.type = InputBinding::Type::GamepadButton;
        jumpGamepad.gamepadButton.gamepadId = 0;
        jumpGamepad.gamepadButton.button = GamepadButton::A;
        
        inputManager.DefineAction("Jump", jumpKey);
        inputManager.DefineAction("Jump", jumpGamepad);  // Multiple bindings
        
        // Analog movement with gamepad
        InputBinding leftStickX;
        leftStickX.type = InputBinding::Type::GamepadAxis;
        leftStickX.gamepadAxis.gamepadId = 0;
        leftStickX.gamepadAxis.axis = GamepadAxis::LeftStickX;
        inputManager.DefineAction("MoveHorizontal", leftStickX);
        
        InputBinding leftStickY;
        leftStickY.type = InputBinding::Type::GamepadAxis;
        leftStickY.gamepadAxis.gamepadId = 0;
        leftStickY.gamepadAxis.axis = GamepadAxis::LeftStickY;
        inputManager.DefineAction("MoveVertical", leftStickY);
    }
};
```

### Advanced Input Mapping

```cpp
class InputMapper {
public:
    void LoadDefaultMappings() {
        auto& inputManager = InputManager::GetInstance();
        
        // Create action definitions
        std::vector<InputAction> actions = {
            {"Move Forward", {{InputBinding::Type::Key, {.key = KeyCode::W}}}, false},
            {"Move Backward", {{InputBinding::Type::Key, {.key = KeyCode::S}}}, false},
            {"Move Left", {{InputBinding::Type::Key, {.key = KeyCode::A}}}, false},
            {"Move Right", {{InputBinding::Type::Key, {.key = KeyCode::D}}}, false},
            {"Jump", {{InputBinding::Type::Key, {.key = KeyCode::Space}}}, false},
            {"Fire", {{InputBinding::Type::MouseButton, {.mouseButton = MouseButton::Left}}}, false},
            {"Aim", {{InputBinding::Type::MouseButton, {.mouseButton = MouseButton::Right}}}, false},
            {"Reload", {{InputBinding::Type::Key, {.key = KeyCode::R}}}, false}
        };
        
        // Add gamepad bindings
        AddGamepadMappings(actions);
        
        // Register all actions
        for (const auto& action : actions) {
            for (const auto& binding : action.bindings) {
                inputManager.DefineAction(action.name, binding);
            }
        }
    }
    
    void SaveCustomMappings(const std::string& filePath) {
        // Save current input mappings to file
        auto& inputManager = InputManager::GetInstance();
        inputManager.SaveInputMap(filePath);
    }
    
    void LoadCustomMappings(const std::string& filePath) {
        // Load custom input mappings from file
        auto& inputManager = InputManager::GetInstance();
        inputManager.LoadInputMap(filePath);
    }
    
    void SetupInputCurves() {
        auto& inputManager = InputManager::GetInstance();
        
        // Setup custom curve for camera look
        InputCurve lookCurve;
        lookCurve.type = InputCurve::Type::Quadratic;
        lookCurve.sensitivity = 2.0f;
        lookCurve.deadZone = 0.05f;
        
        inputManager.SetGamepadAxisCurve(0, GamepadAxis::RightStickX, lookCurve);
        inputManager.SetGamepadAxisCurve(0, GamepadAxis::RightStickY, lookCurve);
        
        // Linear curve for movement
        InputCurve moveCurve;
        moveCurve.type = InputCurve::Type::Linear;
        moveCurve.sensitivity = 1.0f;
        moveCurve.deadZone = 0.15f;
        
        inputManager.SetGamepadAxisCurve(0, GamepadAxis::LeftStickX, moveCurve);
        inputManager.SetGamepadAxisCurve(0, GamepadAxis::LeftStickY, moveCurve);
    }
    
private:
    void AddGamepadMappings(std::vector<InputAction>& actions) {
        // Add gamepad alternatives for each action
        for (auto& action : actions) {
            if (action.name == "Jump") {
                InputBinding gamepadBinding;
                gamepadBinding.type = InputBinding::Type::GamepadButton;
                gamepadBinding.gamepadButton = {0, GamepadButton::A};
                action.bindings.push_back(gamepadBinding);
            }
            else if (action.name == "Fire") {
                InputBinding triggerBinding;
                triggerBinding.type = InputBinding::Type::GamepadAxis;
                triggerBinding.gamepadAxis = {0, GamepadAxis::RightTrigger, 0.5f};
                action.bindings.push_back(triggerBinding);
            }
            // ... add more gamepad mappings
        }
    }
};
```

### Menu Input Handling

```cpp
class MenuInputHandler : public InputEventHandler {
public:
    void OnKeyboard(const KeyboardEvent& event) override {
        if (!event.pressed) return;
        
        switch (event.key) {
            case KeyCode::Up:
            case KeyCode::W:
                NavigateUp();
                break;
                
            case KeyCode::Down:
            case KeyCode::S:
                NavigateDown();
                break;
                
            case KeyCode::Enter:
            case KeyCode::Space:
                ActivateCurrentItem();
                break;
                
            case KeyCode::Escape:
                GoBack();
                break;
        }
    }
    
    void OnGamepadButton(const GamepadButtonEvent& event) override {
        if (!event.pressed) return;
        
        switch (event.button) {
            case GamepadButton::DPadUp:
                NavigateUp();
                break;
                
            case GamepadButton::DPadDown:
                NavigateDown();
                break;
                
            case GamepadButton::A:
                ActivateCurrentItem();
                break;
                
            case GamepadButton::B:
            case GamepadButton::Back:
                GoBack();
                break;
        }
    }
    
private:
    void NavigateUp() {
        if (m_selectedIndex > 0) {
            m_selectedIndex--;
            PlayMenuSound("navigate");
        }
    }
    
    void NavigateDown() {
        if (m_selectedIndex < m_menuItems.size() - 1) {
            m_selectedIndex++;
            PlayMenuSound("navigate");
        }
    }
    
    void ActivateCurrentItem() {
        if (m_selectedIndex < m_menuItems.size()) {
            m_menuItems[m_selectedIndex]->Activate();
            PlayMenuSound("select");
        }
    }
    
    void GoBack() {
        if (m_parentMenu) {
            menuManager->SetActiveMenu(m_parentMenu);
            PlayMenuSound("back");
        }
    }
    
    size_t m_selectedIndex = 0;
    std::vector<MenuItem*> m_menuItems;
    Menu* m_parentMenu = nullptr;
};
```

## Implementation Status

**Current Status**: Planning and Architecture Phase

### Completed
- ✅ Architecture design
- ✅ API specification
- ✅ Event system design
- ✅ Action system planning

### In Progress
- 🔄 Platform-specific input implementation (Windows)
- 🔄 Keyboard and mouse input processing
- 🔄 Basic gamepad support

### Planned
- ⏳ Advanced gamepad features (rumble, LED control)
- ⏳ Input mapping system
- ⏳ Input recording and playback
- ⏳ Accessibility features
- ⏳ Multi-platform support (Linux, macOS)
- ⏳ Mobile touch input support

## Performance Considerations

### Event Processing
- **Event Queuing**: Efficient event queue with minimal allocations
- **Batch Processing**: Process multiple events in a single frame
- **State Caching**: Cache input states to avoid repeated queries

### Platform Integration
- **Native APIs**: Use platform-specific APIs for optimal performance
- **Raw Input**: Support for raw input on Windows for minimal latency
- **Hot-Plugging**: Efficient gamepad connection/disconnection handling

### Memory Management
- **Pool Allocation**: Reuse event objects to minimize garbage collection
- **State Tables**: Efficient bit-packed state storage for digital inputs
- **Analog Filtering**: Smooth analog input with minimal CPU overhead

## See Also

- [Core Game Class](../Core/Game.md) - Main game loop integration
- [Platform Window](../Platform/Window.md) - Window and system integration
- [Audio System](../Audio/AudioSystem.md) - Audio feedback for input events
- [UI System](../UI/UISystem.md) - User interface input handling

## Future Extensions

- **Touch Input**: Mobile and tablet touch input support
- **Gesture Recognition**: Advanced gesture and motion input
- **Accessibility**: Voice control and alternative input methods
- **VR Input**: Virtual reality controller and hand tracking support
- **Custom Devices**: Support for specialized input devices and hardware
