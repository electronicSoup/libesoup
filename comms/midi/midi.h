
#include "libesoup_config.h"

#if defined(SYS_MIDI_TX) || defined(SYS_MIDI_RX)

struct midi_data {
#if defined(SYS_MIDI_TX)
	enum gpio_pin        tx_pin;
#endif
#if defined(SYS_MIDI_RX)
	enum gpio_pin    rx_pin;
	void           (*process_midi_rx)(void);
#endif
};


extern result_t midi_reserve(struct midi_data *data);

#endif // defined(SYS_MIDI_TX) || defined(SYS_MIDI_RX)
