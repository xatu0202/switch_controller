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
    ZL,
    ZR,
    SL,
    SR,
    H_LEFT,
    H_RIGHT,
    H_UP,
    H_DOWN,
    HOME,
    PLUS,
    MINUS,
    CAPTURE,
    NOTHING
} Buttons_t;

typedef struct {
    Buttons_t button;
    uint16_t  duration;
} command;
