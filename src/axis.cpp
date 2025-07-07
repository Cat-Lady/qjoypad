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
        QString word = *it;

        // Remove colons at the end of tokens (e.g. "axis", "3:")
        if (word.endsWith(":")) {
            word.chop(1);
        }

        if (word == "maxspeed") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXMOUSESPEED) maxSpeed = val;
            else return false;
        }
        else if (word == "dzone") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= JOYMAX) dZone = val;
            else return false;
        }
        else if (word == "xzone") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= JOYMAX) xZone = val;
            else return false;
        }
        else if (word == "tcurve") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= PowerFunction) transferCurve = val;
            else return false;
        }
        else if (word == "sens") {
            ++it;
            if (it == words.end()) return false;
            fval = (*it).toFloat(&ok);
            if (ok && fval >= SENSITIVITY_MIN && fval <= SENSITIVITY_MAX)
                sensitivity = fval;
            else return false;
        }
        else if (word == "+key") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) pkeycode = val;
            else return false;
        }
        else if (word == "-key") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) nkeycode = val;
            else return false;
        }
        else if (word == "+mouse") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) {
                puseMouse = true;
                pkeycode = val;
            }
            else return false;
        }
        else if (word == "-mouse") {
            ++it;
            if (it == words.end()) return false;
            val = (*it).toInt(&ok);
            if (ok && val >= 0 && val <= MAXKEY) {
                nuseMouse = true;
                nkeycode = val;
            }
            else return false;
        }
        else if (word == "zeroone") {
            interpretation = ZeroOne;
            gradient = false;
            absolute = false;
        }
        else if (word == "absolute") {
            interpretation = AbsolutePos;
            gradient = true;
            absolute = true;
        }
        else if (word == "gradient") {
            interpretation = Gradient;
            gradient = true;
            absolute = false;
        }
        else if (word == "throttle+") {
            throttle = 1;
        }
        else if (word == "throttle-") {
            throttle = -1;
        }
        else if (word == "mouse+v") {
            mode = MousePosVert;
        }
        else if (word == "mouse-v") {
            mode = MouseNegVert;
        }
        else if (word == "mouse+h") {
            mode = MousePosHor;
        }
        else if (word == "mouse-h") {
            mode = MouseNegHor;
        }
        else if (word == "keyboardandmousehor") {
            mode = KeyboardAndMouseHor;
        }
        else if (word == "keyboardandmousevert") {
            mode = KeyboardAndMouseVert;
        }
        else if (word == "keyboardandmousehorrev") {
            mode = KeyboardAndMouseHorRev;
        }
        else if (word == "keyboardandmousevertrev") {
            mode = KeyboardAndMouseVertRev;
        }
        else {
            // Unknown word - instead of returning false, just ignore and continue
            // You may also output debug info if you want:
            // qDebug() << "Unrecognized token in axis config:" << word;
        }
    }

    adjustGradient();

    return true;
}


void Axis::timerCalled() {
    timerTick(++tick);
}

void Axis::write(QTextStream &stream) {
    // write regular axis parameters:
    stream << "Axis " << (index + 1) << ": ";

    switch (interpretation) {
    case ZeroOne:
        stream << "ZeroOne, ";
        break;
    case Gradient:
        stream << "Gradient, ";
        break;
    case AbsolutePos:
        stream << "Absolute, ";
        break;
    }

    stream << "dZone " << dZone << ", "
           << "xZone " << xZone << ", "
           << "maxSpeed " << maxSpeed << ", "
           << "tCurve " << transferCurve;

    // write keys and mode if applicable

    // Write keys +key / -key or +mouse / -mouse for keyboard and keyboard+mouse modes
    if (mode == Keyboard ||
        mode == KeyboardAndMouseHor ||
        mode == KeyboardAndMouseVert ||
        mode == KeyboardAndMouseHorRev ||
        mode == KeyboardAndMouseVertRev) {
        
        stream << ", " 
               << (puseMouse ? "+mouse " : "+key ") << pkeycode << ", "
               << (nuseMouse ? "-mouse " : "-key ") << nkeycode;
    }

    // Write mode as name (for easier reading)
    switch (mode) {
        case Keyboard:
            stream << ", keyboard";
            break;
        case MousePosVert:
            stream << ", mouseposvert";
            break;
        case MouseNegVert:
            stream << ", mousenegvert";
            break;
        case MousePosHor:
            stream << ", mouseposhor";
            break;
        case MouseNegHor:
            stream << ", mouseneghor";
            break;
        case KeyboardAndMouseHor:
            stream << ", keyboardandmousehor";
            break;
        case KeyboardAndMouseVert:
            stream << ", keyboardandmousevert";
            break;
        case KeyboardAndMouseHorRev:
            stream << ", keyboardandmousehorrev";
            break;
        case KeyboardAndMouseVertRev:
            stream << ", keyboardandmousevertrev";
            break;
    }

    stream << "\n";
}


