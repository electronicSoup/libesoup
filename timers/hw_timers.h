
#define TIMER_1 1
#define TIMER_2 2
#define TIMER_3 3
#define TIMER_4 4
#define TIMER_5 5

typedef enum {
    uSeconds,
    mSeconds,
    Seconds
} ty_time_units;


extern u8 hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void));
extern void hw_timer_cancel(u8 timer);
