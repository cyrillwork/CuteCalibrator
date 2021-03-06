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

#include "calibrator.hh"

// Calibrator implementations
#include "calibrator/Usbtouchscreen.hpp"
#include "calibrator/Evdev.hpp"
#include "calibrator/XorgPrint.hpp"

#include <cstring>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include <algorithm>
#include <unordered_map>

static const char* VERSION = "3.15";

// strdup: non-ansi
static char* my_strdup(const char* s)
{
    size_t len = strlen(s) + 1;
    void* p = malloc(len);

    if (p == nullptr)
        return nullptr;

    return (char*) memcpy(p, s, len);
}

bool isTouchDevice(XIDeviceInfo *dev)
{
    int i;
    XIAnyClassInfo **classes = dev->classes;
    int num_classes = dev->num_classes;

    bool result = false;

    for (i = 0; i < num_classes; i++)
    {
        if(classes[i]->type == XITouchClass)
        {
            result = true;
            break;
        }
    }

    return result;
}

static Atom xinput_parse_atom(Display *display, const char *name)
{
    Bool is_atom = True;
    int i;

    for (i = 0; name[i] != '\0'; i++) {
        if (!isdigit(name[i])) {
            is_atom = False;
            break;
        }
    }

    if (is_atom)
        return atoi(name);
    else
        return XInternAtom(display, name, False);
}

static std::string get_xinput_prop(  const char * name,
                                         Display *display,
                                         XID id)
{
    Atom            property;
    Atom            act_type;
    int             act_format;
    unsigned long   nitems, bytes_after;
    unsigned char   *data;
    unsigned char   buff[256] = {};

    auto iDev = XOpenDevice(display, id);

    if (!iDev)
    {
        XCloseDisplay(display);
        return "";
    }

    property = xinput_parse_atom(display, name);

    if (XGetDeviceProperty(display, iDev, property, 0, 1000, False,
                           AnyPropertyType, &act_type, &act_format,
                           &nitems, &bytes_after, &data) != Success)
    {
        XCloseDevice(display, iDev);
        XCloseDisplay(display);
    }

//    std::cout << "act_format = " << act_format << std::endl;
//    std::cout << "nitems = " << nitems << std::endl;

    if((act_format == 8) && (nitems > 0))
    {
        std::copy_n(data, nitems, buff);
        //std::cout << "buff = " << buff << std::endl;
        XFree(data);
        return std::string((char*)buff);
    }

    return "";
}

static int getIdTouch(Display *display, std::vector<int> &array_id, bool list_devices)
{
    int ndevices;
    int i, j;
    XIDeviceInfo *info, *dev;

    int result_device = -1;

    info = XIQueryDevice(display, XIAllDevices, &ndevices);

    for(i = 0; i < ndevices; i++)
    {
        dev = &info[i];

        if (dev->use == XIMasterPointer || dev->use == XIMasterKeyboard)
        {

            for (j = 0; j < ndevices; j++)
            {
                XIDeviceInfo* sd = &info[j];

                if ((sd->use == XISlavePointer || sd->use == XISlaveKeyboard) &&
                     (sd->attachment == dev->deviceid))
                {

                    auto it = std::find(array_id.begin(), array_id.end(), sd->deviceid);
                    if(it == array_id.end())
                    {
                        continue;
                    }

                    //if(isTouchDevice(sd) || (array_id.size() == 1))
                    {
                        if(result_device == -1)
                        {
                            result_device = sd->deviceid;
                        }

                        Calibrator::arrayCalibrators->push_back(
                                    CalibratorData{
                                            sd->deviceid,
                                            get_xinput_prop("Device Node", display, sd->deviceid)
                                    }
                        );

//                        if (list_devices)
//                        {
//                            printf("Device \"%s\" id=%i\n", sd->name, (int)sd->deviceid);
//                        }

                    }
                }
            }
        }
    }

    XIFreeDeviceInfo(info);

    return result_device;
}



