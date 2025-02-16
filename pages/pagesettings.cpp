/*
    Copyright 2016 - 2020 Benjamin Vedder	benjamin@vedder.se

    This file is part of VESC Tool.

    VESC Tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VESC Tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include "pagesettings.h"
#include "ui_pagesettings.h"
#include <QDebug>
#include <cmath>
#include "utility.h"

PageSettings::PageSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageSettings)
{
    mVesc = nullptr;

#ifdef HAS_GAMEPAD
    mGamepad = nullptr;
    mUseGamepadControl = false;
#endif

    mTimer = new QTimer(this);
    mTimer->start(100);
    connect(mTimer, SIGNAL(timeout()),
            this, SLOT(timerSlot()));

    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);

    if (mSettings.contains("app_scale_factor")) {
        ui->uiScaleBox->setValue(mSettings.value("app_scale_factor").toDouble());
    }

    if (mSettings.contains("app_scale_auto")) {
        ui->uiAutoScaleBox->setChecked(mSettings.value("app_scale_auto").toBool());
    }

    ui->uiScaleBox->setEnabled(!ui->uiAutoScaleBox->isChecked());

#ifdef HAS_GAMEPAD
    auto confAxis = [](QGamepad *gp, QGamepadManager::GamepadAxis axis) {
        if (gp) {
            QGamepadManager::instance()->configureAxis(gp->deviceId(), axis);
        }
    };

    connect(ui->jsConf1Button, &QPushButton::clicked,
            [=]() {confAxis(mGamepad, QGamepadManager::AxisLeftX);});
    connect(ui->jsConf2Button, &QPushButton::clicked,
            [=]() {confAxis(mGamepad, QGamepadManager::AxisLeftY);});
    connect(ui->jsConf3Button, &QPushButton::clicked,
            [=]() {confAxis(mGamepad, QGamepadManager::AxisRightX);});
    connect(ui->jsConf4Button, &QPushButton::clicked,
            [=]() {confAxis(mGamepad, QGamepadManager::AxisRightY);});

    if (mSettings.contains("js_is_configured")) {
        ui->jsConfigOkBox->setChecked(mSettings.value("js_is_configured").toBool());
    }
    if (mSettings.contains("js_is_inverted")) {
        ui->jsInvertedBox->setChecked(mSettings.value("js_is_inverted").toBool());
    }
    if (mSettings.contains("js_is_bidirectional")) {
        ui->jsBidirectionalBox->setChecked(mSettings.value("js_is_bidirectional").toBool());
    }
    if (mSettings.contains("js_axis")) {
        ui->jseAxisBox->setCurrentIndex(mSettings.value("js_axis").toInt());
    }
    if (mSettings.contains("js_control_type")) {
        ui->jsControlTypeBox->setCurrentIndex(mSettings.value("js_control_type").toInt());
    }
    if (mSettings.contains("js_current_min")) {
        ui->jsCurrentMinBox->setValue(mSettings.value("js_current_min").toDouble());
    }
    if (mSettings.contains("js_current_max")) {
        ui->jsCurrentMaxBox->setValue(mSettings.value("js_current_max").toDouble());
    }
    if (mSettings.contains("js_erpm_min")) {
        ui->jsErpmMinBox->setValue(mSettings.value("js_erpm_min").toDouble());
    }
    if (mSettings.contains("js_erpm_max")) {
        ui->jsErpmMaxBox->setValue(mSettings.value("js_erpm_max").toDouble());
    }
    if (mSettings.contains("js_range_min")) {
        ui->jsMinBox->setValue(mSettings.value("js_range_min").toDouble());
    }
    if (mSettings.contains("js_range_max")) {
        ui->jsMaxBox->setValue(mSettings.value("js_range_max").toDouble());
    }

    if (mSettings.contains("js_name")) {
        ui->jsListBox->clear();
        auto gamepads = QGamepadManager::instance()->connectedGamepads();
        for (auto g: gamepads) {
            auto name = QGamepadManager::instance()->gamepadName(g);
            ui->jsListBox->addItem(name, g);
            if (name == mSettings.value("js_name").toString()) {
                if (mGamepad) {
                    mGamepad->deleteLater();
                }
                mGamepad = new QGamepad(g, this);
            }
        }
    }
#endif
}

PageSettings::~PageSettings()
{
#ifdef HAS_GAMEPAD
    mSettings.setValue("js_is_configured", ui->jsConfigOkBox->isChecked());
    mSettings.setValue("js_is_inverted", ui->jsInvertedBox->isChecked());
    mSettings.setValue("js_is_bidirectional", ui->jsBidirectionalBox->isChecked());
    mSettings.setValue("js_axis", ui->jseAxisBox->currentIndex());
    mSettings.setValue("js_control_type", ui->jsControlTypeBox->currentIndex());
    mSettings.setValue("js_current_min", ui->jsCurrentMinBox->value());
    mSettings.setValue("js_current_max", ui->jsCurrentMaxBox->value());
    mSettings.setValue("js_erpm_min", ui->jsErpmMinBox->value());
    mSettings.setValue("js_erpm_max", ui->jsErpmMaxBox->value());
    mSettings.setValue("js_range_min", ui->jsMinBox->value());
    mSettings.setValue("js_range_max", ui->jsMaxBox->value());
    if (mGamepad) {
        mSettings.setValue("js_name", mGamepad->name());
    }
#endif

    delete ui;
}

VescInterface *PageSettings::vesc() const
{
    return mVesc;
}

void PageSettings::setVesc(VescInterface *vesc)
{
    mVesc = vesc;
    if (mVesc) {
        ui->loadQmlUiConnectBox->setChecked(mVesc->getLoadQmlUiOnConnect());
    }
}

void PageSettings::setUseGamepadControl(bool useControl)
{
#ifdef HAS_GAMEPAD
    if (ui->jsConfigOkBox->isChecked()) {
        if (mGamepad) {
            mUseGamepadControl = useControl;
        } else {
            mVesc->emitMessageDialog("Gamepad Control",
                                     "No recognized gamepad is connected.",
                                     false, false);
        }
    } else if (mVesc) {
        mVesc->emitMessageDialog("Gamepad Control",
                                 "Gamepad control is not configured. Go to Settings->Gamepad to configure it.",
                                 false, false);
    }
#else
    (void)useControl;
#endif
}

bool PageSettings::isUsingGamepadControl()
{
#ifdef HAS_GAMEPAD
    return mUseGamepadControl;
#else
    return false;
#endif
}

void PageSettings::timerSlot()
{
#ifdef HAS_GAMEPAD
    if (mGamepad) {
        ui->jsAxis1Bar->setValue(mGamepad->axisLeftX() * 1000.0);
        ui->jsAxis2Bar->setValue(mGamepad->axisLeftY() * 1000.0);
        ui->jsAxis3Bar->setValue(mGamepad->axisRightX() * 1000.0);
        ui->jsAxis4Bar->setValue(mGamepad->axisRightY() * 1000.0);

        double ax = 0.0;
        if (ui->jseAxisBox->currentIndex() == 0) {
            ax = mGamepad->axisLeftX() * 1000.0;
        } else if (ui->jseAxisBox->currentIndex() == 1) {
            ax = mGamepad->axisLeftY() * 1000.0;
        } else if (ui->jseAxisBox->currentIndex() == 2) {
            ax = mGamepad->axisRightX() * 1000.0;
        } else if (ui->jseAxisBox->currentIndex() == 3) {
            ax = mGamepad->axisRightY() * 1000.0;
        }

        if (ui->jsInvertedBox->isChecked()) {
            ax = -ax;
        }

        double input = Utility::map(ax,
                                    ui->jsMinBox->value(), ui->jsMaxBox->value(),
                                    ui->jsBidirectionalBox->isChecked() ? -1.0 : 0.0, 1.0);
        double range = 0.0;
        int decimals = 2;
        QString name = "Undefined";
        QString unit = "";
        int ctrlt = ui->jsControlTypeBox->currentIndex();
        if (ctrlt == 0 || ctrlt == 1) {
            range = input >= 0 ? fabs(ui->jsCurrentMaxBox->value()) : fabs(ui->jsCurrentMinBox->value());
            input *= range;
            name = "Current";
            unit = " A";

            if (mVesc && mUseGamepadControl) {
                if (ctrlt == 0 || input > 0) {
                    mVesc->commands()->setCurrent(input);
                } else {
                    mVesc->commands()->setCurrentBrake(input);
                }
            }
        } else if (ctrlt == 2) {
            range = 1.0;
            input *= range;
            name = "Duty";
            unit = "";

            if (mVesc && mUseGamepadControl) {
                mVesc->commands()->setDutyCycle(input);
            }
        } else if (ctrlt == 3) {
            range = input >= 0 ? fabs(ui->jsErpmMaxBox->value()) : fabs(ui->jsErpmMinBox->value());
            input *= range;
            name = "Speed";
            unit = " ERPM";
            decimals = 0;

            if (mVesc && mUseGamepadControl) {
                mVesc->commands()->setRpm(input);
            }
        } else if (ctrlt == 4) {
            range = 360.0;
            input *= range;
            name = "Position";
            unit = " Degrees";
            decimals = 1;

            if (mVesc && mUseGamepadControl) {
                mVesc->commands()->setPos(input);
            }
        }

        ui->jsDisp->setRange(range);
        ui->jsDisp->setUnit(unit);
        ui->jsDisp->setName(name);
        ui->jsDisp->setVal(input);
        ui->jsDisp->setDecimals(decimals);

        if (!mGamepad->isConnected()) {
            mGamepad->deleteLater();
            mGamepad = nullptr;
        }
    }
#endif
}

void PageSettings::on_uiScaleBox_valueChanged(double arg1)
{
    mSettings.setValue("app_scale_factor", arg1);
}

void PageSettings::on_uiAutoScaleBox_toggled(bool checked)
{
    mSettings.setValue("app_scale_auto", checked);
    ui->uiScaleBox->setEnabled(!checked);
}

void PageSettings::on_jsScanButton_clicked()
{
#ifdef HAS_GAMEPAD
    ui->jsListBox->clear();
    auto gamepads = QGamepadManager::instance()->connectedGamepads();
    for (auto g: gamepads) {
        ui->jsListBox->addItem(QGamepadManager::instance()->gamepadName(g), g);
    }
#endif
}

void PageSettings::on_jsConnectButton_clicked()
{
#ifdef HAS_GAMEPAD
    QVariant item = ui->jsListBox->currentData();
    if (item.isValid()) {
        if (mGamepad) {
            mGamepad->deleteLater();
        }
        mGamepad = new QGamepad(item.toInt(), this);
    }
#endif
}

void PageSettings::on_jsResetConfigButton_clicked()
{
#ifdef HAS_GAMEPAD
    if (mGamepad) {
        QGamepadManager::instance()->resetConfiguration(mGamepad->deviceId());
    }
#endif
}

void PageSettings::on_loadQmlUiConnectBox_toggled(bool checked)
{
    if (mVesc) {
        mVesc->setLoadQmlUiOnConnect(checked);
    }
}
