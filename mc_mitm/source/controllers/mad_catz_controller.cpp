/*
 * Copyright (c) 2020-2022 ndeadly
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
#include "mad_catz_controller.hpp"
#include <stratosphere.hpp>

namespace ams::controller {

    namespace {

        const constexpr float stick_scale_factor = float(UINT12_MAX) / UINT8_MAX;

    }

    void MadCatzController::ProcessInputData(const bluetooth::HidReport *report) {
        auto madcatz_report = reinterpret_cast<const MadCatzReportData *>(&report->data);

        switch(madcatz_report->id) {
            case 0x01:
                this->MapInputReport0x01(madcatz_report); break;
            case 0x02:
                this->MapInputReport0x02(madcatz_report); break;
            default:
                break;
        }
    }

    void MadCatzController::MapInputReport0x01(const MadCatzReportData *src) {
        m_left_stick.SetData(
            static_cast<uint16_t>(stick_scale_factor * src->input0x01.left_stick.x) & 0xfff,
            static_cast<uint16_t>(stick_scale_factor * (UINT8_MAX - src->input0x01.left_stick.y)) & 0xfff
        );
        m_right_stick.SetData(
            static_cast<uint16_t>(stick_scale_factor * src->input0x01.right_stick.x) & 0xfff,
            static_cast<uint16_t>(stick_scale_factor * (UINT8_MAX - src->input0x01.right_stick.y)) & 0xfff
        );
        
        m_buttons.dpad_down   = (src->input0x01.buttons.dpad == MadCatzDPad_S)  ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_SE) ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_SW);
        m_buttons.dpad_up     = (src->input0x01.buttons.dpad == MadCatzDPad_N)  ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_NE) ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_NW);
        m_buttons.dpad_right  = (src->input0x01.buttons.dpad == MadCatzDPad_E)  ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_NE) ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_SE);
        m_buttons.dpad_left   = (src->input0x01.buttons.dpad == MadCatzDPad_W)  ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_NW) ||
                                (src->input0x01.buttons.dpad == MadCatzDPad_SW);

        m_buttons.A = src->input0x01.buttons.B;
        m_buttons.B = src->input0x01.buttons.A;
        m_buttons.X = src->input0x01.buttons.Y;
        m_buttons.Y = src->input0x01.buttons.X;

        m_buttons.R  = src->input0x01.buttons.R1;
        m_buttons.ZR = src->input0x01.right_trigger > 0;
        m_buttons.L  = src->input0x01.buttons.L1;
        m_buttons.ZL = src->input0x01.left_trigger > 0; 

        m_buttons.minus = src->input0x01.buttons.select;
        m_buttons.plus  = src->input0x01.buttons.start;

        m_buttons.lstick_press = src->input0x01.buttons.L3;
        m_buttons.rstick_press = src->input0x01.buttons.R3;

        //m_buttons.home = src->input0x01.buttons.home;
    }

    void MadCatzController::MapInputReport0x02(const MadCatzReportData *src) {
        // Media buttons
        m_buttons.home = src->input0x02.play;
    }

}
