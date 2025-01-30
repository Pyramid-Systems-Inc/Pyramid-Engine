#pragma once
#include <Pyramid/Core/Game.hpp>

class BasicGame : public Pyramid::Game
{
public:
    explicit BasicGame();
    virtual ~BasicGame() = default;

protected:
    void onCreate() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
};
