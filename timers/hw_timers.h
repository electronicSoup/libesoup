
typedef enum {
    uSeconds,
    mSeconds,
    Seconds
} ty_time_units;


extern u8 hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void));
extern void hw_timer_cancel(u8 timer);
