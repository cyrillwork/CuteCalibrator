#pragma once
#ifndef __TESTEMPTY_H
#define __TESTEMPTY_H

#include "gtkmm.hpp"

#include "gui/inotifyfs.h"

#include <mutex>
#include <atomic>

class TouchEmpty: public CalibrationArea
{
public:
    TouchEmpty(PtrCalibrator calb, PtrCommonData data, Gtk::Window *_parent);

protected:
    bool on_expose_event(GdkEventExpose *event) override;
    bool on_timer_signal() override;
    friend void eventCloseWindow(CalibrationArea *area, int id);

private:
    std::shared_ptr<InotifyFS> inotifyFS;
    std::string fileName;
};

#endif
