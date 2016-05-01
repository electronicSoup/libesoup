
typedef struct {
    u8            tx_pin;
    u8            rx_pin;
    u8            uart;
    u16           magic;
    u16           uart_mode;
    u16           baud;
//    u8            line_termination;
//    ty_time_units first_resp_char_units;
//    u16           time_for_first_resp_char;
//    ty_time_units last_character_units;
//    u16           timeout_last_character_ms;
//    void        (*process_line)(u8 *, u16);
//    u8           *rx_buffer;
//    u16           rx_buffer_len;
//    u16           rx_buffer_size;
    void        (*process_char)(u8);
    u8           *tx_buffer;
    u16           tx_buffer_read_index;
    u16           tx_buffer_size;
} uart_data;


extern void     uart_init(void);
extern result_t uart_reserve(uart_data *data);
