/*
 * Copyright (c) 2020-2025 ndeadly
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "8bitdo_controller.hpp"
#include <stratosphere.hpp>
#include "controller_utils.hpp"

namespace ams::controller {

    namespace {

        constexpr u8 TriggerMax = UINT8_MAX;

        constinit const u16 DpadStickPositions[] = { SwitchAnalogStick::Min, SwitchAnalogStick::Center, SwitchAnalogStick::Max };

    }

    void EightBitDoController::ProcessInputData(const bluetooth::HidReport *report) {
        auto eightbitdo_report = reinterpret_cast<const EightBitDoReportData *>(&report->data);

        switch(eightbitdo_report->id) {
            case 0x01:
                this->MapInputReport0x01(eightbitdo_report); break;
            case 0x03:
                this->MapInputReport0x03(eightbitdo_report, report->size == 11 ? EightBitDoReportFormat_ZeroV1 : EightBitDoReportFormat_ZeroV2); break;
            default:
                break;
        }
    }

    void EightBitDoController::MapInputReport0x01(const EightBitDoReportData *src) {
        if (m_controller_type == EightBitDoControllerType_Zero) {
            bool dpad_down  = (src->input0x01_v1.dpad == EightBitDoDPadV1_S)  ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_SE) ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_SW);
            bool dpad_up    = (src->input0x01_v1.dpad == EightBitDoDPadV1_N)  ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_NE) ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_NW);
            bool dpad_right = (src->input0x01_v1.dpad == EightBitDoDPadV1_E)  ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_NE) ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_SE);
            bool dpad_left  = (src->input0x01_v1.dpad == EightBitDoDPadV1_W)  ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_NW) ||
                              (src->input0x01_v1.dpad == EightBitDoDPadV1_SW);

            // Map dpad as left stick
            m_left_stick.SetData(
                DpadStickPositions[1 + dpad_right - dpad_left],
                DpadStickPositions[1 + dpad_up - dpad_down]
            );
        } else {
            m_left_stick  = PackAnalogStickValues(src->input0x01_v2.left_stick.x,  InvertAnalogStickValue(src->input0x01_v2.left_stick.y));
            m_right_stick = PackAnalogStickValues(src->input0x01_v2.right_stick.x, InvertAnalogStickValue(src->input0x01_v2.right_stick.y));

            m_buttons.dpad_down  = (src->input0x01_v2.dpad == EightBitDoDPadV2_S)  ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_SE) ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_SW);
            m_buttons.dpad_up    = (src->input0x01_v2.dpad == EightBitDoDPadV2_N)  ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_NE) ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_NW);
            m_buttons.dpad_right = (src->input0x01_v2.dpad == EightBitDoDPadV2_E)  ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_NE) ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_SE);
            m_buttons.dpad_left  = (src->input0x01_v2.dpad == EightBitDoDPadV2_W)  ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_NW) ||
                                   (src->input0x01_v2.dpad == EightBitDoDPadV2_SW);

            m_buttons.A = src->input0x01_v2.buttons.B;
            m_buttons.B = src->input0x01_v2.buttons.A;
            m_buttons.X = src->input0x01_v2.buttons.Y;
            m_buttons.Y = src->input0x01_v2.buttons.X;

            m_buttons.L = src->input0x01_v2.buttons.L1;
            m_buttons.R = src->input0x01_v2.buttons.R1;

            m_buttons.ZL = src->input0x01_v2.left_trigger  > (m_trigger_threshold * TriggerMax);
            m_buttons.ZR = src->input0x01_v2.right_trigger > (m_trigger_threshold * TriggerMax);

            if (m_controller_type == EightBitDoControllerType_Sn30ProXboxCloud) {
                m_buttons.minus = src->input0x01_v2.buttons.v1.select;
                m_buttons.plus  = src->input0x01_v2.buttons.v1.start;

                m_buttons.lstick_press = src->input0x01_v2.buttons.v1.L3;
                m_buttons.rstick_press = src->input0x01_v2.buttons.v1.R3;

                m_buttons.home = src->input0x01_v2.buttons.v1.home;
            } else {
                m_buttons.minus = src->input0x01_v2.buttons.v2.select;
                m_buttons.plus  = src->input0x01_v2.buttons.v2.start;

                m_buttons.lstick_press = src->input0x01_v2.buttons.v2.L3;
                m_buttons.rstick_press = src->input0x01_v2.buttons.v2.R3;

                m_buttons.home = src->input0x01_v2.buttons.v2.home;
            }
        }
    }

    void EightBitDoController::MapInputReport0x03(const EightBitDoReportData *src, EightBitDoReportFormat fmt) {
        if (m_controller_type == EightBitDoControllerType_Zero) {
            if (fmt == EightBitDoReportFormat_ZeroV1) {
                m_buttons.A = src->input0x03_v1.buttons.B;
                m_buttons.B = src->input0x03_v1.buttons.A;
                m_buttons.X = src->input0x03_v1.buttons.Y;
                m_buttons.Y = src->input0x03_v1.buttons.X;

                m_buttons.R = src->input0x03_v1.buttons.R1;
                m_buttons.L = src->input0x03_v1.buttons.L1;

                m_buttons.minus = src->input0x03_v1.buttons.v2.select;
                m_buttons.plus  = src->input0x03_v1.buttons.v2.start;
            } else if (fmt == EightBitDoReportFormat_ZeroV2) {
                bool dpad_down  = src->input0x03_v2.left_stick.y == 0xff;
                bool dpad_up    = src->input0x03_v2.left_stick.y == 0x00;
                bool dpad_right = src->input0x03_v2.left_stick.x == 0xff;
                bool dpad_left  = src->input0x03_v2.left_stick.x == 0x00;

                // Map dpad as left stick
                m_left_stick.SetData(
                    DpadStickPositions[1 + dpad_right - dpad_left],
                    DpadStickPositions[1 + dpad_up - dpad_down]
                );

                m_buttons.A = src->input0x03_v2.buttons.B;
                m_buttons.B = src->input0x03_v2.buttons.A;
                m_buttons.X = src->input0x03_v2.buttons.Y;
                m_buttons.Y = src->input0x03_v2.buttons.X;

                m_buttons.R = src->input0x03_v2.buttons.R1;
                m_buttons.L = src->input0x03_v2.buttons.L1;

                m_buttons.minus = src->input0x03_v2.buttons.v2.select;
                m_buttons.plus  = src->input0x03_v2.buttons.v2.start;
            }
        } else {
            m_left_stick  = PackAnalogStickValues(src->input0x03_v3.left_stick.x,  InvertAnalogStickValue(src->input0x03_v3.left_stick.y));
            m_right_stick = PackAnalogStickValues(src->input0x03_v3.right_stick.x, InvertAnalogStickValue(src->input0x03_v3.right_stick.y));

            m_buttons.dpad_down  = (src->input0x03_v3.dpad == EightBitDoDPadV2_S)  ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_SE) ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_SW);
            m_buttons.dpad_up    = (src->input0x03_v3.dpad == EightBitDoDPadV2_N)  ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_NE) ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_NW);
            m_buttons.dpad_right = (src->input0x03_v3.dpad == EightBitDoDPadV2_E)  ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_NE) ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_SE);
            m_buttons.dpad_left  = (src->input0x03_v3.dpad == EightBitDoDPadV2_W)  ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_NW) ||
                                   (src->input0x03_v3.dpad == EightBitDoDPadV2_SW);

            m_buttons.A = src->input0x03_v3.buttons.B;
            m_buttons.B = src->input0x03_v3.buttons.A;
            m_buttons.X = src->input0x03_v3.buttons.Y;
            m_buttons.Y = src->input0x03_v3.buttons.X;

            m_buttons.L = src->input0x03_v3.buttons.L1;
            m_buttons.R = src->input0x03_v3.buttons.R1;

            m_buttons.ZL = src->input0x03_v3.left_trigger  > (m_trigger_threshold * TriggerMax);
            m_buttons.ZR = src->input0x03_v3.right_trigger > (m_trigger_threshold * TriggerMax);

            m_buttons.minus = src->input0x03_v3.buttons.v2.select;
            m_buttons.plus  = src->input0x03_v3.buttons.v2.start;

            m_buttons.lstick_press = src->input0x03_v3.buttons.v2.L3;
            m_buttons.rstick_press = src->input0x03_v3.buttons.v2.R3;

            m_buttons.home = src->input0x03_v3.buttons.v2.home;

            m_battery = convert_battery_100(src->input0x03_v3.battery);
        }

    }

}
