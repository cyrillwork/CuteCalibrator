#ifndef _calibratorbuilder_hh
#define _calibratorbuilder_hh

#include <X11/Xlib.h>
#include <memory>

#include "lang.hh"


int xf86ScaleAxis(int Cx, int to_max, int to_min, int from_max, int from_min);
float scaleAxis(float Cx, int to_max, int to_min, int from_max, int from_min);



struct AxisInfo {
    int min, max;
    bool invert;

    AxisInfo() : min(-1), max(-1), invert(false) { }
    AxisInfo(int mi, int ma, bool inv = false) :
        min(mi), max(ma), invert(inv) { }
    AxisInfo(const AxisInfo& old) :
        min(old.min), max(old.max), invert(old.invert) { }

    void do_invert() {
        invert = !invert;
    }
};

/// struct to hold min/max info of the X and Y axis
struct XYinfo {
    /// Axis swapped
    bool swap_xy;
    /// X, Y axis
    AxisInfo x, y;

    XYinfo() : swap_xy(false) {}

    XYinfo(int xmi, int xma, int ymi, int yma, bool swap_xy_ = false,
        bool inv_x = false, bool inv_y = false) :
        swap_xy(swap_xy_), x(xmi, xma, inv_x), y(ymi, yma, inv_y) {}

    XYinfo(const XYinfo& old) :
        swap_xy(old.swap_xy), x(old.x), y(old.y) {}

    void do_xf86ScaleAxis(const XYinfo& to, const XYinfo& from) {
        x.min = xf86ScaleAxis(x.min, to.x.max, to.x.min, from.x.max, from.x.min);
        x.max = xf86ScaleAxis(x.max, to.x.max, to.x.min, from.x.max, from.x.min);
        y.min = xf86ScaleAxis(y.min, to.y.max, to.y.min, from.y.max, from.y.min);
        y.max = xf86ScaleAxis(y.max, to.y.max, to.y.min, from.y.max, from.y.min);
    }

    void print(const char* xtra="\n") {
        printf("XYinfo: x.min=%i, x.max=%i, y.min=%i, y.max=%i, swap_xy=%i, invert_x=%i, invert_y=%i%s",
               x.min, x.max, y.min, y.max, swap_xy, x.invert, y.invert, xtra);
    }
};


/// Output types
enum OutputType
{
    OUTYPE_AUTO,
    OUTYPE_XORGCONFD,
    OUTYPE_HAL,
    OUTYPE_XINPUT
};




class CalibratorBuilder;
using PtrCalibratorBuilder = std::shared_ptr<CalibratorBuilder>;


class CalibratorBuilder
{
public:
   CalibratorBuilder():
       device_name{nullptr},
       thr_misclick{0},
       thr_doubleclick{7},
       output_type{OUTYPE_AUTO},
       geometry{nullptr},
       use_timeout{true},
       output_filename{nullptr},
       testMode{false},
       small{false},
       device_id((XID)-1),
       device_id_multi((XID)-1)
   { }

   CalibratorBuilder(const CalibratorBuilder& builder);


   CalibratorBuilder* setThrMisclick(int value);
   int getThrMisclick() const;

   CalibratorBuilder* setThrDoubleclick(int value);
   int getThrDoubleclick() const;


   XYinfo getAxys() const;
   CalibratorBuilder*setAxys(const XYinfo&value);

   const char *getDevice_name() const;

   OutputType getOutput_type() const;
   void setOutput_type(const OutputType&value);

   char* getGeometry() const;
   void setGeometry(char*value);

   bool getUse_timeout() const;
   CalibratorBuilder* setUse_timeout(bool value);

   char*getOutput_filename() const;
   void setOutput_filename(char*value);

   bool getTestMode() const;
   void setTestMode(bool value);

   Lang getLang() const;
   void setLang(const Lang&value);

   bool getSmall() const;
   void setSmall(bool value);

   void setDevice_name(const char*value);


   XID getDevice_id() const;
   void setDevice_id(const XID&value);

   XID getDevice_id_multi() const;
   void setDevice_id_multi(const XID&value);

private:
   /// Name of the device (driver)
   const char* device_name;
   XYinfo axys;
   int thr_misclick;
   int thr_doubleclick;
   OutputType output_type;
   char* geometry;
   bool use_timeout;
   char* output_filename;
   bool testMode;
   Lang lang;
   bool small;
   XID device_id;// != (XID)-1)
   XID device_id_multi;// != (XID)-1)
};


#endif
