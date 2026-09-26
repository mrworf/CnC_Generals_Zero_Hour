#pragma once

// Internal exact shroud-binding algebra, not a general matrix interface.
#include "matrix4.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace zh::original_runtime::detail {
inline Matrix4x4 tree_shroud_projection(const Matrix4x4& view,double cell_width,double cell_height,
    int texture_width,int texture_height,double origin_x,double origin_y)
{
    if (!std::isfinite(cell_width) || !std::isfinite(cell_height) || cell_width<=0 || cell_height<=0
        || !std::isfinite(origin_x) || !std::isfinite(origin_y) || texture_width<=0 || texture_height<=0)
        throw std::runtime_error("original tree shroud extent invalid");
    const double native_values[4]={cell_width*texture_width,cell_height*texture_height,
        -origin_x+cell_width,-origin_y+cell_height};
    for(double value:native_values)
        if (!std::isfinite(value) || std::fabs(value)>std::numeric_limits<float>::max()
            || (value && static_cast<float>(value)==0))
            throw std::runtime_error("original tree shroud native Real intermediate unrepresentable");
    const float sx=1.0f/static_cast<float>(native_values[0]);
    const float sy=1.0f/static_cast<float>(native_values[1]);
    if (!std::isfinite(sx) || !std::isfinite(sy) || sx<=0 || sy<=0)
        throw std::runtime_error("original tree shroud native Real scale unrepresentable");
    double augmented[4][8]{};
    for(unsigned row=0;row<4;++row) for(unsigned col=0;col<4;++col) {
        if (!std::isfinite(view[row][col])) throw std::runtime_error("original tree shroud view nonfinite");
        augmented[row][col]=view[row][col];augmented[row][col+4]=(row==col);
    }
    for(unsigned col=0;col<4;++col) {
        unsigned pivot=col;
        for(unsigned row=col+1;row<4;++row)
            if (std::fabs(augmented[row][col])>std::fabs(augmented[pivot][col])) pivot=row;
        if (!augmented[pivot][col] || !std::isfinite(augmented[pivot][col]))
            throw std::runtime_error("original tree shroud view singular");
        if (pivot!=col) for(unsigned item=0;item<8;++item) std::swap(augmented[pivot][item],augmented[col][item]);
        const double divisor=augmented[col][col];
        for(unsigned item=0;item<8;++item) augmented[col][item]/=divisor;
        for(unsigned row=0;row<4;++row) if(row!=col) {
            const double factor=augmented[row][col];
            for(unsigned item=0;item<8;++item) augmented[row][item]-=factor*augmented[col][item];
        }
        for(const auto& row:augmented) for(double value:row)
            if (!std::isfinite(value)) throw std::runtime_error("original tree shroud inverse overflow");
    }
    const double scale[4]={sx,sy,1,1};
    const double offset[4]={static_cast<float>(native_values[2]),static_cast<float>(native_values[3]),0,0};
    Matrix4x4 result;
    // Transpose of native inverse(rowView)*rowOffset*scale: S*O*inverse(WWView).
    for(unsigned row=0;row<4;++row) for(unsigned col=0;col<4;++col) {
        const double value=scale[row]*(augmented[row][col+4]+offset[row]*augmented[3][col+4]);
        if (!std::isfinite(value) || std::fabs(value)>std::numeric_limits<float>::max()
            || (value && static_cast<float>(value)==0))
            throw std::runtime_error("original tree shroud transform unrepresentable");
        result[row][col]=static_cast<float>(value);
    }
    return result;
}
}
