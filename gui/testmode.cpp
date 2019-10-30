
#include "testmode.hpp"

TestMode::TestMode(PtrCalibrator calb, PtrCommonData data):
    CalibrationArea (calb, data),
    isPressButton(false),
    pointsColor(Gray), textColor(Blue), clockColor(Blue),
    XClose(-1), YClose(-1), WidthClose(-1), HeightClose(-1)
{
    // Listen for mouse events
    add_events( Gdk::KEY_PRESS_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::BUTTON_MOTION_MASK );

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


        auto crossCircle = commonData->getCrossCircle()*0.1;
        auto widthPoint = crossCircle;

        setColor(cr, pointsColor);
        cr->set_line_width(2*widthPoint);

        // Draw the points
        for (int i = 0; i < calibrator->get_numclicks(); ++i)
        {
            if(!checkCloseButton(calibrator->get_X(i), calibrator->get_Y(i)))
            {
                setColor(cr, pointsColor);
                cr->arc(calibrator->get_X(i), calibrator->get_Y(i), widthPoint, 0.0, 2.0 * M_PI);
                cr->stroke();
            }
            else
            {
                setColora(cr, pointsColor, 0.1);
                cr->arc(calibrator->get_X(i), calibrator->get_Y(i), widthPoint, 0.0, 2.0 * M_PI);
                cr->stroke();
            }
        }



        { //Draw Main Text
            y -= 3;
            setColor(cr, textColor);

            cr->get_text_extents((*commonData->getDisplay_texts())[FirstLine], extent);
            cr->move_to(x + (text_width-extent.width)/2, y);
            cr->show_text((*commonData->getDisplay_texts())[TestMessage]);

            y += text_height + currentFont.interLines;

            cr->stroke();
        }

        { //Draw close button
            cr->set_line_width(2);

            if(XClose == -1)
            {
                setCoordClose(cr);
            }

            cr->rectangle(XClose - del1, YClose  -  HeightClose - del1, WidthClose + 2*del1, HeightClose + 2*del1);

            cr->move_to(XClose, YClose);
            cr->show_text((*commonData->getDisplay_texts())[CloseButton]);

            cr->stroke();
        }


        //Draw clock
        {
            int del_clock = 120;

            // Draw the clock background
            setColor(cr, clockColor);
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

        cr->restore();

    }

    return true;
}


bool TestMode::on_button_press_event(GdkEventButton *event)
{

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
    //std::cout << "on_button_press_event" << std::endl;
    if(checkCloseButton((int)event->x_root, (int)event->y_root))
    {
       exit(0);
    }

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

void TestMode::setCoordClose(Cairo::RefPtr<Cairo::Context> cr)
{
    //Draw close button
    double XXX =  X[2];
    double YYY =  Y[2];

    Cairo::TextExtents extentClose;

    cr->get_text_extents((*commonData->getDisplay_texts())[CloseButton], extentClose);

    WidthClose  = extentClose.width;
    HeightClose = extentClose.height;

    XClose = XXX - (extentClose.width/2);
    YClose = YYY - (extentClose.height/2);
}

bool TestMode::checkCloseButton(double X, double Y)
{
    int del2 = 10;
    if ( (XClose - del1 - del2 < X ) && ( X < XClose - del1 + WidthClose + 2*del1 + del2) &&
            (YClose  -  HeightClose - del1 - del2 < Y) && (Y < YClose + del1 + del2)
            )
    {
        return true;
    }

    return false;

}
