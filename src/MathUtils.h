#pragma once

#include "../libs/raylib/src/raymath.h"

// ==================================================================================
// For more info on SmoothDamp see:
// https://www.rorydriscoll.com/2016/03/07/frame-rate-independent-damping-using-lerp/
// ==================================================================================

inline float SmoothDamp(float from, float to, float speed, float dt)
{
    return Lerp(from, to, 1 - expf(-speed * dt));
}

inline Vector3 SmoothDamp(Vector3 from, Vector3 to, float speed, float dt)
{
    return Vector3{
        Lerp(from.x, to.x, 1 - expf(-speed * dt)),
            Lerp(from.y, to.y, 1 - expf(-speed * dt)),
            Lerp(from.z, to.z, 1 - expf(-speed * dt))};
}

inline Quaternion SmoothDamp(Quaternion from, Quaternion to, float speed, float dt)
{
    return QuaternionSlerp( from, to, 1 - expf(-speed * dt));
}
