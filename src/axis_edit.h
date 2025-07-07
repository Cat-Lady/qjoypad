#ifndef QJOYPAD_AXIS_EDIT_H
#define QJOYPAD_AXIS_EDIT_H

// This header defines the AxisEdit dialog for editing joystick axis settings.

#include "axis.h"
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Custom widgets for key selection and slider
#include "joyslider.h"
#include "keycode.h"

class AxisEdit : public QDialog {
    Q_OBJECT
public:
    // Constructor takes a pointer to the Axis being edited
    AxisEdit(Axis* ax);

    // Show the dialog (modal)
    void show();

    // Update the slider state to reflect current axis state
    void setState(int val);

protected slots:
    // Slot called when gradient mode selection changes
    void gradientChanged(int index);

    // Slot called when axis mode selection changes
    void modeChanged(int index);

    // Slot called when transfer curve selection changes
    void transferCurveChanged(int index);

    // Slot called when throttle selection changes
    void throttleChanged(int index);

    // Slot called when the user accepts the dialog
    void accept();

protected:
    Axis* axis;               // The Axis being edited

    // Combo box for choosing interpretation mode (ZeroOne, Gradient, Absolute)
    QComboBox* chkGradient;

    // Combo box for choosing axis mode (Keyboard, Mouse directions, KeyboardAndMouse)
    QComboBox* cmbMode;

    // Combo box for choosing throttle mode
    QComboBox* cmbThrottle;

    // Combo box for choosing mouse transfer curve
    QComboBox* cmbTransferCurve;

    QFrame* mouseBox;         // Frame holding mouse-related controls
    QFrame* keyBox;           // Frame holding keyboard-related controls

    QSpinBox* spinSpeed;      // Mouse speed setting
    QLabel* lblSensitivity;   // Label for mouse sensitivity
    QDoubleSpinBox* spinSensitivity; // Mouse sensitivity setting

    KeyButton* btnNeg;        // Button for negative key/mouse selection
    KeyButton* btnPos;        // Button for positive key/mouse selection

    JoySlider* slider;        // Custom slider for dead zone and extreme zone
};

#endif