static void print_property(Display *dpy, XDevice* dev, Atom property)
{
    Atom                act_type;
    char                *name;
    int                 act_format;
    unsigned long       j, nitems, bytes_after;
    unsigned char       *data, *ptr;
    int                 done = False, size = 0;

    name = XGetAtomName(dpy, property);
    printf("\t%s (%ld):\t", name, property);
    XFree(name);

    if (XGetDeviceProperty(dpy, dev, property, 0, 1000, False,
                           AnyPropertyType, &act_type, &act_format,
                           &nitems, &bytes_after, &data) == Success)
    {
        Atom float_atom = XInternAtom(dpy, "FLOAT", True);

        ptr = data;

        if (nitems == 0)
            printf("<no items>");

        switch(act_format)
        {
            case 8: size = sizeof(char); break;
            case 16: size = sizeof(short); break;
            case 32: size = sizeof(long); break;
        }

        for (j = 0; j < nitems; j++)
        {
            switch(act_type)
            {
                case XA_INTEGER:
                    switch(act_format)
                    {
                        case 8:
                            printf("%d", *((char*)ptr));
                            break;
                        case 16:
                            printf("%d", *((short*)ptr));
                            break;
                        case 32:
                            printf("%ld", *((long*)ptr));
                            break;
                    }
                    break;
                case XA_CARDINAL:
                    switch(act_format)
                    {
                        case 8:
                            printf("%u", *((unsigned char*)ptr));
                            break;
                        case 16:
                            printf("%u", *((unsigned short*)ptr));
                            break;
                        case 32:
                            printf("%lu", *((unsigned long*)ptr));
                            break;
                    }
                    break;
                case XA_STRING:
                    if (act_format != 8)
                    {
                        printf("Unknown string format.\n");
                        done = True;
                        break;
                    }
                    printf("\"%s\"", ptr);
                    j += strlen((char*)ptr); /* The loop's j++ jumps over the
                                                terminating 0 */
                    ptr += strlen((char*)ptr); /* ptr += size below jumps over
                                                  the terminating 0 */
                    break;
                case XA_ATOM:
                    {
                        Atom a = *(Atom*)ptr;
                        name = (a) ? XGetAtomName(dpy, a) : NULL;
                        printf("\"%s\" (%d)", name ? name : "None", (int)a);
                        XFree(name);
                        break;
                    }
                default:
                    if (float_atom != None && act_type == float_atom)
                    {
                        printf("%f", *((float*)ptr));
                        break;
                    }

                    name = XGetAtomName(dpy, act_type);
                    printf("\t... of unknown type '%s'\n", name);
                    XFree(name);
                    done = True;
                    break;
            }

            ptr += size;

            if (done == True)
                break;
            if (j < nitems - 1)
                printf(", ");
        }
        printf("\n");
        XFree(data);
    } else
        printf("\tFetch failure\n");

}


/**
 * find a calibratable touchscreen device (using XInput)
 *
 * if pre_device is nullptr, the last calibratable device is selected.
 * retuns number of devices found,
 * the data of the device is returned in the last 3 function parameters
 */
