#pragma once

struct quaternion
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;

    explicit quaternion() = default;
    explicit quaternion(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    explicit quaternion(vec3 v, f32 s) : x(v.x), y(v.y), z(v.z), w(s) {}

    inline quaternion operator *(quaternion q)
    {
        quaternion Result = quaternion(
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        );
        return Result;
    }
};

inline mat4
GetRotationMatrix(quaternion q)
{
    f32 x2 = Square(q.x);
    f32 y2 = Square(q.y);
    f32 z2 = Square(q.z);
    f32 xy = q.x * q.y;
    f32 xz = q.x * q.z;
    f32 yz = q.y * q.z;
    f32 wx = q.w * q.x;
    f32 wy = q.w * q.y;
    f32 wz = q.w * q.z;

    mat4 Result = mat4(
        vec4(1.f - 2.f * (y2 + z2), 2.f * (xy - wz), 2.f * (xz + wy), 0.f),
        vec4(2.f * (xy + wz), 1.f - 2.f * (x2 + z2), 2.f * (yz - wx), 0.f),
        vec4(2.f * (xz - wy), 2.f * (yz + wx), 1.f - 2.f * (x2 + y2), 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}
