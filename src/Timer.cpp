#include "Timer.hpp"

Timer::Timer(float delay, bool repeat) {
    this->delay = delay;
    this->repeat = repeat;
    this->timeElapsed = 0;
}

bool Timer::update(float deltaTime) {
    if (this->timeElapsed >= delay) {
        if (this->repeat) {
            this->timeElapsed = 0;
        }

        return true;
    } else {
        this->timeElapsed += deltaTime;
    }

    return false;
}

