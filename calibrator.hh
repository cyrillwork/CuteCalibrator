/*
 * Copyright (c) 2009 Tias Guns
 * Copyright (c) 2009 Soren Hauberg
 * Copyright (c) 2011 Antoine Hue
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

#ifndef _calibrator_hh
#define _calibrator_hh

#include <stdexcept>
#include <stdio.h>
#include <vector>
#include <memory>

#include "calibratorbuilder.hh"


// XXX: we currently don't handle lines that are longer than this
#define MAX_LINE_LEN    1024
#define RESERVE_POINTS  5000


/*
 * Number of blocks. We partition the screen into 'num_blocks' x 'num_blocks'
 * rectangles of equal size. We then ask the user to press points that are
 * located at the corner closes to the center of the four blocks in the corners
 * of the screen. The following ascii art illustrates the situation. We partition
 * the screen into 8 blocks in each direction. We then let the user press the
 * points marked with 'O'.
 *
 *   +--+--+--+--+--+--+--+--+
 *   |  |  |  |  |  |  |  |  |
 *   +--O--+--+--+--+--+--O--+
 *   |  |  |  |  |  |  |  |  |
 *   +--+--+--+--+--+--+--+--+
 *   |  |  |  |  |  |  |  |  |
 *   +--+--+--+--+--+--+--+--+
 *   |  |  |  |  |  |  |  |  |
 *   +--+--+--+--+--+--+--+--+
 *   |  |  |  |  |  |  |  |  |
 *   +--+--+--+--+--+--+--+--+
 *   |  |  |  |  |  |  |  |  |
 *   +--+--+--+--+--+--+--+--+
 *   |  |  |  |  |  |  |  |  |
 *   +--O--+--+--+--+--+--O--+
 *   |  |  |  |  |  |  |  |  |
 *   +--+--+--+--+--+--+--+--+
 */
const int num_blocks = 8;


/// Names of the points
enum {
    UL = 0, // Upper-left
    UR = 1, // Upper-right
    LL = 2, // Lower-left
    LR = 3,  // Lower-right
    NUM_POINTS
};



class WrongCalibratorException : public std::invalid_argument {
    public:
        WrongCalibratorException(const std::string& msg = "") :
            std::invalid_argument(msg) {}
};


class Calibrator;
using PtrCalibrator = std::shared_ptr<Calibrator>;

/// Base class for calculating new calibration parameters
class Calibrator
{

public:

    /// Parse arguments and create calibrator
    static PtrCalibrator make_calibrator(int argc, char** argv);

    Calibrator(const PtrCalibratorBuilder builder);

    virtual ~Calibrator() {}

    /// get the number of clicks already registered
    int get_numclicks() const
    { return clicked.num; }

    /// get the number of clicks already registered
    int get_X(int i) const
    { return clicked.x[i]; }

    int get_Y(int i) const
    { return clicked.y[i]; }

    void  set_numclicks(int numclicks)
    { clicked.num = numclicks; }

    /// reset clicks
    void reset()
    {  clicked.num = 0; clicked.x.clear(); clicked.y.clear();}

    /// add a click with the given coordinates
    bool add_click(int x, int y);

    void add_click_simple(int x, int y);

    virtual void restore_calibration() = 0;

    /// calculate and apply the calibration
    virtual bool finish(int width, int height);
    /// get the sysfs name of the device,
    /// returns NULL if it can not be found
    const char* get_sysfs_name();


    static bool getVerbose();


    static const std::string & getPathResource();

    std::shared_ptr<CalibratorBuilder> options;

    void setBigReserve(){
        clicked.x.reserve(RESERVE_POINTS);
        clicked.y.reserve(RESERVE_POINTS);
    };


protected:
    /// check whether the coordinates are along the respective axis
    bool along_axis(int xy, int x0, int y0);

    /// Apply new calibration, implementation dependent
    virtual bool finish_data(const XYinfo new_axys) =0;


    /// Check whether the given name is a sysfs device name
    bool is_sysfs_name(const char* name);

    /// Check whether the X server has xorg.conf.d support
    bool has_xorgconfd_support(Display* display=NULL);

    static int find_device(const char* pre_device, bool list_devices,
            XID& device_id, const char*& device_name, XYinfo& device_axys, XID& device_id_multi);


    /// Clicked values (screen coordinates)
    struct {
        /// actual number of clicks registered
        int num;
        /// click coordinates
        std::vector<int> x, y;
    } clicked;

    /// Be verbose or not
    static bool verbose;

    /// Original values
    XYinfo old_axys;

    /// Restore values
    XYinfo restore_axys;

private:

    // sysfs path/file
    static const char* SYSFS_INPUT;
    static const char* SYSFS_DEVNAME;

    static std::string pathResource;
};



// Interfance for a CalibratorTester
class CalibratorTesterInterface
{
public:
    // emulate the driver processing the coordinates in 'raw'
    virtual XYinfo emulate_driver(const XYinfo& raw, bool useNewAxis, const XYinfo& screen, const XYinfo& device) = 0;

    virtual void new_axis_print() = 0;

    //* From Calibrator
    /// add a click with the given coordinates
    virtual bool add_click(int x, int y) = 0;
    /// calculate and apply the calibration
    virtual bool finish(int width, int height) = 0;
};

#endif
