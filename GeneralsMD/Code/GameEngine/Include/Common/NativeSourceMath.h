// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "WWMath/matrix3d.h"
#include <cmath>

// Game-owned adapters for the original source operations. Use only unchanged
// public WWMath headers; do not replace library members or import D3DX.
inline float nativeSourceYaw(const Matrix3D& matrix) noexcept {
    return WWMath::Atan2(matrix[1][0], matrix[0][0]);
}
inline Vector3 nativeSourceRotateVector(const Matrix3D& matrix, const Vector3& vector) noexcept {
    return Vector3(
        (matrix[0][0]*vector[0] + matrix[0][1]*vector[1] + matrix[0][2]*vector[2]),
        (matrix[1][0]*vector[0] + matrix[1][1]*vector[1] + matrix[1][2]*vector[2]),
        (matrix[2][0]*vector[0] + matrix[2][1]*vector[1] + matrix[2][2]*vector[2]));
}
inline void nativeSourceBuildTransformMatrix(Matrix3D& matrix, const Vector3& position,
                                            const Vector3& direction) noexcept {
    // Source precondition: direction is unitized. Preserve the original zero
    // XY projection fallback, yaw-before-pitch order and untouched translation.
    float sinp, cosp, siny, cosy;
    const float len2 = static_cast<float>(std::sqrt(direction.X*direction.X + direction.Y*direction.Y));
    sinp = direction.Z;
    cosp = len2;
    if (len2 != 0.0f) {
        siny = direction.Y / len2;
        cosy = direction.X / len2;
    } else {
        siny = 0.0f;
        cosy = 1.0f;
    }
    matrix.Make_Identity();
    matrix.Translate(position);
    matrix.Rotate_Z(siny, cosy);
    matrix.Rotate_Y(-sinp, cosp);
}
