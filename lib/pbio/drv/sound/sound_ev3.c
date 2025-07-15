// SPDX-License-Identifier: MIT
// Copyright (c) 2025 The Pybricks Authors

#include <pbdrv/config.h>

#if PBDRV_CONFIG_SOUND_EV3

#include <stdint.h>

#include <pbdrv/gpio.h>

#include <tiam1808/armv5/am1808/interrupt.h>
#include <tiam1808/ehrpwm.h>
#include <tiam1808/hw/soc_AM1808.h>
#include <tiam1808/hw/hw_syscfg0_AM1808.h>
#include <tiam1808/psc.h>

#include "../drv/gpio/gpio_ev3.h"

// Audio amplifier enable
static const pbdrv_gpio_t pin_sound_en = PBDRV_GPIO_EV3_PIN(13, 3, 0, 6, 15);
// Audio output pin
#define SYSCFG_PINMUX3_PINMUX3_7_4_GPIO0_0 0
static const pbdrv_gpio_t pin_audio = PBDRV_GPIO_EV3_PIN(3, 7, 4, 0, 0);

static void sound_isr() {
    static int test = 0;
    IntSystemStatusClear(SYS_INT_EHRPWM0);
    EHRPWMETIntClear(SOC_EHRPWM_0_REGS);
    if (++test == 880) {
        test = 0;
        HWREGB(SOC_UART_1_REGS) = 'A';
    }
}

void pbdrv_sound_init() {
    // Turn on EPWM
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_EHRPWM, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    EHRPWMTimebaseClkConfig(SOC_EHRPWM_0_REGS, SOC_EHRPWM_0_MODULE_FREQ / 10, SOC_EHRPWM_0_MODULE_FREQ);
    EHRPWMPWMOpFreqSet(SOC_EHRPWM_0_REGS, SOC_EHRPWM_0_MODULE_FREQ / 10, 440, EHRPWM_COUNT_UP, true);
    EHRPWMLoadCMPB(SOC_EHRPWM_0_REGS, SOC_EHRPWM_0_MODULE_FREQ / 10 / 440 / 2, true, 0, true);
    EHRPWMConfigureAQActionOnB(
        SOC_EHRPWM_0_REGS,
        EHRPWM_AQCTLB_ZRO_EPWMXBHIGH,
        EHRPWM_AQCTLB_PRD_DONOTHING,
        EHRPWM_AQCTLB_CAU_DONOTHING,
        EHRPWM_AQCTLB_CAD_DONOTHING,
        EHRPWM_AQCTLB_CBU_EPWMXBLOW,
        EHRPWM_AQCTLB_CBD_DONOTHING,
        EHRPWM_AQSFRC_ACTSFB_DONOTHING
        );
    EHRPWMDBOutput(SOC_EHRPWM_0_REGS, EHRPWM_DBCTL_OUT_MODE_BYPASS);
    EHRPWMChopperDisable(SOC_EHRPWM_0_REGS);
    EHRPWMTZTripEventDisable(SOC_EHRPWM_0_REGS, false);
    EHRPWMTZTripEventDisable(SOC_EHRPWM_0_REGS, true);

    // Interrupts
    IntRegister(SYS_INT_EHRPWM0, sound_isr);
    IntChannelSet(SYS_INT_EHRPWM0, 2);
    IntSystemEnable(SYS_INT_EHRPWM0);
    EHRPWMETIntSourceSelect(SOC_EHRPWM_0_REGS, EHRPWM_ETSEL_INTSEL_TBCTREQUPRD);
    EHRPWMETIntPrescale(SOC_EHRPWM_0_REGS, EHRPWM_ETPS_INTPRD_FIRSTEVENT);
    EHRPWMETIntEnable(SOC_EHRPWM_0_REGS);

    // Configure IO pin modes
    pbdrv_gpio_alt(&pin_audio, SYSCFG_PINMUX3_PINMUX3_7_4_EPWM0B);
    pbdrv_gpio_out_high(&pin_sound_en);
}

void pbdrv_sound_start(const uint16_t *data, uint32_t length, uint32_t sample_rate) {

}

void pbdrv_sound_stop() {

}

#endif // PBDRV_CONFIG_SOUND_EV3
