// 颜色格式转换，速度比QColor更快

#ifndef COLOR_H
#define COLOR_H

#include <cmath>

// r，g，b，h，s，v的范围为0~255
class RgbColor
{
   public:
    unsigned char r, g, b;

    RgbColor()
    {
    }

    RgbColor(unsigned char _r, unsigned char _g, unsigned char _b)
        : r(_r), g(_g), b(_b)
    {
    }
};

class HsvColor
{
   public:
    unsigned char h, s, v;

    HsvColor()
    {
    }

    HsvColor(unsigned char _h, unsigned char _s, unsigned char _v)
        : h(_h), s(_s), v(_v)
    {
    }
};

RgbColor hsvToRgb(HsvColor hsv)
{
    if (hsv.s == 0) return RgbColor(hsv.v, hsv.v, hsv.v);

    unsigned char region, remainder, p, q, t;
    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;
    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            return RgbColor(hsv.v, t, p);
        case 1:
            return RgbColor(q, hsv.v, p);
        case 2:
            return RgbColor(p, hsv.v, t);
        case 3:
            return RgbColor(p, q, hsv.v);
        case 4:
            return RgbColor(t, p, hsv.v);
        default:
            return RgbColor(hsv.v, p, q);
    }
}

HsvColor rgbToHsv(RgbColor rgb)
{
    HsvColor hsv;
    unsigned char rgbMin, rgbMax;
    rgbMin = min(rgb.r, min(rgb.g, rgb.b));
    rgbMax = max(rgb.r, max(rgb.g, rgb.b));

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

#endif
