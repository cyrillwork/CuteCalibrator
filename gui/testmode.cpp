
#include "testmode.hpp"

TestMode::TestMode(PtrCalibrator calb, PtrCommonData data):
    CalibrationArea (calb, data),
    isPressButton(false),
    pointsColor(Blue)
{
    // Listen for mouse events
    add_events( Gdk::KEY_PRESS_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::BUTTON_MOTION_MASK
                //| Gdk::MOTION_NOTIFY
                );
    calibrator->setBigReserve();
}

bool TestMode::on_expose_event(GdkEventExpose *event)
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
        cr->set_font_size(fontSize);
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

            {
                cr->get_text_extents((*commonData->getDisplay_texts())[FirstLine], extent);
                cr->move_to(x + (text_width-extent.width)/2, y);
                cr->show_text((*commonData->getDisplay_texts())[TestMessage]);
                y += text_height + interLines;
            }
            cr->stroke();
        }


        // Draw the points
        for (int i = 0; i < calibrator->get_numclicks(); i++)
        {
            setColor(cr, pointsColor);

            cr->set_line_width(commonData->getCrossCircle()*0.2);
            cr->arc(calibrator->get_X(i), calibrator->get_Y(i), commonData->getCrossCircle()*0.1, 0.0, 2.0 * M_PI);
            cr->stroke();
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
            cr->set_font_size(fontSize);
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


bool TestMode::on_button_press_event(GdkEventButton *event)
{
    //std::cout << "on_button_press_event" << std::endl;

    // Handle click
    time_elapsed = 0;

    calibrator->add_click_simple((int)event->x_root, (int)event->y_root);

    // Force a redraw
    redraw();
    isPressButton = true;

    return true;
}

bool TestMode::on_button_release_event(GdkEventButton *event)
{
    //std::cout << "on_button_release_event " << calibrator->get_numclicks() << std::endl;
    isPressButton = false;
    return true;
}

bool TestMode::on_motion_notify_event(GdkEventMotion*event)
{
    if(isPressButton)
    {
        calibrator->add_click_simple((int)event->x_root, (int)event->y_root);
        // Force a redraw
        redraw();
    }

    return true;
}

bool TestMode::on_timer_signal()
{
    if (calibrator->options->getUse_timeout())
    {
        time_elapsed += commonData->getTimeStep();
        if (time_elapsed > commonData->getMaxTime())
        {
            exit(0);
        }

        if(isPressButton)
        {

        }

        // Update clock
        Glib::RefPtr<Gdk::Window> win = get_window();

        if (win)
        {
//            const Gdk::Rectangle rect(display_width/2 - commonData->getClockRadius() - commonData->getClockLineWidth(),
//                                     display_height/2 - commonData->getClockRadius() - commonData->getClockLineWidth(),
//                                     2 * commonData->getClockRadius() + 1 + 2 * commonData->getClockLineWidth(),
//                                     2 * commonData->getClockRadius() + 1 + 2 * commonData->getClockLineWidth());

            const Gdk::Rectangle rect( 0, 0, display_width, display_height);

            win->invalidate_rect(rect, false);
        }
    }

    return true;
}
