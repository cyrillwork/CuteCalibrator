#include "touchempty.hpp"
#include <fstream>

void eventCloseWindow(CalibrationArea */*area*/, int /*id*/)
{
    //std::cout << "eventCloseWindow" << id << std::endl;
    ::exit(0);
}

TouchEmpty::TouchEmpty(PtrCalibrator calb, PtrCommonData data, Gtk::Window*_parent):
    CalibrationArea (calb, data, _parent),
    fileName{"/tmp/touch_empty"}
{
    calibrator->setBigReserve();

    if(!InotifyFS::FileIsExist(fileName))
    {
        std::ofstream f(fileName.c_str());
        f.close();
    }

    inotifyFS = std::make_shared<InotifyFS>(1, fileName);
    inotifyFS->Init(&eventCloseWindow);
}

bool TouchEmpty::on_expose_event(GdkEventExpose *event)
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
        cr->restore();
    }

    return true;
}

bool TouchEmpty::on_timer_signal()
{
    return true;
}



