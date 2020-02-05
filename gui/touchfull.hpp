#pragma once
#ifndef __TOUCHFULL_H
#define __TOUCHFULL_H

#include "gtkmm.hpp"

#include "gui/inotifyfs.h"

class TouchFull: public CalibrationArea
{
public:    
    enum class TouchFullMode
    {
        TouchDetect = 1,
        Calibration
    };

    TouchFull(PtrCalibrator calb, PtrCommonData data, Gtk::Window *_parent);

    friend void eventTouchID(CalibrationArea *area, int id);

protected:
    bool on_expose_event(GdkEventExpose *event) override;
    bool on_button_press_event(GdkEventButton *event) override;
    bool on_timer_signal() override;

    void printTouchInfo();

private:
    int animate;
    bool animateDirection;
    bool loadImages;
    TouchFullMode  currentMode;

    std::vector<std::shared_ptr<InotifyFS>> arrayInotifyFS;

    std::vector<std::string> fileNames = {
                                            "arrow1.png",       "arrow2.png",       "arrow3.png",       "arrow4.png",
                                            "arrowGreen1.png",  "arrowGreen2.png",  "arrowGreen3.png",  "arrowGreen4.png"
                                         };                                         
    std::vector<Cairo::RefPtr<Cairo::ImageSurface>> images;
};

#endif
