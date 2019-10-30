#pragma once

#ifndef __TESTMODE_H
#define __TESTMODE_H

#include "gtkmm.hpp"

class TestMode: public CalibrationArea
{

public:

    TestMode(PtrCalibrator calb, PtrCommonData data);

protected:

    bool on_expose_event(GdkEventExpose *event) override;

    bool on_button_press_event(GdkEventButton *event) override;
    bool on_button_release_event(GdkEventButton *event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;

    bool on_timer_signal() override;

private:
    void setCoordClose(Cairo::RefPtr<Cairo::Context> cr);
    bool checkCloseButton(double X, double Y);

    bool isPressButton;

    Color pointsColor;
    Color textColor;
    Color clockColor;

    double XClose;
    double YClose;
    double WidthClose;
    double HeightClose;
    int del1 = 10;

};

#endif
