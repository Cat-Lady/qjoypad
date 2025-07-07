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

#define DZONE 3000
#define XZONE 30000

class Axis : public QObject {
    Q_OBJECT

public:
    enum Interpretation { ZeroOne, Gradient, AbsolutePos };

    // Added new modes for extended axis handling:
    enum Mode {
        Keyboard,
        MousePosVert,
        MouseNegVert,
        MousePosHor,
        MouseNegHor,
        KeyboardAndMouseHor,     // new mode: horizontal mouse movement + keys
        KeyboardAndMouseVert,    // new mode: vertical mouse movement + keys
        KeyboardAndMouseHorRev,
        KeyboardAndMouseVertRev,
    };

    enum TransferCurve { Linear, Quadratic, Cubic, QuadraticExtreme, PowerFunction };

    friend class AxisEdit;

    Axis(int i, QObject *parent = 0);
    ~Axis();

    bool read(QTextStream &stream);
    void write(QTextStream &stream);
    void release();
    void jsevent(int value);
    void toDefault();
    bool isDefault();
    QString getName();
    bool inDeadZone(int val);
    QString status();

    void setKey(bool positive, int value);
    void setKey(bool useMouse, bool positive, int value);

    void timerTick(int tick);
    void adjustGradient();
    int axisIndex() const { return index; }

protected:
    int tick;
    bool isOn;
    int index;
    virtual void move(bool press);
    bool isDown;
    bool useMouse;

    float inverseRange;

    Interpretation interpretation;
    bool gradient;
    bool absolute;
    int maxSpeed;
    unsigned int transferCurve;
    float sensitivity;
    int throttle;
    int dZone;
    int xZone;
    double sumDist;
    Mode mode;
    int pkeycode;
    int nkeycode;
    bool puseMouse;
    bool nuseMouse;
    int downkey;
    int state;
    int duration;
    QTimer timer;

protected slots:
    void timerCalled();
};

#endif
