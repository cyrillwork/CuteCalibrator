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

// Must be before Xlib stuff
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <cairomm/context.h>

#include "calibrator.hh"
#include "gui/gui_common.hpp"
#include "gui/calibration.hpp"
#include "gui/testmode.hpp"
#include "gui/touchid.hpp"

#include <iostream>
#include <thread>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{    
    auto calibrator = Calibrator::make_calibrator(argc, argv);

    //auto iii = ::system("echo");

    // GTK-mm setup
    Gtk::Main kit(argc, argv);
    Glib::RefPtr< Gdk::Screen > screen = Gdk::Screen::get_default();

    //int num_monitors = screen->get_n_monitors(); TODO, multiple monitors?
    //Gdk::Color back_color;
    //back_color.set_rgb_p(1.0, 0.0, 0.0);

    Gtk::Window win(Gtk::WindowType::WINDOW_POPUP);
    // in case of window manager: set as full screen to hide window decorations
    if(!calibrator->options->getGeometry())
    {
        Gdk::Rectangle rect;
        screen->get_monitor_geometry(0, rect);

        if(Calibrator::getVerbose())
        {
            std::cout << "X=" << rect.get_x() << std::endl;
            std::cout << "Y=" << rect.get_y() << std::endl;
            std::cout << "Width=" << rect.get_width() << std::endl;
            std::cout << "Height=" << rect.get_height() << std::endl;
        }

        // when no window manager: explicitely take size of full screen
        win.move(rect.get_x(), rect.get_y());
        win.resize(rect.get_width(), rect.get_height());

        win.fullscreen();
    }
    else
    {
        std::string str1( calibrator->options->getGeometry() );

        int pos = str1.find_first_of("x");
        if(pos != -1)
        {
            int width   = 0;
            int height  = 0;
            int X = 0;
            int Y = 0;

            std::string strWidth(str1, 0, pos);
            std::string strHeight(str1, pos + 1, str1.size() - pos - 1);

            width = std::atoi(strWidth.c_str());
            height = std::atoi(strHeight.c_str());            

            auto pos_x1 = str1.find_first_of("+");
            if(pos_x1 != std::string::npos)
            {
                auto pos_x2 = str1.find_first_of("+", pos_x1 + 1);
                if(pos_x2 != std::string::npos)
                {
                    std::string strX(str1, pos_x1 + 1, pos_x2 - pos_x1 - 1);
                    std::string strY(str1, pos_x2 + 1, str1.size() - pos_x2 - 1);

                    X = std::atoi(strX.c_str());
                    Y = std::atoi(strY.c_str());
                }
            }

            if(Calibrator::getVerbose())
            {
                std::cout << "X=" << X << std::endl;
                std::cout << "Y=" << Y << std::endl;
                std::cout << "Width=" << width << std::endl;
                std::cout << "Height=" << height << std::endl;
            }

            Calibrator::argX = X;
            Calibrator::argY = Y;

            win.move(X, Y);
            win.resize(width, height);
        }
    }

    //win.set_keep_above(false);
    //win.set_keep_below();
    win.set_keep_above();

    //win.gtk_style_set_background();
    //win.modify_bg(Gtk::STATE_NORMAL, back_color);

    auto data = std::make_shared<CommonData>();
    data->initDataFromFile(calibrator->options->getLang().toString());

    std::shared_ptr<CalibrationArea> area = nullptr;

    if(calibrator->options->getTouchID())
    {
        area = std::make_shared<TouchID>(calibrator, data, &win);
    }
    else
    {
        if(calibrator->options->getTestMode())
        {
            area = std::make_shared<TestMode>(calibrator, data, &win);
        }
        else
        {
            area = std::make_shared<Calibration>(calibrator, data, &win);
        }
    }


    win.add(*area);
    area->show();


    Gtk::Main::run(win);
    Gtk::Main::quit();
    // Remove the current window
    //mainApp->remove_window(*currentWindow);

    return 0;
}