int Calibrator::find_device(const char* pre_device, bool list_devices,
        XID& device_id, const char*& device_name, XYinfo& device_axys)
{
    bool pre_device_is_id = true;
    bool pre_device_is_sysfs = false;
    int found = 0;    

    std::vector<int> array_id;


    Display* display = XOpenDisplay(nullptr);

    if (display == nullptr)
    {
        fprintf(stderr, "Unable to connect to X server\n");
        exit(1);
    }


    int xi_opcode, event, error;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error))
    {
        fprintf(stderr, "X Input extension not available.\n");
        exit(1);
    }

    // verbose, get Xi version
    if (verbose)
    {
        XExtensionVersion *version = XGetExtensionVersion(display, INAME);

        if (version && (version != (XExtensionVersion*) NoSuchExtension)) {
            printf("DEBUG: %s version is %i.%i\n",
                INAME, version->major_version, version->minor_version);
            XFree(version);
        }
    }

    if (pre_device != nullptr)
    {
        // check whether the pre_device is an ID (only digits)
        int len = strlen(pre_device);
        for (int loop=0; loop<len; loop++)
        {
            if (!isdigit(pre_device[loop]))
            {
                pre_device_is_id = false;
                break;
            }
        }
    }

    std::string pre_device_sysfs;
    if (pre_device != nullptr && !pre_device_is_id)
    {
        /* avoid overflow below - 10000 devices should be OK */
        if ( strlen(pre_device) < strlen("event") + 4 &&
             strncmp(pre_device, "event", strlen("event")) == 0 )
        {
            // check whether the pre_device is an sysfs-path name
            char filename[40]; // actually 35, but hey...
            (void) sprintf(filename, "%s/%s/%s", SYSFS_INPUT, pre_device, SYSFS_DEVNAME);

            std::ifstream ifile(filename);
            if (ifile.is_open())
            {
                if (!ifile.eof())
                {
                    pre_device_is_sysfs = true;
                    std::getline(ifile, pre_device_sysfs);
                    ifile.close();
                }
            }
        }
    }

    if (verbose)
        printf("DEBUG: Skipping virtual master devices and devices without axis valuators.\n");
    int ndevices;

    XDeviceInfoPtr list, slist;
    slist=list=(XDeviceInfoPtr) XListInputDevices (display, &ndevices);

    for (int i=0; i<ndevices; i++, list++)
    {

        if (list->use == IsXKeyboard || list->use == IsXPointer) // virtual master device
        {
            continue;
        }

        // if we are looking for a specific device
        if (pre_device != nullptr)
        {
            if ((pre_device_is_id && list->id == (XID) atoi(pre_device)) ||
                (!pre_device_is_id && strcmp(list->name, pre_device_is_sysfs ? pre_device_sysfs.c_str() : pre_device ) == 0))
            {
                // OK, fall through
            }
            else
            {
                // skip, not this device
                continue;
            }
        }

        XAnyClassPtr any = (XAnyClassPtr) (list->inputclassinfo);

        for (int j=0; j<list->num_classes; j++)
        {
            //std::cout << "any->c_class=" << any->c_class << std::endl;
            if (any->c_class == ValuatorClass)
//                && (       (list->type == 112)     // TouchScreen
//                        ||  (list->type == 109)*/ // Keyboard
//                        ))
            {
                XValuatorInfoPtr V = (XValuatorInfoPtr) any;
                XAxisInfoPtr ax = (XAxisInfoPtr) V->axes;

                //std::cout << "j=" << j << " mode = " << (int)V->mode << std::endl;

                if (V->mode != Absolute)
                {
                    if (verbose)
                        printf("DEBUG: Skipping device '%s' id=%i, does not report Absolute events.\n",
                            list->name, (int)list->id);
                }
                else
                    if (V->num_axes < 2 ||
                    (ax[0].min_value == -1 && ax[0].max_value == -1) ||
                    (ax[1].min_value == -1 && ax[1].max_value == -1))
                    {
                        if (verbose)
                            printf("DEBUG: Skipping device '%s' id=%i, does not have two calibratable axes.\n",
                                list->name, (int)list->id);
                    }
                else
                {
                    /* a calibratable device (has 2 axis valuators) */
                    found++;
                    device_id = list->id;
                    device_name = my_strdup(list->name);

                    device_axys.x.min = ax[0].min_value;
                    device_axys.x.max = ax[0].max_value;
                    device_axys.y.min = ax[1].min_value;
                    device_axys.y.max = ax[1].max_value;

                    array_id.push_back(list->id);
                    coordsMap[(int)list->id] = device_axys;

                    if (list_devices)
                    {
                        printf("Device \"%s\" id=%i\n", device_name, (int)device_id);
                    }
                }
            }

            /*
             * Increment 'any' to point to the next item in the linked
             * list.  The length is in bytes, so 'any' must be cast to
             * a character pointer before being incremented.
             */
            any = (XAnyClassPtr) ((char *) any + any->length);
        }

    }

    if(found >= 1)
    {
        auto id = getIdTouch(display, array_id, list_devices);
        if(id != -1)
        {
            device_id = id;
            device_axys = coordsMap[device_id];
        }
    }

    XFreeDeviceList(slist);
    XCloseDisplay(display);

    return found;
}

