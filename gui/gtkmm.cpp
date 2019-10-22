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

#define MAX_RIGHT       10
#define MAX_LEFT        69
#define MOVE_STEP       2


CalibrationArea::CalibrationArea(Calibrator* calibrator0)
  : calibrator(calibrator0), time_elapsed(0), message(NULL)
{
    if(!get_display_texts_json(&display_texts, calibrator0))
    {
        // setup strings
        get_display_texts_default(&display_texts, calibrator0);
    }

    // Listen for mouse events
    add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK);
    set_flags(Gtk::CAN_FOCUS);

    // parse geometry string
    const char* geo = calibrator->get_geometry();
    if (geo != NULL)
    {
        int gw,gh;
        int res = sscanf(geo,"%dx%d",&gw,&gh);
        if (res != 2) {
            fprintf(stderr,"Warning: error parsing geometry string - using defaults.\n");
            geo = NULL;
        } else {
            set_display_size( gw, gh );
        }
    }

    if (geo == NULL)
        set_display_size(get_width(), get_height());

    // Setup timer for animation
    if(calibrator->get_use_timeout()){
        sigc::slot<bool> slot = sigc::mem_fun(*this, &CalibrationArea::on_timer_signal);
        Glib::signal_timeout().connect(slot, time_step);
    }

    for(const auto& iii: fileNames)
    {
        try {
            images.push_back( Cairo::ImageSurface::create_from_png(Calibrator::getPathResource() + iii) );
        } catch (std::exception) {
            std::cout << "Error load or parse file " << Calibrator::getPathResource() + iii << std::endl;
            loadImages = false;
        }
    }


    if(calibrator0->getSmall())
    {
       fontSize      = smallFontSize;
       interLines    = smallInterLines;
       nameFont      = smallNameFont;

       mainFont = Cairo::ToyFontFace::create(   nameFont,
                                     Cairo::FONT_SLANT_NORMAL, /*Cairo::FONT_SLANT_ITALIC,*/
                                     Cairo::FONT_WEIGHT_NORMAL /*Cairo::FONT_WEIGHT_BOLD*/);

    }
    else
    {
        mainFont = Cairo::ToyFontFace::create(   nameFont,
                                      Cairo::FONT_SLANT_NORMAL, /*Cairo::FONT_SLANT_ITALIC,*/
                                      Cairo::FONT_WEIGHT_NORMAL /*Cairo::FONT_WEIGHT_BOLD*/);
    }
    //std::cout << "nameFont " << nameFont << std::endl;

}

