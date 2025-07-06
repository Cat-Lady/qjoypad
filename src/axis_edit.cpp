#include "config.h"
#include "axis_edit.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

AxisEdit::AxisEdit(Axis* ax)
    : QDialog() {
    // Initialize the dialog and populate with current axis settings
    axis = ax;
    setWindowTitle("Set " + axis->getName());
    setWindowIcon(QPixmap(QJOYPAD_ICON24));

    // Main vertical layout for the dialog
    QVBoxLayout* v = new QVBoxLayout(this);
    v->setMargin(5);
    v->setSpacing(5);

    // Layouts for gradient and mode selection controls
    QHBoxLayout* h = new QHBoxLayout();
    QVBoxLayout* v2 = new QVBoxLayout();
    v2->setMargin(5);
    v2->setSpacing(5);

    // Interpretation mode combo box (ZeroOne, Gradient, Absolute)
    chkGradient = new QComboBox(this);
    chkGradient->insertItem((int) Axis::ZeroOne, tr("Use 0 or max always"), Qt::DisplayRole);
    chkGradient->insertItem((int) Axis::Gradient, tr("Relative movement (previously gradient)"), Qt::DisplayRole);
    chkGradient->insertItem((int) Axis::AbsolutePos, tr("Absolute movement (direct position)"), Qt::DisplayRole);
    chkGradient->setCurrentIndex(axis->interpretation);
    connect(chkGradient, SIGNAL(activated(int)), this, SLOT(gradientChanged(int)));
    v2->addWidget(chkGradient);

    // Axis mode combo box with added KeyboardAndMouse mode
    cmbMode = new QComboBox(this);
    cmbMode->insertItem((int) Axis::Keyboard, tr("Keyboard/Mouse Button"), Qt::DisplayRole);
    cmbMode->insertItem((int) Axis::MousePosVert, tr("Mouse (Vert.)"), Qt::DisplayRole);
    cmbMode->insertItem((int) Axis::MouseNegVert, tr("Mouse (Vert. Rev.)"), Qt::DisplayRole);
    cmbMode->insertItem((int) Axis::MousePosHor, tr("Mouse (Hor.)"), Qt::DisplayRole);
    cmbMode->insertItem((int) Axis::MouseNegHor, tr("Mouse (Hor. Rev.)"), Qt::DisplayRole);
    cmbMode->insertItem((int) Axis::KeyboardAndMouse, tr("Keyboard + Mouse"), Qt::DisplayRole);  // New combined mode
    cmbMode->setCurrentIndex(axis->mode);
    connect(cmbMode, SIGNAL(activated(int)), this, SLOT(modeChanged(int)));
    v2->addWidget(cmbMode);

    // Transfer curve combo box for mouse movement sensitivity curve
    cmbTransferCurve = new QComboBox(this);
    cmbTransferCurve->insertItem(Axis::Linear, tr("Linear"), Qt::DisplayRole);
    cmbTransferCurve->insertItem(Axis::Quadratic, tr("Quadratic"), Qt::DisplayRole);
    cmbTransferCurve->insertItem(Axis::Cubic, tr("Cubic"), Qt::DisplayRole);
    cmbTransferCurve->insertItem(Axis::QuadraticExtreme, tr("Quadratic Extreme"), Qt::DisplayRole);
    cmbTransferCurve->insertItem(Axis::PowerFunction, tr("Power Function"), Qt::DisplayRole);
    cmbTransferCurve->setCurrentIndex(axis->transferCurve);
    cmbTransferCurve->setEnabled(axis->gradient);
    connect(cmbTransferCurve, SIGNAL(activated(int)), this, SLOT(transferCurveChanged(int)));
    v2->addWidget(cmbTransferCurve);

    h->addLayout(v2);

    // Mouse control panel frame
    mouseBox = new QFrame(this);
    mouseBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
    v2 = new QVBoxLayout(mouseBox);
    v2->setSpacing(5);
    v2->setMargin(5);

    QLabel* mouseLabel = new QLabel(tr("&Mouse Speed"), mouseBox);
    v2->addWidget(mouseLabel);

    spinSpeed = new QSpinBox(mouseBox);
    spinSpeed->setRange(0, MAXMOUSESPEED);
    spinSpeed->setSingleStep(1);
    spinSpeed->setValue(axis->maxSpeed);
    v2->addWidget(spinSpeed);

    lblSensitivity = new QLabel(tr("&Sensitivity"), mouseBox);
    v2->addWidget(lblSensitivity);

    spinSensitivity = new QDoubleSpinBox(mouseBox);
    spinSensitivity->setRange(1e-3F, 1e+3F);
    spinSensitivity->setSingleStep(0.10);
    spinSensitivity->setValue(axis->sensitivity);
    v2->addWidget(spinSensitivity);

    h->addWidget(mouseBox);
    mouseLabel->setBuddy(spinSpeed);
    lblSensitivity->setBuddy(spinSensitivity);

    v->addLayout(h);

    // Slider for dead zone and extreme zone
    slider = new JoySlider(axis->dZone, axis->xZone, axis->state, this);
    v->addWidget(slider);

    // Keyboard control panel frame
    keyBox = new QFrame(this);
    keyBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
    h = new QHBoxLayout(keyBox);
    h->setSpacing(5);
    h->setMargin(5);

    // Negative key/mouse button selector
    btnNeg = new KeyButton(axis->getName(), axis->nkeycode, keyBox, true, axis->nuseMouse);

    // Throttle mode combo box
    cmbThrottle = new QComboBox(keyBox);
    cmbThrottle->insertItem(0, tr("Neg. Throttle"), Qt::DisplayRole);
    cmbThrottle->insertItem(1, tr("No Throttle"), Qt::DisplayRole);
    cmbThrottle->insertItem(2, tr("Pos. Throttle"), Qt::DisplayRole);
    cmbThrottle->setCurrentIndex(axis->throttle + 1);
    connect(cmbThrottle, SIGNAL(activated(int)), this, SLOT(throttleChanged(int)));

    // Positive key/mouse button selector
    btnPos = new KeyButton(axis->getName(), axis->pkeycode, keyBox, true, axis->puseMouse);

    h->addWidget(btnNeg);
    h->addWidget(cmbThrottle);
    h->addWidget(btnPos);

    v->addWidget(keyBox);

    // Dialog buttons: OK and Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    v->addWidget(buttonBox);

    // Initialize widget states based on current axis settings
    gradientChanged(axis->interpretation);
    modeChanged(axis->mode);
    transferCurveChanged(axis->transferCurve);
    throttleChanged(axis->throttle + 1);
}

