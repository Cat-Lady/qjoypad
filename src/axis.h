#ifndef QJOYPAD_AXIS_H
#define QJOYPAD_AXIS_H

#include <stdlib.h>
#include <math.h>

#include <QTimer>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include "constant.h"
#include "error.h"

// Default and arbitrary values for dZone and xZone
#define DZONE 3000
#define XZONE 30000

// Represents one joystick axis
class Axis : public QObject {
    Q_OBJECT

    // Interpretation modes for how the axis input is processed
    enum Interpretation { ZeroOne, Gradient, AbsolutePos };

    // Behavior mode: what kind of output this axis produces
    enum Mode {
        Keyboard,
        MousePosVert,
        MouseNegVert,
        MousePosHor,
        MouseNegHor,
        KeyboardAndMouse // ← NEW: combined keyboard and mouse mode
    };

    // Axis transfer curve (for mouse movement smoothing, etc.)
    enum TransferCurve {
        Linear,
        Quadratic,
        Cubic,
        QuadraticExtreme,
        PowerFunction
    };

    // Allow AxisEdit class to access internals directly
    friend class AxisEdit;

public:
    Axis(int i, QObject *parent = 0);
    ~Axis();

    // Load axis settings from a stream
    bool read(QTextStream &stream);

    // Save axis settings to a stream
    void write(QTextStream &stream);

    // Reset all outputs to neutral (release keys, stop movement)
    void release();

    // Handle new joystick value from hardware
    void jsevent(int value);

    // Reset to default values
    void toDefault();

    // Check if current state equals default
    bool isDefault();

    // Get name of this axis
    QString getName();

    // Check if a value lies within the dead zone
    bool inDeadZone(int val);

    // Text description of current status (for GUI labels)
    QString status();

    // Set key binding (used by quickset dialog)
    void setKey(bool positive, int value);
    void setKey(bool useMouse, bool positive, int value);

    // Called periodically every MSEC ms (see constant.h)
    void timerTick(int tick);

    // Recalculate the gradient mouse response curve
    void adjustGradient();

    int axisIndex() const { return index; }

protected:
    int tick;

    // Whether the axis is logically 'on' (positive or negative)
    bool isOn;

    // Index of this axis on the joystick
    int index;

    // Send press/release event
    virtual void move(bool press);

    // Whether a key is currently held down
    bool isDown;

    // Whether mouse emulation is currently active
    bool useMouse;

    // Internal value for gradient curve calculation
    float inverseRange;

    // Axis behavior settings
    Interpretation interpretation;
    bool gradient;
    bool absolute;
    int maxSpeed; // 0..MAXMOUSESPEED
    unsigned int transferCurve;
    float sensitivity;
    int throttle; // -1 (nkey), 0 (none), 1 (pkey)
    int dZone;    // Dead zone threshold
    int xZone;    // Extreme zone threshold
    double sumDist;
    Mode mode;    // Keyboard / Mouse / Combined behavior

    // Key bindings
    int pkeycode;     // Positive direction
    int nkeycode;     // Negative direction
    bool puseMouse;   // Use mouse on positive direction
    bool nuseMouse;   // Use mouse on negative direction

    // Currently pressed key
    int downkey;

    // Most recent value from jsevent()
    int state;

    // Key press duration in gradient mode
    int duration;

    QTimer timer;

public slots:
    void timerCalled();
};

#endif // QJOYPAD_AXIS_H
