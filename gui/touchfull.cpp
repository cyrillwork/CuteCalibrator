#include "touchfull.hpp"

#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#define MAX_RIGHT       10
#define MAX_LEFT        69
#define MOVE_STEP       2

void eventTouchID(CalibrationArea *_area, int id)
{
    TouchFull *area = dynamic_cast<TouchFull*>(_area);
    if(area)
    {
        if(area->calibrator->getVerbose())
        {
            std::cout << "!!! eventTouchID id=" << id << std::endl;
        }

        area->calibrator->options->setDevice_id(id);

        auto it = area->calibrator->coordsMap.find(id);
        if(it != area->calibrator->coordsMap.end())
        {
            if(area->calibrator->getVerbose())
            {
                it->second.print();
            }
            area->calibrator->options->setAxys(it->second);
            area->calibrator->setOld_axys(it->second);
        }
        else
        {
            std::cout << "!!! Didn't find coords for id=" << id << std::endl;
            ::exit(1);
        }

        area->calibrator->Init();

        std::stringstream xinput;
        xinput << "xinput map-to-output "
               << id << " " << area->calibrator->options->getCrtc();

        if(::system(xinput.str().c_str()))
        {
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        if(area->calibrator->getVerbose())
        {
            std::cout << xinput.str() << std::endl;
        }

        area->currentMode = TouchFull::TouchFullMode::Calibration;
        area->time_elapsed = 0;

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        // Listen for mouse events
        area->add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK);
    }
    else
    {
        std::cout << "eventTouchID error area nullptr "  << std::endl;
    }
}

TouchFull::TouchFull(PtrCalibrator calb, PtrCommonData data, Gtk::Window*_parent):
    CalibrationArea (calb, data, _parent)
    , animate(0)
    , animateDirection(true)
    , loadImages{true}
    , currentMode{TouchFullMode::TouchDetect}
{
    if(calibrator->getVerbose())
    {
        std::cout << " TouchFull Window crtc=" << calb->options->getCrtc() << std::endl;
    }

    for(const auto& iii: fileNames)
    {
        try
        {
            images.push_back( Cairo::ImageSurface::create_from_png(Calibrator::getPathResource() + iii) );
        }
        catch (std::exception)
        {
            std::cout << "Error load or parse file " << Calibrator::getPathResource() + iii << std::endl;
            loadImages = false;
        }
    }

    for(auto iii: *Calibrator::arrayCalibrators)
    {
        if(calibrator->getVerbose())
        {
            std::cout << "id=" << iii.id << " deviceNode=" << iii.deviceNode << std::endl;
        }

        auto ptrFS1 = std::make_shared<InotifyFS>(iii.id, iii.deviceNode, this);
        ptrFS1->Init(eventTouchID);
        arrayInotifyFS.push_back(ptrFS1);
    }

    if(calibrator->options->getTimeout() > 0)
    {
        time_elapsed = 0;//calibrator->options->getTimeout() * 1000;
        commonData->setMaxTime(calibrator->options->getTimeout() * 1000);
    }
}


bool TouchFull::on_expose_event(GdkEventExpose *event)
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

        if(!showLastMessage)
        { //Draw Main Text
            y -= 3;

            cr->get_text_extents((*commonData->getDisplay_texts())[FirstLine], extent);
            cr->move_to(x + (text_width-extent.width)/2, y);

            if(currentMode == TouchFullMode::TouchDetect)
            {
                std::string showText{""};
                if( (*commonData->getDisplay_texts()).size() > TouchIDMessage)
                {
                    showText = (*commonData->getDisplay_texts())[TouchIDMessage];
                }
                cr->show_text(showText);
            }
            else
            {
                cr->show_text((*commonData->getDisplay_texts())[FirstLine]);
                y += text_height + currentFont.interLines;

                cr->get_text_extents((*commonData->getDisplay_texts())[SecondLine], extent);
                cr->move_to(x + (text_width-extent.width)/2, y);
                cr->show_text((*commonData->getDisplay_texts())[SecondLine]);
                y += text_height + currentFont.interLines;
            }

            cr->stroke();
        }

        if(currentMode == TouchFullMode::Calibration)
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
            // Draw the clock background
            setColor(cr, Blue);
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


bool TouchFull::on_button_press_event(GdkEventButton *event)
{
    if(currentMode == TouchFullMode::Calibration)
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
    }

    return true;
}

bool TouchFull::on_timer_signal()
{
    if(isCloseWindow != -1)
    { //downcount for close application
        --isCloseWindow;
        if(isCloseWindow == 0)
        {
            ::exit(0);
        }
        return true;
    }


    if (calibrator->options->getUse_timeout())
    {
        time_elapsed += commonData->getTimeStep();
        if (time_elapsed > commonData->getMaxTime())
        {
            if(currentMode == TouchFull::TouchFullMode::TouchDetect)
            {
                ::exit(0);
            }

            if(showLastMessage)
            {                                
                printTouchInfo();

                checkFinish();
                return true;
            }

            calibrator->restore_calibration();
            this->exit(0);
        }

        // Update clock
        Glib::RefPtr<Gdk::Window> win = get_window();
        if (win)
        {
            //win->set_keep_above();
            //std::cout << " set_keep_above on_timer_signal " << std::endl;

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

void TouchFull::printTouchInfo()
{
    std::stringstream sss;
    sss << "touch_id: ";
    sss << calibrator->options->getCrtc() << " ";

    if(calibrator->options->getGeometry())
    {
        sss << calibrator->options->getGeometry() << " ";
    }
    else
    {
        sss << "-1 ";
    }
    sss << calibrator->options->getDevice_id();

    std::cout << sss.str();
}
