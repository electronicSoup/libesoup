
#define NUMBER_HW_TIMERS  5
#define TIMER_1 0
#define TIMER_2 1
#define TIMER_3 2
#define TIMER_4 3
#define TIMER_5 4

typedef enum {
    uSeconds,
    mSeconds,
    Seconds
} ty_time_units;

extern void hw_timer_init(void);
extern u8 hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void));
extern void hw_timer_cancel(u8 timer);
