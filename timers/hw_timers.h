
#define NUMBER_HW_TIMERS  5
#define TIMER_1 0
#define TIMER_2 1
#define TIMER_3 2
#define TIMER_4 3
#define TIMER_5 4

#define BAD_TIMER 0xff

extern void     hw_timer_init(void);
extern u8       hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(u8), u8 data);
extern result_t hw_timer_restart(u8 hw_timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(u8), u8 data);
extern result_t hw_timer_pause(u8 timer);

extern void     hw_timer_cancel(u8 timer);
extern void     hw_timer_cancel_all();
