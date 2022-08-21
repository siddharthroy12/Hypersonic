#pragma once

class Timer {
    public:
        Timer(float delay, bool repeat);
        float delay;
        float timeElapsed;
        bool repeat;
        bool update(float deltaTime);
};
