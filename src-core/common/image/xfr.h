#pragma once

#include <string>
#define cimg_use_png
#define cimg_display 0
#include "CImg.h"

/*
 XFR or Color curve correction support
 by ZbychuButItWasTaken
*/

namespace image
{
    namespace xfr
    {
        // Struct representing and storing XFR Values
        struct XFR
        {
            XFR(float r1, float r2, float r3, float g1, float g2, float g3, float b1, float b2, float b3);

            // Config values
            float R[3];
            float G[3];
            float B[3];

            // LUTs
            int red_lut[1024], green_lut[1024], blue_lut[1024];
        };

        // Apply color curve correction
        void applyXFR(XFR &xfr, cimg_library::CImg<unsigned short> &r, cimg_library::CImg<unsigned short> &g, cimg_library::CImg<unsigned short> &b);

        // Load XFR file
        XFR loadXFRFromFile(std::string path);
    }
}