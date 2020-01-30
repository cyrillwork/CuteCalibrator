# Cute calibrator for Linux (based on xinput_calibrator)

For build (You need gcc >= 4.7, cmake, boost, X11_dev, X11_Xi_Lib, gtkmm)
run ./build.sh

For help
./xinput_calibrator --help

For run
./xinput_calibrator --path ./pic/

Improvements compared to original xinput_calibtator
1. A nicer design:)
2. Add X and Y to option geometry. This is used to calibrate multi-touch system.
3. Add regim touch detected (option touch_id). This is used to determinate id touch in multi-touch system.
4. Add test mode for checking current calibration.

Deterioration
1. Work only with evdev.