void Axis::release() {
    if (isDown) {
        move(false);
        isDown = false;
    }
}

void Axis::jsevent(int value) {
    if (throttle == 0)
        state = value;
    else if (throttle == -1)
        state = (value + JOYMIN) / 2;
    else
        state = (value + JOYMAX) / 2;

    if (isOn && abs(state) <= dZone) {
        isOn = false;
        if (gradient) {
            duration = 0;
            release();
            timer.stop();
            disconnect(&timer, SIGNAL(timeout()), 0, 0);
            tick = 0;
        }
    }
    else if (!isOn && abs(state) >= dZone) {
        isOn = true;
        if (gradient) {
            duration = (abs(state) * FREQ) / JOYMAX;
            connect(&timer, SIGNAL(timeout()), this, SLOT(timerCalled()));
            timer.start(MSEC);
        }
    }
    else return;

    if (!gradient) {
        move(isOn);
    }
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
    return (interpretation == ZeroOne) &&
           (gradient == false) &&
           (absolute == false) &&
           (throttle == 0) &&
           (maxSpeed == 100) &&
           (dZone == DZONE) &&
           (xZone == XZONE) &&
           (mode == Keyboard) &&
           (pkeycode == 0) &&
           (nkeycode == 0) &&
           (puseMouse == false) &&
           (nuseMouse == false);
}

QString Axis::getName() {
    return tr("Axis %1").arg(index + 1);
}

bool Axis::inDeadZone(int val) {
    int value;
    if (throttle == 0)
        value = val;
    else if (throttle == -1)
        value = (val + JOYMIN) / 2;
    else
        value = (val + JOYMAX) / 2;
    return (abs(value) < dZone);
}

QString Axis::status() {
    QString label;
    if (mode == Keyboard) {
        if (throttle == 0) {
            if (puseMouse != nuseMouse) {
                label = tr("KEYBOARD/MOUSE");
            }
            else if (puseMouse) {
                label = tr("MOUSE");
            }
            else {
                label = tr("KEYBOARD");
            }
        }
        else {
            label = tr("THROTTLE");
        }
    }
    else {
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
    }
    else {
        nkeycode = value;
        nuseMouse = useMouse;
    }
}