static void usage(char* cmd, unsigned thr_misclick)
{
    fprintf(stderr, "Usage: %s [-h|--help] [-v|--verbose] [--list] [--device <device name or XID or sysfs path>] [--precalib <minx> <maxx> <miny> <maxy>] [--misclick <nr of pixels>] [--output-type <auto|xorg.conf.d|hal|xinput>] [--fake] [--geometry <w>x<h>] [--no-timeout]\n", cmd);
    fprintf(stderr, "\t-h, --help: print this help message\n");
    fprintf(stderr, "\t-v, --verbose: print debug messages during the process\n");
    fprintf(stderr, "\t--version: show version of program end exit\n");
    fprintf(stderr, "\t--list: list calibratable input devices and quit\n");
    fprintf(stderr, "\t--device <device name or XID or sysfs event name (e.g event5)>: select a specific device to calibrate\n");
    fprintf(stderr, "\t--precalib: manually provide the current calibration setting (eg. the values in xorg.conf)\n");
    fprintf(stderr, "\t--misclick: set the misclick threshold (0=off, default: %i pixels)\n", thr_misclick);
    fprintf(stderr, "\t--output-type <auto|xorg.conf.d|hal|xinput>: type of config to output (auto=automatically detect, default: auto)\n");
    fprintf(stderr, "\t--fake: emulate a fake device (for testing purposes)\n");
    fprintf(stderr, "\t--geometry: format xterm style  widthxheight+X+Y (1920x1080+1920+0)\n");
    fprintf(stderr, "\t--no-timeout: turns off the timeout\n");
    fprintf(stderr, "\t--touch_id: run touch detecting mode\n");
    fprintf(stderr, "\t--touch_empty: show empty window mode\n");
    fprintf(stderr, "\t--timeout: amount sec to start calibration, use with touch_id option\n");
    fprintf(stderr, "\t--output-filename: write calibration data to file (USB: override default /etc/modprobe.conf.local\n");
    fprintf(stderr, "\t--lang <en>|<fr> :set language, default english\n");
    fprintf(stderr, "\t--small: set small type calibrator\n");
    fprintf(stderr, "\t--testmode: run test mode\n");
    fprintf(stderr, "\t--crtc: name video output (example: VGA-0 or DVI-D-0, get from xrandr) (Run touch detecting mode, then calibration)\n");
    fprintf(stderr, "\t--path <resource path>: set resource path\n");
}

