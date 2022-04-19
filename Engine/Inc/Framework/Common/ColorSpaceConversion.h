#ifndef __COLOR_CONVER_H__
#define __COLOR_CONVER_H__
#include "Framework/Math/NutMath.hpp"
namespace Engine
{
    using RGB = Vector3D<float>;
    using YCbCr = Vector3D<float>;
    const Matrix<float, 4, 4> RGB2YCbCr = { {{
        { 0.299f, -0.168736f,  0.5f     ,    0.0f },
        { 0.587f, -0.331264f, -0.418688f,    0.0f },
        { 0.114f,  0.5f     , -0.081312f,    0.0f },
        { 0.0f  ,  128.0f   ,  128.0f   ,    0.0f }
    }} };
    const Matrix<float, 4, 4> YCbCr2RGB = { {{
        {      1.0f,    1.0f     ,    1.0f  ,    0.0f },
        {      0.0f, -0.344136f,    1.772f,    0.0f },
        {    1.402f, -0.714136f,    0.0f  ,    0.0f },
        { -179.456f,  135.458816f, -226.816f,    0.0f },
    }} };

    YCbCr ConvertRGB2YCbCr(const RGB& rgb)
    {
        YCbCr result = rgb;
        //the w need be 0,so using the normal edition
        TransformNormal(result, RGB2YCbCr);
        return result;
    }

    RGB ConvertYCbCr2RGB(const YCbCr& ycbcr)
    {
        RGB result = ycbcr;
        TransformNormal(result, YCbCr2RGB);
        return result;
    }
}
#endif // !__COLOR_CONVER_H__

