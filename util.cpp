#include "util.h"

QColor inactiveGrayFrom(const QColor &color)
{
    int gray = qGray(color.rgb());
    if (gray <= 200)
    {
        gray += 55;
        gray = qMax(gray, 115);
    }
    else gray -= 45;
    return { gray, gray, gray };
}

double perceptiveLuminance(const QColor &color)
{
    return (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255.0F;
}