void CalibrationArea::set_display_size(int width, int height) {
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



bool CalibrationArea::on_expose_event(GdkEventExpose *event)
{
    // check that screensize did not change (if no manually specified geometry)
    if (calibrator->get_geometry() == NULL &&
         (display_width != get_width() ||
         display_height != get_height()) ) {
        set_display_size(get_width(), get_height());
    }

    Glib::RefPtr<Gdk::Window> window = get_window();
    if (window)
    {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
        cr->save();

        setColor(cr, White);
        cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
        cr->fill();
        //cr->clip();

        cr->set_font_face(mainFont);
        cr->set_font_size(fontSize);
        setColor(cr, Blue);

        double text_height = -1;
        double text_width = -1;

        Cairo::TextExtents extent;

        for (auto it = display_texts.begin(); it != display_texts.end(); ++it)
        {
            cr->get_text_extents(*it, extent);
            text_width = std::max(text_width, extent.width);
            text_height = std::max(text_height, extent.height);
        }
        text_height += 2;

        double x = (display_width - text_width) / 2;
        double y = (display_height - text_height) / 2 /*- 60*/;

// cyrill
//        cr->set_line_width(2);
//        cr->rectangle(x - 10, y - (display_texts.size()*text_height) - 10,
//                text_width + 20, (display_texts.size()*text_height) + 20);

        if(!showLastMessage)
        { //Draw Main Text
            y -= 3;
            //for (auto rev_it = display_texts.begin(); rev_it != display_texts.end(); ++rev_it)
            {
                cr->get_text_extents(display_texts[FirstLine], extent);
                cr->move_to(x + (text_width-extent.width)/2, y);
                cr->show_text(display_texts[FirstLine]);
                y += text_height + interLines;

                cr->get_text_extents(display_texts[SecondLine], extent);
                cr->move_to(x + (text_width-extent.width)/2, y);
                cr->show_text(display_texts[SecondLine]);
                y += text_height + interLines;
            }
            cr->stroke();
        }

        {
            // Draw the points
            for (int i = 0; i <= calibrator->get_numclicks(); i++)
            {
                //bool change = false;
                bool target = false;

                //int boarder  = defaultBoarderWidth;
                if(i >= 4)
                {
                    break;
                }

                // set color: already clicked or not
                if (i < calibrator->get_numclicks())
                {
                    //green if already clicked
                    //cr->set_source_rgb(0, 0.8, 0.0);
                    setColor(cr, Green);
                }
                else
                {
                    //red if it's target
                    //cr->set_source_rgb(0.8, 0.0, 0.0);
                    setColor(cr, Red);
                    target = true;
                }

                //std::cout << "boarder=" << boarder<< std::endl;

                cr->set_line_width(1);

                cr->move_to(X[i] - cross_lines*0.25, Y[i]);
                cr->rel_line_to(cross_lines*0.5, 0);

                cr->move_to(X[i], Y[i] - cross_lines*0.25);
                cr->rel_line_to(0, cross_lines*0.5);
                cr->stroke();

                cr->set_line_width(defaultBoarderWidth);

                cr->arc(X[i], Y[i], cross_circle*0.5, 0.0, 2.0 * M_PI);
                cr->stroke();

                cr->arc(X[i], Y[i], cross_circle*0.75, 0.0, 2.0 * M_PI);
                cr->stroke();

                //cr->set_line_width(boarder);
                cr->arc(X[i], Y[i], cross_circle, 0.0, 2.0 * M_PI);
                cr->stroke();


                if(target)
                {
                    int del1 = MAX_RIGHT;
                    int del2 = MAX_LEFT;

                    del1 -= animate;
                    del2 -= animate;

                    if(animateDirection)
                        animate += MOVE_STEP;
                    else
                        animate -= MOVE_STEP;

                    if((animate > 8) || (animate < 0))
                    {
                        animateDirection = !animateDirection;
                    }

                    if(loadImages)
                    {
                        cr->set_source (images[RIGHT], X[i] + del1 , Y[i] - 10); //anim cross_circle*0.25
                        cr->paint ();

                        cr->set_source (images[LEFT], X[i] - del2, Y[i] - 10);
                        cr->paint ();

                        cr->set_source (images[UP], X[i] - 10, Y[i] - del2);
                        cr->paint ();

                        cr->set_source (images[DOWN], X[i] - 10, Y[i] + del1);
                        cr->paint ();
                    }
                }
                else
                {
                    if(loadImages)
                    {
                        cr->set_source (images[GREEN_RIGHT], X[i] + MAX_RIGHT, Y[i] - 10); //anim cross_circle*0.25
                        cr->paint();

                        cr->set_source (images[GREEN_LEFT], X[i] - MAX_LEFT, Y[i] - 10);
                        cr->paint();

                        cr->set_source (images[GREEN_UP],   X[i] - 10, Y[i] - MAX_LEFT);
                        cr->paint ();

                        cr->set_source (images[GREEN_DOWN], X[i] - 10, Y[i] + MAX_RIGHT);
                        cr->paint ();
                    }
                }

            }
        }

        //Draw clock
        if(!showLastMessage)
        {

            int del_clock = 120;

            // Draw the clock background
            setColor(cr, Blue);
            cr->set_line_width(1);
            cr->arc(display_width/2, display_height/2 + del_clock, clock_radius/2, 0.0, 2.0 * M_PI);
            cr->stroke();

            setColor(cr, Blue);
            cr->set_line_width(1);
            cr->arc(display_width/2, display_height/2 + del_clock, (clock_radius/2) - clock_line_width, 0.0, 2.0 * M_PI);
            cr->stroke();


            /*
            if(calibrator->get_use_timeout())
            {
                // Draw the clock background
                cr->arc(display_width/2, display_height/2, clock_radius/2, 0.0, 2.0 * M_PI);

                //setColor(cr, Gray);
                setColor(cr, White);
                cr->fill_preserve();
                cr->stroke();
            }
            */

            cr->set_line_width(clock_line_width);
            cr->arc(display_width/2, display_height/2 + del_clock, (clock_radius - clock_line_width)/2,
                    3/2.0*M_PI, (3/2.0*M_PI) + ((double)time_elapsed/(double)max_time) * 2*M_PI);

            //setColor(cr, Blue);
            cr->stroke();
        }

        // Draw the message (if any)
        if (message != NULL)
        {
            //cr->set_source_rgb(0.0, 0.0, 0.0);
            //setColor(cr, Black);
            setColor(cr, Green);

            // Frame the message
            cr->set_font_size(fontSize);
            Cairo::TextExtents extent;
            cr->get_text_extents(this->message, extent);
            text_width = extent.width;
            text_height = extent.height;

            if(!showLastMessage) {
                //Draw warning messages
                x = (display_width - text_width) / 2;
                y = (display_height - text_height) / 2  + 120 + clock_radius;
            } else {
                //Draw last message
                x = (display_width - text_width) / 2;
                y = (display_height - text_height) / 2;

                cr->move_to(x + (text_width-extent.width)/2, y);
            }

            // Print the message
            cr->move_to(x, y);
            cr->show_text(this->message);
            cr->stroke();
        }

        cr->restore();

    }

    return true;
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

bool CalibrationArea::on_timer_signal()
{
    if (calibrator->get_use_timeout())
    {
        time_elapsed += time_step;
        if (time_elapsed > max_time)
        {
            if(showLastMessage)
            {
                checkFinish();
            }

            calibrator->restore_calibration();

            exit(0);
        }
    
        // Update clock
        Glib::RefPtr<Gdk::Window> win = get_window();

        if (win)
        {
            //win->set_keep_above();
            //std::cout << " set_keep_above on_timer_signal " << std::endl;

//            const Gdk::Rectangle rect(display_width/2 - clock_radius - clock_line_width,
//                                     display_height/2 - clock_radius - clock_line_width,
//                                     2 * clock_radius + 1 + 2 * clock_line_width,
//                                     2 * clock_radius + 1 + 2 * clock_line_width);

            const Gdk::Rectangle rect( 0, 0, display_width, display_height);

            win->invalidate_rect(rect, false);
        }
    }
    
    return true;
}

bool CalibrationArea::on_button_press_event(GdkEventButton *event)
{
    // Handle click
    time_elapsed = 0;

    if(!showLastMessage)
    {
        bool success = calibrator->add_click((int)event->x_root, (int)event->y_root);
        if (!success && calibrator->get_numclicks() == 0) {
            draw_message(display_texts[MissClick].c_str());
        } else {
            draw_message(NULL);
        }
    }

    // Are we done yet?
    if (calibrator->get_numclicks() >= 4) {

        if(showLastMessage)
        {
            checkFinish();
        }
        else
        {
            draw_message(display_texts[EndMessage].c_str());
            showLastMessage = true;
        }

        time_elapsed = 6*max_time/7;

    }

    // Force a redraw
    redraw();

    return true;
}

void CalibrationArea::draw_message(const char* msg)
{
    this->message = msg;
}

void CalibrationArea::setColor(Cairo::RefPtr<Cairo::Context> cr, const CalibrationArea::Color& color) const
{
    cr->set_source_rgb(color.red, color.green, color.blue);
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
