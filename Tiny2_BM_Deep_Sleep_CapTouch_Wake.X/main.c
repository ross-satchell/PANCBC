/*
 *  ATtiny1627 ? Cap Touch Hold with RGB Colour Progression
 *
 *  RGB LED: common cathode, active HIGH
 *    PA1 = Blue
 *    PA2 = Red
 *    PA3 = Green
 *
 *  PB7 = active LOW LED (turns on when hold confirmed at 3s)
 *  PB3 = pulse output (HIGH for ~128ms on touch start)
 *  PA6 = cap touch input
 *
 *  Sequence:
 *    PA6 rising edge ? RGB RED, PB3 HIGH, TCA starts
 *    ~128ms (4 ticks)   ? PB3 LOW
 *    ~1.504s (47 ticks) ? still held? RGB ORANGE : abort
 *    ~2.976s (93 ticks) ? still held? RGB GREEN + PB7 ON : abort
 *    GREEN_HOLD_TICKS   ? RGB OFF, PB7 OFF, return to idle
 *
 *  Touch detection uses two conditions at each checkpoint:
 *    (a) TouchPulseCount >= TOUCH_THRESHOLD_TOTAL
 *    (b) Last pulse within RECENT_WINDOW_TICKS of checkpoint
 *
 *  Clock: 32kHz ULP, no prescaler divider
 *  TCA:   DIV1024 ? 31.25 Hz (~32ms/tick)
 *    PB3 pulse        =  4 ticks = ~128ms
 *    Red?Orange check = 47 ticks = ~1.504s
 *    Orange?Green     = 93 ticks = ~2.976s
 *    Green hold       = 156 ticks = ~4.992s  ? 5s
 */

#include <xc.h>
#include <avr/xmega.h>
#include <avr/interrupt.h>
#include <stdbool.h>

/* ?? Timing ????????????????????????????????????????????????? */
#define PB3_PULSE_TICKS         4
#define RED_TO_ORANGE_TICKS    47
#define ORANGE_TO_GREEN_TICKS  93

/*
 * GREEN_HOLD_SECONDS: how long to hold green + PB7 on after
 * the 3s hold is confirmed. Set in whole seconds.
 * TCA tick rate = 32000 Hz / 1024 = 31.25 Hz (~32ms per tick)
 * Converted to ticks automatically by SECONDS_TO_TICKS.
 * Use uint16_t for hold_ticks if setting above ~8s.
 */
#define TCA_TICK_HZ                 31      /* ticks per second (rounded) */
#define SECONDS_TO_TICKS(s)         ((s) * TCA_TICK_HZ)

#define GREEN_HOLD_SECONDS          5
#define GREEN_HOLD_TICKS            SECONDS_TO_TICKS(GREEN_HOLD_SECONDS)

/* ?? Touch sensitivity ?????????????????????????????????????? */
#define TOUCH_THRESHOLD_TOTAL   3
#define RECENT_WINDOW_TICKS     8

/* ?? RGB pin masks on PORTA ???????????????????????????????? */
#define PIN_BLUE    PIN1_bm
#define PIN_RED     PIN2_bm
#define PIN_GREEN   PIN3_bm
#define RGB_MASK    (PIN_BLUE | PIN_RED | PIN_GREEN)

#define RGB_OFF()    (PORTA.OUTCLR = RGB_MASK)
#define RGB_RED()    do { PORTA.OUTCLR = RGB_MASK; \
                          PORTA.OUTSET = PIN_RED;             } while(0)
#define RGB_ORANGE() do { PORTA.OUTCLR = RGB_MASK; \
                          PORTA.OUTSET = PIN_RED | PIN_GREEN; } while(0)
#define RGB_GREEN()  do { PORTA.OUTCLR = RGB_MASK; \
                          PORTA.OUTSET = PIN_GREEN;           } while(0)

#define LED_ON()     (PORTB.OUTCLR = PIN7_bm)
#define LED_OFF()    (PORTB.OUTSET = PIN7_bm)

/* ?? States ????????????????????????????????????????????????? */
typedef enum {
    WAITING_TOUCH,
    TCA_RUNNING,
    GREEN_HOLD,         /* counting down GREEN_HOLD_TICKS      */
} AppState;

/* ?? Prototypes ????????????????????????????????????????????? */
void MainClkCtrl(void);
void Init_Ports(void);
void Init_TCA(void);
void TCA_Start(void);
void TCA_Stop(void);
void Abort_Sequence(void);

/* ?? Shared state ??????????????????????????????????????????? */
volatile AppState State           = WAITING_TOUCH;
volatile uint8_t  TouchPulseCount = 0;
volatile uint8_t  LastPulseTick   = 0;
volatile uint8_t  SequenceTick    = 0;

/* ?? Check still held ?????????????????????????????????????? */
static inline uint8_t Check_Still_Held(void) {
    cli();
    uint8_t total    = TouchPulseCount;
    uint8_t lastTick = LastPulseTick;
    uint8_t nowTick  = SequenceTick;
    TouchPulseCount  = 0;
    LastPulseTick    = nowTick;
    sei();

    uint8_t gap    = nowTick - lastTick;
    uint8_t recent = (gap <= RECENT_WINDOW_TICKS);
    return (total >= TOUCH_THRESHOLD_TOTAL) && recent;
}

