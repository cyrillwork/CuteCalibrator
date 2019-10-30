/*
 * Copyright (c) 2009 Tias Guns
 * Copyright (c) 2009 Soren Hauberg
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

#ifndef GUI_GTKMM_HPP
#define GUI_GTKMM_HPP

#include <gtkmm/drawingarea.h>
#include "calibrator.hh"
#include "gui/gui_common.hpp"

#include <list>
#include <map>

enum
{
    RIGHT = 0,
    LEFT,
    UP,
    DOWN,
    GREEN_RIGHT,
    GREEN_LEFT,
    GREEN_UP,
    GREEN_DOWN
};

enum
{
    FirstLine = 0,
    SecondLine,
    MissClick,
    EndMessage,
    TestMessage,
    CloseButton
};

class CalibrationArea : public Gtk::DrawingArea
{
public:

    CalibrationArea(PtrCalibrator calb, PtrCommonData data);

protected:

    class Color{
    public:
        double red;
        double green;
        double blue;
    };

    // Calibrator
    PtrCalibrator calibrator;

    // Options data
    PtrCommonData commonData;

    double X[4], Y[4];
    int display_width, display_height;
    int time_elapsed;

    //std::vector<std::string> display_texts;

    const char* message;

    // Signal handlers
    virtual bool on_timer_signal() = 0;

    bool on_key_press_event(GdkEventKey *event) override;

    void checkFinish();

    // Helper functions
    void set_display_size(int width, int height);
    void redraw();
    void draw_message(const char* msg);

    void setColor(Cairo::RefPtr<Cairo::Context> cr, const Color& color) const;
    void setColora(Cairo::RefPtr<Cairo::Context> cr, const Color& color, const double alpha) const;

    bool showLastMessage        = false;
    bool successCalibaration    = false;

    const Color Black   = {0.0,     0.0,    0.0};
    const Color Green   = {0.0,     0.8,    0.0};
    const Color Blue    = {0.18,    0.37,   0.77};
    const Color Red     = {0.8,     0.0,    0.0};
    const Color White   = {1.0,     1.0,    1.0};
    const Color Gray    = {0.5,     0.5,    0.5};

    CommonData::Font currentFont;
    Cairo::RefPtr<Cairo::ToyFontFace> mainFont;
};

#endif
