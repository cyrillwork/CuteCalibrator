
#include "calibration.hpp"


#define MAX_RIGHT       10
#define MAX_LEFT        69
#define MOVE_STEP       2


Calibration::Calibration(PtrCalibrator calb, PtrCommonData data):
    CalibrationArea (calb, data),
    animate(0),
    animateDirection(true)
{
    // Listen for mouse events
    add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK);


    for(const auto& iii: fileNames)
    {
        try {
            images.push_back( Cairo::ImageSurface::create_from_png(Calibrator::getPathResource() + iii) );
        }
        catch (std::exception)
        {
            std::cout << "Error load or parse file " << Calibrator::getPathResource() + iii << std::endl;
            loadImages = false;
        }
    }

}

bool Calibration::on_expose_event(GdkEventExpose *event)
{
    // check that screensize did not change (if no manually specified geometry)
    if (calibrator->options->getGeometry() == NULL &&
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
        cr->set_font_size(currentFont.fontSize);
        setColor(cr, Blue);

        double text_height = -1;
        double text_width = -1;

        Cairo::TextExtents extent;

        for (auto it = commonData->getDisplay_texts()->begin(); it != commonData->getDisplay_texts()->end(); ++it)
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
                cr->get_text_extents((*commonData->getDisplay_texts())[FirstLine], extent);
                cr->move_to(x + (text_width-extent.width)/2, y);
                cr->show_text((*commonData->getDisplay_texts())[FirstLine]);
                y += text_height + currentFont.interLines;

                cr->get_text_extents((*commonData->getDisplay_texts())[SecondLine], extent);
                cr->move_to(x + (text_width-extent.width)/2, y);
                cr->show_text((*commonData->getDisplay_texts())[SecondLine]);
                y += text_height + currentFont.interLines;
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

                cr->move_to(X[i] - commonData->getCrossLines()*0.25, Y[i]);
                cr->rel_line_to(commonData->getCrossLines()*0.5, 0);

                cr->move_to(X[i], Y[i] - commonData->getCrossLines()*0.25);
                cr->rel_line_to(0, commonData->getCrossLines()*0.5);
                cr->stroke();

                cr->set_line_width(commonData->getDefaultBoarderWidth());

                cr->arc(X[i], Y[i], commonData->getCrossCircle()*0.5, 0.0, 2.0 * M_PI);
                cr->stroke();

                cr->arc(X[i], Y[i], commonData->getCrossCircle()*0.75, 0.0, 2.0 * M_PI);
                cr->stroke();

                //cr->set_line_width(boarder);
                cr->arc(X[i], Y[i], commonData->getCrossCircle(), 0.0, 2.0 * M_PI);
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
            cr->arc(display_width/2, display_height/2 + del_clock, commonData->getClockRadius()/2, 0.0, 2.0 * M_PI);
            cr->stroke();

            setColor(cr, Blue);
            cr->set_line_width(1);
            cr->arc(display_width/2, display_height/2 + del_clock, (commonData->getClockRadius()/2) - commonData->getClockLineWidth(), 0.0, 2.0 * M_PI);
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

            cr->set_line_width(commonData->getClockLineWidth());
            cr->arc(display_width/2, display_height/2 + del_clock, (commonData->getClockRadius() - commonData->getClockLineWidth())/2,
                    3/2.0*M_PI, (3/2.0*M_PI) + ((double)time_elapsed/(double)commonData->getMaxTime()) * 2*M_PI);

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
            cr->set_font_size(currentFont.fontSize);
            Cairo::TextExtents extent;
            cr->get_text_extents(this->message, extent);
            text_width = extent.width;
            text_height = extent.height;

            if(!showLastMessage) {
                //Draw warning messages
                x = (display_width - text_width) / 2;
                y = (display_height - text_height) / 2  + 120 + commonData->getClockRadius();
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


bool Calibration::on_button_press_event(GdkEventButton *event)
{
    // Handle click
    time_elapsed = 0;

    if(!showLastMessage)
    {
        bool success = calibrator->add_click((int)event->x_root, (int)event->y_root);
        if (!success && calibrator->get_numclicks() == 0)
        {
            draw_message((*commonData->getDisplay_texts())[MissClick].c_str());
        }
        else
        {
            draw_message(NULL);
        }
    }

    // Are we done yet?
    if (calibrator->get_numclicks() >= 4)
    {
        if(showLastMessage)
        {
            checkFinish();
        }
        else
        {
            draw_message((*commonData->getDisplay_texts())[EndMessage].c_str());
            showLastMessage = true;
        }

        time_elapsed = commonData->getMaxTime() - commonData->getLastTime();
        //std::cout << "time_elapsed=" << time_elapsed << std::endl;
    }

    // Force a redraw
    redraw();

    return true;
}

bool Calibration::on_timer_signal()
{
    if (calibrator->options->getUse_timeout())
    {
        time_elapsed += commonData->getTimeStep();
        if (time_elapsed > commonData->getMaxTime())
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