void AxisEdit::show() {
    QDialog::show();
    setFixedSize(size());
}

void AxisEdit::setState(int val) {
    slider->setValue(val);
}

void AxisEdit::gradientChanged(int index) {
    bool gradient = index != Axis::ZeroOne;
    cmbTransferCurve->setEnabled(gradient);
    if (gradient) {
        transferCurveChanged(axis->transferCurve);
    } else {
        lblSensitivity->setEnabled(false);
        spinSensitivity->setEnabled(false);
    }
}

void AxisEdit::modeChanged(int index) {
    if (index == Axis::Keyboard) {
        // Keyboard only mode: disable mouse controls, enable keys
        mouseBox->setEnabled(false);
        keyBox->setEnabled(true);
    }
    else if (index == Axis::KeyboardAndMouse) {
        // New combined mode: enable both mouse and keyboard controls
        mouseBox->setEnabled(true);
        keyBox->setEnabled(true);
        if ((Axis::Interpretation)chkGradient->currentIndex() != Axis::ZeroOne) {
            cmbTransferCurve->setEnabled(true);
            transferCurveChanged(axis->transferCurve);
        }
    }
    else {
        // Mouse only modes: enable mouse controls, disable keys
        mouseBox->setEnabled(true);
        keyBox->setEnabled(false);
        if ((Axis::Interpretation)chkGradient->currentIndex() != Axis::ZeroOne) {
            cmbTransferCurve->setEnabled(true);
            transferCurveChanged(axis->transferCurve);
        }
    }
}

void AxisEdit::transferCurveChanged(int index) {
    if (index == Axis::PowerFunction) {
        lblSensitivity->setEnabled(true);
        spinSensitivity->setEnabled(true);
    } else {
        lblSensitivity->setEnabled(false);
        spinSensitivity->setEnabled(false);
    }
}

void AxisEdit::throttleChanged(int index) {
    switch (index) {
        case 0:
            btnNeg->setEnabled(true);
            btnPos->setEnabled(false);
            break;
        case 1:
            btnNeg->setEnabled(true);
            btnPos->setEnabled(true);
            break;
        case 2:
            btnNeg->setEnabled(false);
            btnPos->setEnabled(true);
            break;
    }
    slider->setThrottle(index - 1);
}

void AxisEdit::accept() {
    // Save all widget states back to the Axis object
    axis->interpretation = (Axis::Interpretation)chkGradient->currentIndex();
    axis->gradient = axis->interpretation != Axis::ZeroOne;
    axis->absolute = axis->interpretation == Axis::AbsolutePos;
    axis->maxSpeed = spinSpeed->value();
    axis->transferCurve = (Axis::TransferCurve)cmbTransferCurve->currentIndex();
    axis->sensitivity = spinSensitivity->value();
    axis->throttle = cmbThrottle->currentIndex() - 1;
    axis->dZone = slider->deadZone();
    axis->xZone = slider->xZone();
    axis->mode = (Axis::Mode)cmbMode->currentIndex();
    axis->pkeycode = btnPos->getValue();
    axis->nkeycode = btnNeg->getValue();
    axis->puseMouse = btnPos->choseMouse();
    axis->nuseMouse = btnNeg->choseMouse();
    axis->adjustGradient();

    QDialog::accept();
}
