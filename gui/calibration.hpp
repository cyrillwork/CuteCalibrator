#pragma once

#ifndef __CALIBRATION_H
#define __CALIBRATION_H

#include "gtkmm.hpp"

class Calibration: public CalibrationArea
{
public:
    Calibration(PtrCalibrator calb, PtrCommonData data, Gtk::Window *_parent);

protected:

    bool on_expose_event(GdkEventExpose *event) override;
    bool on_button_press_event(GdkEventButton *event) override;
    bool on_timer_signal() override;

private:
    int animate;
    bool animateDirection;
    bool loadImages             = true;

    std::vector<std::string> fileNames = {
                                            "arrow1.png",       "arrow2.png",       "arrow3.png",       "arrow4.png",
                                            "arrowGreen1.png",  "arrowGreen2.png",  "arrowGreen3.png",  "arrowGreen4.png"
                                         };

    std::vector<Cairo::RefPtr<Cairo::ImageSurface>> images;

};




#endif
