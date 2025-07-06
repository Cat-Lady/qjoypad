#include "axis.h"
#include "event.h"
#include "time.h"

#define sqr(a) ((a)*(a))
#define cub(a) ((a)*(a)*(a))
#define clamp(a, a_low, a_high) ((a) < (a_low) ? (a_low) : (a) > (a_high) ? (a_high) : (a))

Axis::Axis(int i, QObject *parent) : QObject(parent) {
    index = i;
    isOn = false;
    isDown = false;
    useMouse = false;
    state = 0;
    interpretation = ZeroOne;
    gradient = false;
    absolute = false;
    toDefault();
    tick = 0;
}

Axis::~Axis() {
    release();
}

bool Axis::read(QTextStream &stream) {
    QString input = stream.readLine().toLower();
    QRegExp regex("[\\s,]+");
    QStringList words = input.split(regex);

    bool ok;
    int val;
    float fval;

    for (QStringList::Iterator it = words.begin(); it != words.end(); ++it) {
        if (*it == "maxspeed") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXMOUSESPEED) maxSpeed = val;
            else return false;
        } else if (*it == "dzone") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= JOYMAX) dZone = val;
            else return false;
        } else if (*it == "xzone") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= JOYMAX) xZone = val;
            else return false;
        } else if (*it == "tcurve") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= PowerFunction) transferCurve = val;
            else return false;
        } else if (*it == "sens") {
            if (++it == words.end()) return false;
            fval = (*it).toFloat(&ok);
            if (ok && fval >= SENSITIVITY_MIN && fval <= SENSITIVITY_MAX) sensitivity = fval;
            else return false;
        } else if (*it == "+key") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) pkeycode = val;
            else return false;
        } else if (*it == "-key") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) nkeycode = val;
            else return false;
        } else if (*it == "+mouse") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) { puseMouse = true; pkeycode = val; }
            else return false;
        } else if (*it == "-mouse") {
            if (++it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) { nuseMouse = true; nkeycode = val; }
            else return false;
        } else if (*it == "zeroone") {
            interpretation = ZeroOne; gradient = false; absolute = false;
        } else if (*it == "absolute") {
            interpretation = AbsolutePos; gradient = true; absolute = true;
        } else if (*it == "gradient") {
            interpretation = Gradient; gradient = true; absolute = false;
        } else if (*it == "throttle+") {
            throttle = 1;
        } else if (*it == "throttle-") {
            throttle = -1;
        } else if (*it == "mouse+v") {
            mode = MousePosVert;
        } else if (*it == "mouse-v") {
            mode = MouseNegVert;
        } else if (*it == "mouse+h") {
            mode = MousePosHor;
        } else if (*it == "mouse-h") {
            mode = MouseNegHor;
        } else if (*it == "keyboardandmouse") {
            mode = KeyboardAndMouse;
        }
    }

    adjustGradient();
    return true;
}

void Axis::write(QTextStream &stream) {
    stream << "\tAxis " << (index + 1) << ": ";
    stream << ((interpretation == ZeroOne) ? "ZeroOne" :
              (interpretation == Gradient) ? "Gradient" : "Absolute") << ", ";
    if (throttle > 0) stream << "throttle+, ";
    else if (throttle < 0) stream << "throttle-, ";
    if (dZone != DZONE) stream << "dZone " << dZone << ", ";
    if (xZone != XZONE) stream << "xZone " << xZone << ", ";
    if (mode == Keyboard || mode == KeyboardAndMouse) {
        stream << (puseMouse ? "+mouse " : "+key ") << pkeycode << ", "
               << (nuseMouse ? "-mouse " : "-key ") << nkeycode;
        if (mode == KeyboardAndMouse)
            stream << ", keyboardandmouse";
        stream << "\n";
    } else {
        if (gradient) stream << "maxSpeed " << maxSpeed << ", ";
        if (transferCurve != Quadratic) stream << "tCurve " << transferCurve << ", ";
        if (sensitivity != 1.0F) stream << "sens " << sensitivity << ", ";
        stream << "mouse";
        if (mode == MousePosVert) stream << "+v\n";
        else if (mode == MouseNegVert) stream << "-v\n";
        else if (mode == MousePosHor) stream << "+h\n";
        else if (mode == MouseNegHor) stream << "-h\n";
    }
}

void Axis::release() {
    if (isDown) {
        move(false);
        isDown = false;
    }
}

void Axis::jsevent(int value) {
    state = (throttle == 0) ? value :
            (throttle == -1) ? (value + JOYMIN) / 2 :
                               (value + JOYMAX) / 2;

    if (isOn && abs(state) <= dZone) {
        isOn = false;
        if (gradient) {
            duration = 0;
            release();
            timer.stop();
            disconnect(&timer, SIGNAL(timeout()), 0, 0);
            tick = 0;
        }
    } else if (!isOn && abs(state) >= dZone) {
        isOn = true;
        if (gradient) {
            duration = (abs(state) * FREQ) / JOYMAX;
            connect(&timer, SIGNAL(timeout()), this, SLOT(timerCalled()));
            timer.start(MSEC);
        }
    } else return;

    if (!gradient)
        move(isOn);
}

