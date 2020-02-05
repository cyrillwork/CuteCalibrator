# Cute calibrator for Linux (based on xinput_calibrator)

For build (You need gcc >= 4.7, cmake, boost, X11_dev, X11_Xi_Lib, gtkmm)
run ./build.sh

For help
./xinput_calibrator --help

For run
./xinput_calibrator --path ./pic/

Improvements compared to original xinput_calibtator
1. A nicer design:)
2. X and Y to option geometry. This is used to calibrate multi-touch system.
3. Mode touch detected (option touch_id). This is used to determinate id touch in multi-touch system.
4. Test mode for checking current calibration.
5. Mode that combines "touch_id" and calibration (option crtc). In this mode, the "xinput map-to-output id output" command is automatically executed after definition touch_id.

Deterioration
1. Work only with evdev.