/* ??????????????????????????????????????????????????????????? */
int main(void) {

    MainClkCtrl();
    Init_Ports();
    Init_TCA();

    sei();

    while (1) {
        SLPCTRL.CTRLA = (State == WAITING_TOUCH)
            ? (SLPCTRL_SMODE_PDOWN_gc | SLPCTRL_SEN_bm)
            : (SLPCTRL_SMODE_IDLE_gc  | SLPCTRL_SEN_bm);

        asm("SLEEP");
        asm("NOP");
    }

    return 0;
}

/* ?? TCA0 OVF ISR ? fires every ~32ms ???????????????????????? */
ISR(TCA0_OVF_vect) {

    static uint8_t  ticks       = 0;
    static uint16_t hold_ticks  = 0;

    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;

    SequenceTick++;

    /* ?? GREEN_HOLD: fixed countdown, ignore touch ???????? */
    if (State == GREEN_HOLD) {
        hold_ticks++;
        if (hold_ticks >= GREEN_HOLD_TICKS) {
            hold_ticks = 0;
            LED_OFF();
            RGB_OFF();
            TCA_Stop();
            SequenceTick = 0;
            State        = WAITING_TOUCH;
        }
        return;
    }

    /* ?? TCA_RUNNING ??????????????????????????????????????? */
    ticks++;

    /* ~128ms: lower PB3 */
    if (ticks == PB3_PULSE_TICKS) {
        PORTB.OUTCLR = PIN3_bm;
    }

    /* ~1.5s: Red ? Orange */
    if (ticks == RED_TO_ORANGE_TICKS) {
        if (Check_Still_Held()) {
            RGB_ORANGE();
        } else {
            Abort_Sequence();
            ticks = 0;
            return;
        }
    }

    /* ~3s: Orange ? Green */
    if (ticks >= ORANGE_TO_GREEN_TICKS) {
        ticks = 0;
        if (Check_Still_Held()) {
            RGB_GREEN();
            LED_ON();
            /*
             * Restart TCA to count GREEN_HOLD_TICKS from zero.
             * hold_ticks is reset here too so the countdown
             * starts cleanly regardless of prior state.
             */
            TCA_Stop();
            hold_ticks   = 0;
            SequenceTick = 0;
            TCA_Start();
            State        = GREEN_HOLD;
        } else {
            Abort_Sequence();
        }
    }
}

/* ?? PA6 ISR ? rising edge on every cap touch pulse ?????????? */
ISR(PORTA_PORT_vect) {
    PORTA.INTFLAGS = PIN6_bm;

    if (State == WAITING_TOUCH) {
        TouchPulseCount = 0;
        LastPulseTick   = 0;
        SequenceTick    = 0;
        RGB_RED();
        PORTB.OUTSET    = PIN3_bm;
        TCA_Start();
        State           = TCA_RUNNING;
    } else if (State == TCA_RUNNING) {
        TouchPulseCount++;
        LastPulseTick = SequenceTick;
    }
    /* Pulses during GREEN_HOLD are ignored */
}

/* ?? Abort ????????????????????????????????????????????????? */
void Abort_Sequence(void) {
    TCA_Stop();
    PORTB.OUTCLR    = PIN3_bm;
    RGB_OFF();
    cli();
    TouchPulseCount = 0;
    LastPulseTick   = 0;
    SequenceTick    = 0;
    sei();
    State = WAITING_TOUCH;
}

/* ?? TCA helpers ??????????????????????????????????????????? */
void TCA_Start(void) {
    TCA0.SINGLE.CNT   = 0;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc
                      | TCA_SINGLE_ENABLE_bm;
}

void TCA_Stop(void) {
    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
    TCA0.SINGLE.CNT    = 0;
}

/* ?? Clock ????????????????????????????????????????????????? */
void MainClkCtrl(void) {
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSCULP32K_gc);
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, ~CLKCTRL_PEN_bm);
}

/* ?? TCA0 init ????????????????????????????????????????????? */
void Init_TCA(void) {
    TCA0.SINGLE.CTRLA   = 0;
    TCA0.SINGLE.CTRLB   = TCA_SINGLE_WGMODE_NORMAL_gc;
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.PER     = 1;
    TCA0.SINGLE.CNT     = 0;
}

/* ?? Ports ????????????????????????????????????????????????? */
void Init_Ports(void) {
    PORTB.DIRSET = PIN7_bm | PIN3_bm;
    LED_OFF();
    PORTB.OUTCLR = PIN3_bm;

    PORTA.DIRSET = PIN_BLUE | PIN_RED | PIN_GREEN;
    RGB_OFF();

    PORTA.PIN6CTRL = PORT_ISC_RISING_gc;

    PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;

    PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTB.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTB.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTB.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTB.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTB.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;

    PORTC.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
}