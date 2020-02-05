/*
   Nintendo Switch Fightstick - Proof-of-Concept

   Based on the LUFA library's Low-Level Joystick Demo
   (C) Dean Camera
   Based on the HORI's Pokken Tournament Pro Pad design
   (C) HORI

   This project implements a modified version of HORI's Pokken Tournament Pro Pad
   USB descriptors to allow for the creation of custom controllers for the
   Nintendo Switch. This also works to a limited degree on the PS3.

   Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
   Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
   the Pokken Controller from functioning at the same level as the Pro
   Controller. However, by default most of the descriptors are there, with the
   exception of Home and Capture. Descriptor modification allows us to unlock
   these buttons for our use.
 */

#include "Joystick.h"
#include "arduino/Arduino.h"

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    X,
    Y,
    A,
    B,
    L,
    R,
    H_LEFT,
    H_RIGHT,
    H_UP,
    HOME,
    NOTHING,
    COUNT,
    TRIGGERS
} Buttons_t;

typedef struct {
    Buttons_t button;
    uint16_t  duration;
} command;

static const command step[] = {
    {H_UP,    1      },
    {RIGHT,   1      },
    {H_RIGHT, 1      },
    {RIGHT,   1      },
    {A,       1      },
    {NOTHING, 4      },


    {A,       1      },
    {NOTHING, 4      },
    {LEFT,    1      },
    {H_LEFT,  1      },
    {LEFT,    1      }
    // end loop
};

int main(void)
{
    SetupHardware();
    GlobalInterruptEnable();
    for (;;)
    {
        HID_Task();
        USB_USBTask();
    }
}

void SetupHardware(void)
{
    // We need to disable watchdog if enabled by bootloader/fuses.
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    // We need to disable clock division before initializing the USB hardware.
    clock_prescale_set(clock_div_1);
    // We can then initialize our hardware and peripherals, including the USB stack.

    #ifdef ALERT_WHEN_DONE
        // Both PORTD and PORTB will be used for the optional LED flashing and buzzer.
    #warning LED and Buzzer functionality enabled. All pins on both PORTB and \
    PORTD will toggle when printing is done.
        DDRD  = 0xFF;                        // Teensy uses PORTD
        PORTD =  0x0;
        // We'll just flash all pins on both ports since the UNO R3
        DDRB  = 0xFF;                        // uses PORTB. Micro can use either or, but both give us 2 LEDs
        PORTB =  0x0;                        // The ATmega328P on the UNO will be resetting, so unplug it?
    #endif
    // The USB stack should be initialized last.
    USB_Init();
}

void EVENT_USB_Device_Connect(void)
{
}

void EVENT_USB_Device_Disconnect(void)
{
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    // We setup the HID report endpoints.
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

    // We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void)
{
    // We can handle two control requests: a GetReport and a SetReport.

    // Not used here, it looks like we don't receive control request from the Switch.
}

void HID_Task(void)
{
    if (USB_DeviceState != DEVICE_STATE_Configured) {
        return;
    }
    Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
    if (Endpoint_IsOUTReceived()) {
        if (Endpoint_IsReadWriteAllowed()) {
            USB_JoystickReport_Output_t JoystickOutputData;
            while (Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError) { }
        }
        Endpoint_ClearOUT();
    }
    Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
    if (Endpoint_IsINReady()) {
        USB_JoystickReport_Input_t JoystickInputData;
        GetNextReport(&JoystickInputData);
        while (Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError) { }
        Endpoint_ClearIN();
    }
}

typedef enum {
    SYNC_CONTROLLER,
    SYNC_POSITION,
    BREATHE,
    PROCESS,
    CLEANUP,
    DONE
} State_t;
State_t state = SYNC_CONTROLLER;

#define ECHOES 2
int                        echoes = 0;
USB_JoystickReport_Input_t last_report;

int report_count   = 0;
int xpos           = 0;
int ypos           = 0;
int bufindex       = 0;
int duration_count = 0;
int portsval       = 0;

int count    = 0;
int looped   = 0;
int max_loop = 10000;
// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t *const ReportData)
{
    // Prepare an empty report
    memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
    ReportData->LX  = STICK_CENTER;
    ReportData->LY  = STICK_CENTER;
    ReportData->RX  = STICK_CENTER;
    ReportData->RY  = STICK_CENTER;
    ReportData->HAT = HAT_CENTER;

    // Repeat ECHOES times the last report
    if (echoes > 0)
    {
        memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
        echoes--;

        return;
    }
    state = PROCESS;
    // States and moves management
    switch (state) {
        case PROCESS:
            switch (step[bufindex].button) {
                case UP:
                    ReportData->LY = STICK_MIN;
                    break;

                case LEFT:
                    ReportData->LX = STICK_MIN;
                    break;

                case DOWN:
                    ReportData->LY = STICK_MAX;
                    break;

                case RIGHT:
                    ReportData->LX = STICK_MAX;
                    break;

                case A:
                    ReportData->Button |= SWITCH_A;
                    break;

                case B:
                    ReportData->Button |= SWITCH_B;
                    break;

                case R:
                    ReportData->Button |= SWITCH_R;
                    break;

                case HOME:
                    ReportData->Button |= SWITCH_HOME;
                    break;

                case TRIGGERS:
                    ReportData->Button |= SWITCH_L | SWITCH_R;
                    break;
                case H_LEFT:
                    ReportData->HAT = HAT_LEFT;
                    break;
                case H_RIGHT:
                    ReportData->HAT = HAT_RIGHT;
                    break;
                case H_UP:
                    ReportData->HAT = HAT_TOP;
                    break;
                default:
                    ReportData->LX  = STICK_CENTER;
                    ReportData->LY  = STICK_CENTER;
                    ReportData->RX  = STICK_CENTER;
                    ReportData->RY  = STICK_CENTER;
                    ReportData->HAT = HAT_CENTER;
                    break;
            }

            duration_count++;

            if (duration_count > step[bufindex].duration) {
                bufindex++;
                duration_count = 0;
            }
            if (bufindex > (int) (sizeof(step) / sizeof(step[0])) - 1) {
                bufindex       = 0;                                                                          // 7
                duration_count = 0;
                // state          = PROCESS;
                count++;
                if (count % 31 != 0) {
                    looped++;
                    digitalWrite(17, LOW);
                } else {digitalWrite(17, HIGH);}
                if (looped > max_loop) {
                    state = DONE;
                    bufindex = 5;
                    digitalWrite(30, HIGH);
                } else {
                    digitalWrite(30, LOW);
                }
            }

            if (looped > max_loop) {
                state = DONE;
                bufindex = 5;
            }
            break;
        case DONE:

            return;
    }
    // Prepare to echo this report
    memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
    echoes = ECHOES;
}