PtrCalibrator Calibrator::make_calibrator(int argc, char** argv)
{
    bool list_devices = false;

    bool fake = false;
    bool precalib = false;

    XYinfo pre_axys;
    const char* pre_device = nullptr;
    const char* output_filename = nullptr;

    std::shared_ptr<CalibratorBuilder> builder = std::make_shared<CalibratorBuilder>();

    // parse input
    if (argc > 1)
    {
        for (int i = 1; i != argc; ++i)
        {
            // Display help ?
            if (strcmp("-h", argv[i]) == 0 ||
                strcmp("--help", argv[i]) == 0) {
                fprintf(stderr, "xinput_calibrator, v%s\n\n", VERSION);
                usage(argv[0], builder->getThrMisclick());
                exit(0);
            }
            else
            // Verbose output ?
            if (strcmp("-v", argv[i]) == 0 ||
                strcmp("--verbose", argv[i]) == 0)
            {
                verbose = true;
            }
            else                
            if (strcmp("--touch_empty", argv[i]) == 0)
            {
                builder->setTouchEmpty(true);
            }
            else
            if (strcmp("--crtc", argv[i]) == 0)
            {
                builder->setCrtc(argv[++i]);
            }
            else
            if (strcmp("--touch_id", argv[i]) == 0)
            {
                builder->setTouchID(true);
            }
            else
            if (strcmp("--timeout", argv[i]) == 0)
            {
                builder->setTimeout(atoi(argv[++i]));
            }
            else
            //  Test mode
            if (strcmp("--testmode", argv[i]) == 0)
            {
                builder->setTestMode(true);
            }
            else
            //  small
            if (strcmp("--small", argv[i]) == 0)
            {
                builder->setSmall(true);
            }
            else
            //  resource path
            if (strcmp("--path", argv[i]) == 0)
            {
                Calibrator::pathResource = argv[++i];
            }
            else
            if (strcmp("--lang", argv[i]) == 0)
            {
                Lang lang(argv[++i]);
                builder->setLang(lang);
            } else
            // Just list devices ?
            if (strcmp("--list", argv[i]) == 0)
            {
                list_devices = true;
            }
            else
            // Select specific device ?
            if (strcmp("--device", argv[i]) == 0)
            {
                if (argc > i+1)
                    pre_device = argv[++i];
                else
                {
                    fprintf(stderr, "Error: --device needs a device name or id as argument; use --list to list the calibratable input devices.\n\n");
                    usage(argv[0], builder->getThrMisclick());
                    exit(1);
                }
            }
            else
            // Get pre-calibration ?
            if (strcmp("--precalib", argv[i]) == 0)
            {
                precalib = true;
                if (argc > i+1)
                    pre_axys.x.min = atoi(argv[++i]);
                if (argc > i+1)
                    pre_axys.x.max = atoi(argv[++i]);
                if (argc > i+1)
                    pre_axys.y.min = atoi(argv[++i]);
                if (argc > i+1)
                    pre_axys.y.max = atoi(argv[++i]);
            }
            else
            // Get mis-click threshold ?
            if (strcmp("--misclick", argv[i]) == 0)
            {
                if (argc > i+1)
                {
                    builder->setThrMisclick(atoi(argv[++i]));
                }
                else
                {
                    fprintf(stderr, "Error: --misclick needs a number (the pixel threshold) as argument. Set to 0 to disable mis-click detection.\n\n");
                    usage(argv[0], builder->getThrMisclick());
                    exit(1);
                }
            }
            else
            // Get output type ?
            if (strcmp("--output-type", argv[i]) == 0)
            {
                if (argc > i+1)
                {
                    OutputType output_type = OUTYPE_AUTO;
                    i++; // eat it or exit
                    if (strcmp("auto", argv[i]) == 0)
                        output_type = OUTYPE_AUTO;
                    else if (strcmp("xorg.conf.d", argv[i]) == 0)
                        output_type = OUTYPE_XORGCONFD;
                    else if (strcmp("hal", argv[i]) == 0)
                        output_type = OUTYPE_HAL;
                    else if (strcmp("xinput", argv[i]) == 0)
                        output_type = OUTYPE_XINPUT;
                    else {
                        fprintf(stderr, "Error: --output-type needs one of auto|xorg.conf.d|hal|xinput.\n\n");
                        usage(argv[0], builder->getThrMisclick());
                        exit(1);
                    }
                    builder->setOutput_type(output_type);
                }
                else
                {
                    fprintf(stderr, "Error: --output-type needs one argument.\n\n");
                    usage(argv[0], builder->getThrMisclick());
                    exit(1);
                }
            } else
            // specify window geometry?
            if (strcmp("--version", argv[i]) == 0)
            {
                printf("version %s\n", VERSION);
                exit(0);
            } else
            // specify window geometry?
            if (strcmp("--geometry", argv[i]) == 0)
            {
                builder->setGeometry(argv[++i]);
            } else
            // Fake calibratable device ?
            if (strcmp("--fake", argv[i]) == 0)
            {
                fake = true;
            } else
            // Disable timeout
            if (strcmp("--no-timeout", argv[i]) == 0)
            {
                builder->setUse_timeout(false);
			} else

			// Output file
			if (strcmp("--output-filename", argv[i]) == 0) {
				output_filename = argv[++i];
			}
            // unknown option
            else
            {
                fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
                usage(argv[0], builder->getThrMisclick());
                exit(0);
            }
        }
    }

    if ((argc > 1) && (verbose))
    {
        std::cout << "Input params:"<< std::endl;

        for(int i = 1; i != argc; ++i)
        {
            std::cout << argv[i] << " ";
        }
        std::cout << std::endl;
    }

    const char* device_name = nullptr;
    XYinfo      device_axys;

    if (fake)
    {
        // Fake a calibratable device
        device_name = "Fake_device";
        device_axys = XYinfo(0,1000,0,1000);

        if (verbose)
        {
            printf("DEBUG: Faking device: %s\n", device_name);
        }

    }
    else
    {
        /// Choose the device to calibrate
        XID         device_id           = (XID) -1;        

        // Find the right device
        int nr_found = find_device(pre_device, list_devices, device_id, device_name, device_axys);

        if (list_devices)
        {
            // printed the list in find_device
            if (nr_found == 0)
            {
                printf("No calibratable devices found.\n");
            }
            exit(0);
        }

        if (nr_found == 0)
        {
            if (pre_device == nullptr)
                fprintf (stderr, "Error: No calibratable devices found.\n");
            else
                fprintf (stderr, "Error: Device \"%s\" not found; use --list to list the calibratable input devices.\n", pre_device);
            exit(1);
        }

        if (verbose)
        {
            printf("DEBUG: Selected device: %s\n", device_name);
        }

        builder->setAxys(device_axys);
        builder->setDevice_name(device_name);
        builder->setDevice_id(device_id);
    }

    // override min/max XY from command line ?
    if (precalib)
    {
        if (pre_axys.x.min != -1)
            device_axys.x.min = pre_axys.x.min;
        if (pre_axys.x.max != -1)
            device_axys.x.max = pre_axys.x.max;
        if (pre_axys.y.min != -1)
            device_axys.y.min = pre_axys.y.min;
        if (pre_axys.y.max != -1)
            device_axys.y.max = pre_axys.y.max;

        if (verbose)
        {
            printf("DEBUG: Setting precalibration: %i, %i, %i, %i\n",
                device_axys.x.min, device_axys.x.max,
                device_axys.y.min, device_axys.y.max);
        }
        builder->setAxys(device_axys);
    }

// Different device/driver, different ways to apply the calibration values
//    try
//    {
//        // try Usbtouchscreen driver
//        return new CalibratorUsbtouchscreen(device_name, device_axys,
//            thr_misclick, thr_doubleclick, output_type, geometry,
//            use_timeout, output_filename, testMode);
//    }
//    catch(WrongCalibratorException& x)
//    {
//        if (verbose)
//            printf("DEBUG: Not usbtouchscreen calibrator: %s\n", x.what());
//    }

    try
    {        
        if(builder->getCrtc().size() > 0)
        {
            return std::make_shared<CalibratorEvdev>(builder, false);
        }
        else
        {
            return std::make_shared<CalibratorEvdev>(builder);
        }
    }
    catch(WrongCalibratorException& x)
    {
        if (verbose)
            printf("DEBUG: Not evdev calibrator: %s\n", x.what());
    }

    // lastly, presume a standard Xorg driver (evtouch, mutouch, ...)
    return std::make_shared<CalibratorXorgPrint>(builder);
}

void Calibrator::Init()
{
}
