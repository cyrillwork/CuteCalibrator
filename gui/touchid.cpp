#include "touchid.hpp"

TouchID::TouchID(PtrCalibrator calb, PtrCommonData data, Gtk::Window*_parent):
    CalibrationArea (calb, data, _parent),
    pointsColor(Gray),
    textColor(Blue),
    clockColor(Blue),
    XClose(-1), YClose(-1), WidthClose(-1), HeightClose(-1)
{
    // Listen for mouse events
    add_events( Gdk::KEY_PRESS_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::BUTTON_MOTION_MASK );

    calibrator->setBigReserve();

    if(Calibrator::getVerbose())
    {
        if(Calibrator::arrayCalibrators->size() > 0)
        {
            std::cout << "list id with input devices " << std::endl;
        }
        for(auto iii: *Calibrator::arrayCalibrators)
        {
            std::cout << "id = " << iii.id << " device node = "<< iii.deviceNode << std::endl;
        }
    }

    //touchCallBack = std::make_shared<TouchCallBack>();
    //const auto func1 = &(this->eventTouchID);

    for(auto iii: *Calibrator::arrayCalibrators)
    {
        auto ptrFS1 = std::make_shared<InotifyFS>(iii.id, iii.deviceNode);

        ptrFS1->Init(&TouchID::eventTouchID);

        arrayInotifyFS.push_back(ptrFS1);
        //std::cout << "Set inotify id = " << iii.id << " device node = "<< iii.deviceNode << std::endl;
    }

    if(calibrator->options->getTimeout() > 0)
    {
        time_elapsed = 0;//calibrator->options->getTimeout() * 1000;
        commonData->setMaxTime(calibrator->options->getTimeout() * 1000);
    }

}

bool TouchID::on_expose_event(GdkEventExpose *event)
{
    // check that screensize did not change (if no manually specified geometry)
    if (calibrator->options->getGeometry() == NULL &&
         (display_width != get_width() ||
         display_height != get_height()) )
    {
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

        { //Draw Main Text
            y -= 3;
            setColor(cr, textColor);

            cr->get_text_extents((*commonData->getDisplay_texts())[FirstLine], extent);
            cr->move_to(x + (text_width-extent.width)/2, y);

            std::string showText{""};

//            std::cout << "TouchIDMessage = " << TouchIDMessage << std::endl;
//            std::cout << "size = " << (*commonData->getDisplay_texts()).size() << std::endl;

            if( (*commonData->getDisplay_texts()).size() > TouchIDMessage)
            {
                showText = (*commonData->getDisplay_texts())[TouchIDMessage];
            }

            cr->show_text(showText);

            y += text_height + currentFont.interLines;
            cr->stroke();
        }

        //Draw clock
        {

            // Draw the clock background
            setColor(cr, clockColor);
            cr->set_line_width(1);
            cr->arc(display_width/2, display_height/2 + del_clock, commonData->getClockRadius()/2, 0.0, 2.0 * M_PI);
            cr->stroke();

            setColor(cr, Blue);
            cr->set_line_width(1);
            cr->arc(display_width/2, display_height/2 + del_clock, (commonData->getClockRadius()/2) - commonData->getClockLineWidth(), 0.0, 2.0 * M_PI);
            cr->stroke();


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

bool TouchID::on_timer_signal()
{
    if (calibrator->options->getUse_timeout())
    {
        time_elapsed += commonData->getTimeStep();

        if (time_elapsed > commonData->getMaxTime())
        {
            ::exit(0);
        }

        // Update clock
        Glib::RefPtr<Gdk::Window> win = get_window();

        if (win)
        {          
            const Gdk::Rectangle rect(
                                        display_width/2     - commonData->getClockRadius() - commonData->getClockLineWidth(),
                                        display_height/2    - commonData->getClockRadius() - commonData->getClockLineWidth() + del_clock,
                                        2 * commonData->getClockRadius() + 1 + 2 * commonData->getClockLineWidth(),
                                        2 * commonData->getClockRadius() + 1 + 2 * commonData->getClockLineWidth()
                                    );

            win->invalidate_rect(rect, false);
        }
    }

    return true;
}

void TouchID::eventTouchID(int id)
{
    std::cout << id << std::endl;
    ::exit(0);
}

void TouchID::setCoordClose(Cairo::RefPtr<Cairo::Context> cr)
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

