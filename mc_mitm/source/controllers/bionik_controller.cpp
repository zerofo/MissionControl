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
#include "bionik_controller.hpp"
#include <stratosphere.hpp>

namespace ams::controller {

    namespace {

        constexpr u8 TriggerMax = UINT8_MAX;

    }

    void BionikController::ProcessInputData(const bluetooth::HidReport *report) {
        auto Bionik_report = reinterpret_cast<const BionikReportData *>(&report->data);

        switch(Bionik_report->id) {
            case 0x03:
                this->MapInputReport0x03(Bionik_report); break;
            case 0x04:
                this->MapInputReport0x04(Bionik_report); break;
            default:
                break;
        }
    }

    void BionikController::MapInputReport0x03(const BionikReportData *src) {
        m_left_stick  = PackAnalogStickValues(src->input0x03.left_stick.x,  InvertAnalogStickValue(src->input0x03.left_stick.y));
        m_right_stick = PackAnalogStickValues(src->input0x03.right_stick.x, InvertAnalogStickValue(src->input0x03.right_stick.y));

        m_buttons.dpad_down  = (src->input0x03.buttons.dpad == BionikDPad_S)  ||
                               (src->input0x03.buttons.dpad == BionikDPad_SE) ||
                               (src->input0x03.buttons.dpad == BionikDPad_SW);
        m_buttons.dpad_up    = (src->input0x03.buttons.dpad == BionikDPad_N)  ||
                               (src->input0x03.buttons.dpad == BionikDPad_NE) ||
                               (src->input0x03.buttons.dpad == BionikDPad_NW);
        m_buttons.dpad_right = (src->input0x03.buttons.dpad == BionikDPad_E)  ||
                               (src->input0x03.buttons.dpad == BionikDPad_NE) ||
                               (src->input0x03.buttons.dpad == BionikDPad_SE);
        m_buttons.dpad_left  = (src->input0x03.buttons.dpad == BionikDPad_W)  ||
                               (src->input0x03.buttons.dpad == BionikDPad_NW) ||
                               (src->input0x03.buttons.dpad == BionikDPad_SW);

        m_buttons.A = src->input0x03.buttons.B;
        m_buttons.B = src->input0x03.buttons.A;
        m_buttons.X = src->input0x03.buttons.Y;
        m_buttons.Y = src->input0x03.buttons.X;

        m_buttons.R  = src->input0x03.buttons.R1;
        m_buttons.L  = src->input0x03.buttons.L1;
        m_buttons.ZR = src->input0x03.right_trigger > (m_trigger_threshold * TriggerMax);
        m_buttons.ZL = src->input0x03.left_trigger  > (m_trigger_threshold * TriggerMax);

        m_buttons.lstick_press = src->input0x03.buttons.L3;
        m_buttons.rstick_press = src->input0x03.buttons.R3;

        m_buttons.plus  = src->input0x03.buttons.start;
    }

    void BionikController::MapInputReport0x04(const BionikReportData *src) {
        m_buttons.minus = src->input0x04.buttons.back;
        m_buttons.home = src->input0x04.buttons.home;
    }

}
