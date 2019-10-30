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

#include "gui/gtkmm.hpp"
#include "gui/gui_common.hpp"
#include <thread>
#include <iostream>
#include <exception>


CalibrationArea::CalibrationArea(PtrCalibrator calb, PtrCommonData data):
    calibrator(calb),
    commonData(data),
    time_elapsed(0),
    message(NULL)
{

    set_flags(Gtk::CAN_FOCUS);

    // parse geometry string
    const char* geo = calibrator->options->getGeometry();
    if (geo != NULL)
    {
        int gw,gh;
        int res = sscanf(geo,"%dx%d",&gw,&gh);
        if (res != 2)
        {
            fprintf(stderr,"Warning: error parsing geometry string - using defaults.\n");
            geo = NULL;
        }
        else
        {
            set_display_size( gw, gh );
        }
    }

    if (geo == NULL)
    {
        int width = get_width();
        int height = get_height();
        set_display_size(width, height);
    }

    // Setup timer for animation
    if(calibrator->options->getUse_timeout())
    {
        sigc::slot<bool> slot = sigc::mem_fun(*this, &CalibrationArea::on_timer_signal);
        Glib::signal_timeout().connect(slot, commonData->getTimeStep());
    }


    if(calibrator->options->getSmall())
    {        
       currentFont =  data->getSmallFont();
    }
    else
    {
        currentFont =  data->getDefaultFont();
    }

    mainFont = Cairo::ToyFontFace::create(  currentFont.name,
                                  Cairo::FONT_SLANT_NORMAL, /*Cairo::FONT_SLANT_ITALIC,*/
                                  Cairo::FONT_WEIGHT_NORMAL /*Cairo::FONT_WEIGHT_BOLD*/);


    //std::cout << "nameFont " << nameFont << std::endl;
}

void CalibrationArea::set_display_size(int width, int height)
{
    display_width = width;
    display_height = height;

    // Compute absolute circle centers
    const int delta_x = display_width/num_blocks;
    const int delta_y = display_height/num_blocks;
    X[UL] = delta_x;                     Y[UL] = delta_y;
    X[UR] = display_width - delta_x - 1; Y[UR] = delta_y;
    X[LL] = delta_x;                     Y[LL] = display_height - delta_y - 1;
    X[LR] = display_width - delta_x - 1; Y[LR] = display_height - delta_y - 1;

    // reset calibration if already started
    calibrator->reset();
}



void CalibrationArea::redraw()
{
    Glib::RefPtr<Gdk::Window> win = get_window();
    if (win)
    {
        //win->set_keep_above();
        const Gdk::Rectangle rect(0, 0, display_width, display_height);
        win->invalidate_rect(rect, false);
        //std::cout << " set_keep_above redraw " << std::endl;
    }
}



void CalibrationArea::draw_message(const char* msg)
{
    this->message = msg;
}

void CalibrationArea::setColor(Cairo::RefPtr<Cairo::Context> cr, const CalibrationArea::Color& color) const
{
    cr->set_source_rgb(color.red, color.green, color.blue);
}

void CalibrationArea::setColora(Cairo::RefPtr<Cairo::Context> cr, const CalibrationArea::Color&color, const double alpha) const
{
    cr->set_source_rgba(color.red, color.green, color.blue, alpha);
}

bool CalibrationArea::on_key_press_event(GdkEventKey *event)
{
    calibrator->restore_calibration();
    (void) event;
    exit(0);
}

void CalibrationArea::checkFinish()
{
    // Recalibrate
    successCalibaration = calibrator->finish(display_width, display_height);
    if (successCalibaration)
    {
        exit(0);
    }
    else
    {
        // TODO, in GUI ?
        fprintf(stderr, "Error1: unable to apply or save configuration values");
        exit(1);
    }
}