void Axis::timerTick(int tick) {
    if (isOn) {
        if (mode == Keyboard) {
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
        }
        else {
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

    // Support keyboard and mouse simultaneously in KeyboardAndMouse* modes
//    bool mouseMoveActive = false;
    bool keyboardPress = false;

    if (
        mode == Keyboard ||
        mode == KeyboardAndMouseHor ||
        mode == KeyboardAndMouseVert ||
        mode == KeyboardAndMouseHorRev ||
        mode == KeyboardAndMouseVertRev
    ) {
        // handle key presses:
        if (isDown == press && mode == Keyboard) return; // prevent repeats
        if (mode == Keyboard) {
            if (state != 0) {
                useMouse = (state > 0) ? puseMouse : nuseMouse;
            }
            if (press) {
                e.type = useMouse ? FakeEvent::MouseDown : FakeEvent::KeyDown;
                downkey = (state > 0) ? pkeycode : nkeycode;
            }
            else {
                e.type = useMouse ? FakeEvent::MouseUp : FakeEvent::KeyUp;
            }
            e.keycode = downkey;
            sendevent(e);
            isDown = press;
            return;
        }
        else {
            // KeyboardAndMouse* modes - mouse movement + keys
            keyboardPress = press && (abs(state) >= xZone);

            // handle keys:
            if (keyboardPress) {
                // if key not pressed, press it
                if (!isDown) {
                    e.type = ( (state > 0) ? (puseMouse ? FakeEvent::MouseDown : FakeEvent::KeyDown) :
                              (nuseMouse ? FakeEvent::MouseDown : FakeEvent::KeyDown));
                    downkey = (state > 0) ? pkeycode : nkeycode;
                    e.keycode = downkey;
                    sendevent(e);
                    isDown = true;
                }
            }
            else {
                // if key should be released
                if (isDown) {
                    e.type = ( (state > 0) ? (puseMouse ? FakeEvent::MouseUp : FakeEvent::KeyUp) :
                              (nuseMouse ? FakeEvent::MouseUp : FakeEvent::KeyUp));
                    e.keycode = downkey;
                    sendevent(e);
                    isDown = false;
                }
            }

            // handle mouse:

            int dist = 0;
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
                    default:
                        fdist = u;
                    }
                }
                fdist *= maxSpeed;
                if (state < 0) fdist = -fdist;
                sumDist += fdist;
                dist = int(sumDist);
                sumDist -= dist;
            }
            else {
                dist = (state >= 0) ? maxSpeed : -maxSpeed;
            }

            e.type = FakeEvent::MouseMove;
            e.move.x = 0;
            e.move.y = 0;

            // Set mouse movement depending on mode:
            switch (mode) {
                case MousePosVert:
                    e.move.y = dist;
                    break;
                case MouseNegVert:
                    e.move.y = -dist;
                    break;
                case MousePosHor:
                    e.move.x = dist;
                    break;
                case MouseNegHor:
                    e.move.x = -dist;
                    break;
                    case KeyboardAndMouseHor:
                        e.move.x = dist;
                        break;
                    case KeyboardAndMouseHorRev:
                        e.move.x = -dist;
                        break;
                    case KeyboardAndMouseVert:
                        e.move.y = dist;
                        break;
                    case KeyboardAndMouseVertRev:
                        e.move.y = -dist;
                        break;

                default:
                    break;
            }

            if (e.move.x != 0 || e.move.y != 0) {
                sendevent(e);
//                mouseMoveActive = true;
            }

            // Keyboard was handled above (keyboardPress/isDown)

            return;
        }
    }
    else if (mode == MousePosVert || mode == MouseNegVert || mode == MousePosHor || mode == MouseNegHor) {
        // Old modes, mouse only
        int dist = 0;
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
                default:
                    fdist = u;
                }
            }
            fdist *= maxSpeed;
            if (state < 0) fdist = -fdist;
            sumDist += fdist;
            dist = int(sumDist);
            sumDist -= dist;
        }
        else {
            dist = (state >= 0) ? maxSpeed : -maxSpeed;
        }

        FakeEvent e;
        e.type = FakeEvent::MouseMove;
        e.move.x = 0;
        e.move.y = 0;

        switch (mode) {
            case MousePosVert: e.move.y = dist; break;
            case MouseNegVert: e.move.y = -dist; break;
            case MousePosHor: e.move.x = dist; break;
            case MouseNegHor: e.move.x = -dist; break;
            default: break;
        }

        if (e.move.x != 0 || e.move.y != 0)
            sendevent(e);
    }
    else if (mode == Keyboard) {
        // Old keyboard-only modes
        if (isDown == press) return;
        isDown = press;
        FakeEvent e;
        e.type = (useMouse ? (press ? FakeEvent::MouseDown : FakeEvent::MouseUp) : (press ? FakeEvent::KeyDown : FakeEvent::KeyUp));
        e.keycode = (state > 0) ? pkeycode : nkeycode;
        sendevent(e);
    }
}
