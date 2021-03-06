/*
 * Copyright (c) 2009 Tias Guns
 * Copyright 2007 Peter Hutterer (xinput_ methods from xinput)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CALIBRATOR_EVDEV_HPP
#define CALIBRATOR_EVDEV_HPP

#include "calibrator.hh"
#include <X11/extensions/XInput.h>

/***************************************
 * Class for dynamic evdev calibration
 * uses xinput "Evdev Axis Calibration"
 ***************************************/
class CalibratorEvdev: public Calibrator
{
private:
    Display     *display;
    XDeviceInfo *devInfo;
    XDevice     *iDev;

public:

    CalibratorEvdev(PtrCalibratorBuilder options, bool init = true);

    virtual ~CalibratorEvdev();

    void Init() override;

    /// calculate and apply the calibration
    bool finish(int width, int height) override;
    bool finish_data(const XYinfo new_axys) override;

    void restore_calibration() override;

    bool set_swapxy(const int swap_xy);
    bool set_invert_xy(const int invert_x, const int invert_y);
    bool set_calibration(const XYinfo new_axys);

    // xinput_ functions (from the xinput project)
    Atom xinput_parse_atom(Display *display, const char* name);

    XDeviceInfo* xinput_find_device_info(Display *display, const char* name, Bool only_extended);

    bool xinput_do_set_int_prop(const char * name,
                                 Display *display,
                                 XDevice *dev,
                                 int format,
                                 int argc,
                                 const int* argv);
protected:

    bool output_xorgconfd(const XYinfo new_axys);
    bool output_hal(const XYinfo new_axys);
    bool output_xinput(const XYinfo new_axys);
};

#endif
