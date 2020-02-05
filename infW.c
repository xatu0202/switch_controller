#include "Joystick.h"
#include "arduino/Arduino.h"
#include "buttons.h"

static const command step[] = {
    {B,       10      },
    {NOTHING, 10      },
    {B,       10      },
    {NOTHING, 10      },
    {B,       10      },

    {NOTHING, 1       },

    {HOME,    5       }, // HOME
    {NOTHING, 15      }, // 13
    {DOWN,    1       },
    {H_RIGHT, 1       },
    {RIGHT,   1       },
    {H_RIGHT, 1       },
    {RIGHT,   1       },
    {A,       1       },
    {NOTHING, 35      }, // 設定 32
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {A,       1       },
    {NOTHING, 10       }, // 設定->本体 7
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {A,       1       },
    {NOTHING, 10       }, // 設定->本体->日付と時刻 7
    {DOWN,    1       },
    {H_DOWN,  1       },
    {A,       1       },
    {NOTHING, 10       }, // 現在時刻設定 6
    {RIGHT,   1       },
    {H_RIGHT, 1       },
    {UP,      1       },
    {H_RIGHT, 1       },
    {RIGHT,   1       },
    {H_RIGHT, 1       },
    {A,       1       },
    {HOME,    5       },
    {NOTHING, 20      }, // 17
    {HOME,    5       }, // ゲームに戻る
    {NOTHING, 20      },
    {A,       5       },
    {NOTHING, 5       },
    {B,       5       },
    {NOTHING, 5       },
    {B,       5       },
    {NOTHING, 5       },
    {B,       5       },
    {NOTHING, 5       },
    {B,       5       },
    {NOTHING, 5       },
    {B,       5       },
    {NOTHING, 5       },
    {B,       5       },
    {NOTHING, 30      }
    //
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
typedef enum {
    SYNC_CONTROLLER,
    PROCESS,
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

void GetNextReport(USB_JoystickReport_Input_t *const ReportData)
{
    memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
    ReportData->LX  = STICK_CENTER;
    ReportData->LY  = STICK_CENTER;
    ReportData->RX  = STICK_CENTER;
    ReportData->RY  = STICK_CENTER;
    ReportData->HAT = HAT_CENTER;

    if (echoes > 0)
    {
        memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
        echoes--;

        return;
    }
    state = PROCESS;
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
                case H_LEFT:
                    ReportData->HAT = HAT_LEFT;
                    break;
                case H_RIGHT:
                    ReportData->HAT = HAT_RIGHT;
                    break;
                case H_UP:
                    ReportData->HAT = HAT_TOP;
                    break;
                case H_DOWN:
                    ReportData->HAT = HAT_BOTTOM;
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
                bufindex       = 5;    // 7
                duration_count = 0;
            }
            break;
        case DONE:

            return;
    }
    memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
    echoes = ECHOES;
}