void Axis::toDefault() {
    release();
    interpretation = ZeroOne;
    gradient = false;
    absolute = false;
    throttle = 0;
    maxSpeed = 100;
    transferCurve = Quadratic;
    sensitivity = 1.0F;
    dZone = DZONE;
    tick = 0;
    xZone = XZONE;
    mode = Keyboard;
    pkeycode = 0;
    nkeycode = 0;
    puseMouse = false;
    nuseMouse = false;
    downkey = 0;
    state = 0;
    adjustGradient();
}

bool Axis::isDefault() {
    return interpretation == ZeroOne &&
           !gradient &&
           !absolute &&
           throttle == 0 &&
           maxSpeed == 100 &&
           dZone == DZONE &&
           xZone == XZONE &&
           mode == Keyboard &&
           pkeycode == 0 &&
           nkeycode == 0 &&
           !puseMouse &&
           !nuseMouse;
}

QString Axis::getName() {
    return tr("Axis %1").arg(index + 1);
}

bool Axis::inDeadZone(int val) {
    int value = (throttle == 0) ? val :
                (throttle == -1) ? (val + JOYMIN) / 2 :
                                   (val + JOYMAX) / 2;
    return (abs(value) < dZone);
}

QString Axis::status() {
    QString label;
    if (mode == Keyboard || mode == KeyboardAndMouse) {
        if (throttle == 0) {
            if (puseMouse != nuseMouse) label = tr("KEYBOARD/MOUSE");
            else if (puseMouse) label = tr("MOUSE");
            else label = tr("KEYBOARD");
        } else {
            label = tr("THROTTLE");
        }
    } else {
        label = tr("MOUSE");
    }
    return QString("%1 : [%2]").arg(getName(), label);
}

void Axis::setKey(bool positive, int value) {
    setKey(false, positive, value);
}

void Axis::setKey(bool useMouse, bool positive, int value) {
    if (positive) {
        pkeycode = value;
        puseMouse = useMouse;
    } else {
        nkeycode = value;
        nuseMouse = useMouse;
    }
}

void Axis::timerTick(int tick) {
    if (isOn) {
        if (mode == Keyboard || mode == KeyboardAndMouse) {
            if (tick % FREQ == 0) {
                if (duration == FREQ) {
                    if (!isDown) move(true);
                    duration = (abs(state) * FREQ) / JOYMAX;
                    return;
                }
                move(true);
            }
            if (tick % FREQ == duration) {
                move(false);
                duration = (abs(state) * FREQ) / JOYMAX;
            }
        } else {
            move(true);
        }
    }
}

void Axis::adjustGradient() {
    inverseRange = 1.0F / (xZone - dZone);
    sumDist = 0;
}

void Axis::move(bool press) {
    FakeEvent e;

    const bool keyboardMode = (mode == Keyboard || mode == KeyboardAndMouse);
    const bool mouseMode = (mode != Keyboard && mode != KeyboardAndMouse) || (mode == KeyboardAndMouse && press);

    if (keyboardMode) {
        if (isDown == press) return;
        isDown = press;

        if (state != 0)
            useMouse = (state > 0) ? puseMouse : nuseMouse;

        if (press) {
            e.type = useMouse ? FakeEvent::MouseDown : FakeEvent::KeyDown;
            downkey = (state > 0) ? pkeycode : nkeycode;
        } else {
            e.type = useMouse ? FakeEvent::MouseUp : FakeEvent::KeyUp;
        }

        e.keycode = downkey;
        sendevent(e);
    }

    if (mouseMode) {
        if (!press) return;

        int dist;
        if (gradient) {
            const int absState = abs(state);
            float fdist;
            if (absState >= xZone) fdist = 1.0F;
            else if (absState <= dZone) fdist = 0.0F;
            else {
                const float u = inverseRange * (absState - dZone);
                switch (transferCurve) {
                    case Quadratic: fdist = sqr(u); break;
                    case Cubic: fdist = cub(u); break;
                    case QuadraticExtreme:
                        fdist = sqr(u);
                        if (u >= 0.95F) fdist *= 1.5F;
                        break;
                    case PowerFunction:
                        fdist = clamp(powf(u, 1.0F / clamp(sensitivity, 1e-8F, 1e+3F)), 0.0F, 1.0F);
                        break;
                    default: fdist = u;
                }
            }
            fdist *= maxSpeed;
            if (state < 0) fdist = -fdist;
            sumDist += fdist;
            dist = static_cast<int>(rint(sumDist));
            sumDist -= dist;
        } else {
            dist = maxSpeed;
        }

        e.type = absolute ? FakeEvent::MouseMoveAbsolute : FakeEvent::MouseMove;
        if (mode == MousePosVert || mode == KeyboardAndMouse) {
            e.move.x = 0;
            e.move.y = dist;
        } else if (mode == MouseNegVert) {
            e.move.x = 0;
            e.move.y = -dist;
        } else if (mode == MousePosHor) {
            e.move.x = dist;
            e.move.y = 0;
        } else if (mode == MouseNegHor) {
            e.move.x = -dist;
            e.move.y = 0;
        }

        sendevent(e);
    }
}

void Axis::timerCalled() {
    timerTick(++tick);
}
