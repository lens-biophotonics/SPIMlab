#include "utils.h"

#include <cmath>

QColor wavelengthToColor(double wl, double gamma)
{
    /*
     * Based on code by Dan Bruton
     * http://www.physics.sfasu.edu/astro/color/spectra.html
     */

    double R, G, B, attenuation;
    if (wl >= 380 && wl < 440) {
        R = -(wl - 440) / (440 - 380);
        G = 0.0;
        B = 1.0;
    } else if (wl >= 440 && wl < 490) {
        R = 0.0;
        G = (wl - 440) / (490 - 440);
        B = 1.0;
    } else if (wl >= 490 && wl < 510) {
        R = 0.0;
        G = 1.0;
        B = -(wl - 510) / (510 - 490);
    } else if (wl >= 510 && wl < 580) {
        R = (wl - 510) / (580 - 510);
        G = 1.0;
        B = 0.0;
    } else if (wl >= 580 && wl < 645) {
        R = 1.0;
        G = -(wl - 645) / (645 - 580);
        B = 0.0;
    } else if (wl >= 645 && wl < 781) {
        R = 1.0;
        G = 0.0;
        B = 0.0;
    } else {
        R = 0.0;
        G = 0.0;
        B = 0.0;
    }

    if (wl >= 380 && wl < 420)
        attenuation = 0.3 + 0.7 * (wl - 380) / (420 - 380);
    else if (wl >= 420 && wl < 701)
        attenuation = 1.0;
    else if (wl >= 701 && wl < 781)
        attenuation = 0.3 + 0.7 * (780 - wl) / (780 - 700);
    else
        attenuation = 0.0;

    R = round(255 * pow(R * attenuation, gamma));
    G = round(255 * pow(G * attenuation, gamma));
    B = round(255 * pow(B * attenuation, gamma));

    return QColor(R, G, B);
}
