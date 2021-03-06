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
    H_DOWN,
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
    {B,       10      },
    {NOTHING, 10      },
    {B,       10      },
    {NOTHING, 10      },
    {B,       10      },

    {NOTHING, 10      },

    {HOME,    5       }, // HOME
    {NOTHING, 20      },
    {DOWN,    1       },
    {H_RIGHT, 1       },
    {RIGHT,   1       },
    {H_RIGHT, 1       },
    {RIGHT,   1       },
    {A,       1       },
    {NOTHING, 55      }, // 設定
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
    {NOTHING, 23      }, // 設定->本体
    {DOWN,    1       },
    {H_DOWN,  1       },
    {DOWN,    1       },
    {H_DOWN,  1       },
    {A,       1       },
    {NOTHING, 23      }, // 設定->本体->日付と時刻
    {DOWN,    1       },
    {H_DOWN,  1       },
    {A,       1       },
    {NOTHING, 25      }, // 現在時刻設定
    {RIGHT,   1       },
    {H_RIGHT, 1       },
    {UP,      1       },
    {H_RIGHT, 1       },
    {RIGHT,   1       },
    {H_RIGHT, 1       },
    {A,       1       },
    {HOME,    5       },
    {NOTHING, 25      },
    {HOME,    5       }, // ゲームに戻る
    {NOTHING, 50      },
    {A,       5       }, // 話しかける
    {NOTHING, 10      }, // こんにちロ〜！　なにを　しますロミ？
    {B,       5       },
    {NOTHING, 10      },
    {DOWN,    5       },
    {NOTHING, 10      },
    {A,       5       }, // IDくじを選択
    {NOTHING, 30      }, // ただいま　IDくじセンターの抽選コーナーに　繋いだロミ！
    {B,       5       },
    {NOTHING, 15      }, // 引いた　くじの　ナンバーと　XXさんの　ポケモンのIDが
    {B,       5       },
    {NOTHING, 15      }, // みごと　あってると　ステキな　賞品を　もらえちゃうんだロ！
    {B,       5       },
    {NOTHING, 20      }, // 運試しに　レポートを　書いて　引いてみるのは　どうロミ？
    {A,       5       }, // はい
    {NOTHING, 50      }, // レポートを書き込んでいます（ryしっかり書き残した
    {B,       5       },
    {NOTHING, 15      }, // かしこまりロ〜！　抽選　スタート　ロミ！
    {B,       5       },
    {NOTHING, 15      }, //
    {B,       5       },
    {NOTHING, 10      }, // かしこまりロ〜！　抽選スタートロミ！
    {B,       5       },
    {NOTHING, 50      }, // ...... ...... ......
    {B,       5       },
    {NOTHING, 15      }, // ハイ！でたロミ！　くじの　ナンバーは　XXXXX！
    {B,       5       },
    {NOTHING, 15      }, // XXXさんの　ポケモンIDと　どれだけ　あってるか
    {B,       5       },
    {NOTHING, 15      }, // 調べてみるロ！
    {B,       5       },
    {NOTHING, 15      }, // おめでロ〜〜！！！！
    {B,       5       },
    {NOTHING, 15      }, // ボックスに　預けている　XXちゃんの　IDが　みごと
    {B,       5       },
    {NOTHING, 15      }, // くじの　ナンバーと　ぴったしロミ！
    {B,       5       },
    {NOTHING, 15      }, // 〜〜ケタが〜〜〜
    {B,       5       },
    {NOTHING, 15      }, // 〜〜〜等の賞品
    {B,       5       },
    {NOTHING, 15      }, // 〜〜をプレゼント
    {B,       5       },
    {NOTHING, 80      }, // XXは　OOを　手に入れた！
    {B,       5       },
    {NOTHING, 15      }, // XXは　OOを　〜〜に　しまった
    {B,       5       },
    {NOTHING, 15      }, //  それじゃあ　またの　挑戦を　お待ちしてるロ〜〜！
    {B,       5       }, // 閉じる
    {NOTHING, 10      }, //
    {B,       5       },
    {NOTHING, 10      },
    {B,       5       },
    {NOTHING, 10      } // 保険

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

void SetupHardware(void)
{
    // We need to disable watchdog if enabled by bootloader/fuses.
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    // We need to disable clock division before initializing the USB hardware.
    clock_prescale_set(clock_div_1);
    USB_Init();
}

void EVENT_USB_Device_Connect(void)
{
}

void EVENT_USB_Device_Disconnect(void)
{
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
}

void EVENT_USB_Device_ControlRequest(void)
{
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
