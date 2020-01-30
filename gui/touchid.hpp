#pragma once

#ifndef __TESTID_H
#define __TESTID_H

#include "gtkmm.hpp"

#include "gui/inotifyfs.h"

#include <mutex>
#include <atomic>

class TouchID: public CalibrationArea
{
public:
    TouchID(PtrCalibrator calb, PtrCommonData data, Gtk::Window *_parent);

protected:

    bool on_expose_event(GdkEventExpose *event) override;

    bool on_timer_signal() override;

    static void eventTouchID(int id);

private:
    void setCoordClose(Cairo::RefPtr<Cairo::Context> cr);

    Color pointsColor;
    Color textColor;
    Color clockColor;

    double XClose;
    double YClose;
    double WidthClose;
    double HeightClose;

    int del1 = 10;

    std::vector<std::shared_ptr<InotifyFS>> arrayInotifyFS;
};

#endif
