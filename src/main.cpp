#include "daisy_seed.h"
#include "daisysp.h"
#include "midi_uart_monitor.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
MidiUartMonitor midi;

static void OnMidi(const MidiEvent &ev) {
  // example: react to channel-voice events here (no logging)
  // ev.type: NoteOn/NoteOff/ControlChange/PitchBend/etc.
  // ev.channel, ev.data[0], ev.data[1]
  // keep this fast, no blocking
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  for (size_t i = 0; i < size; i++) {
    out[0][i] = in[0][i];
    out[1][i] = in[1][i];
  }
}

int main(void) {
  // seed + serial
  hw.Configure();
  hw.Init();
  hw.StartLog(true); // usb serial prints

  // config for uart midi
  MidiUartMonitor::Config cfg;

  // optional: set a specific uart/periph/pins (uncomment and set as needed)
  cfg.midi.transport_config.periph = UartHandler::Config::Peripheral::USART_1;
  cfg.midi.transport_config.rx = hw.GetPin(14); // your rx pin
  // cfg.midi.transport_config.tx = Pin(); // tx not needed if rx only

  cfg.log_rate_hz = 100.0f; // throttle prints
  cfg.monitor_enabled = true;

  midi.Init(cfg, &hw);
  midi.SetMessageCallback(OnMidi);
  midi.Start();

  for (;;) {
    midi.Poll();
    System::Delay(1); // keep cpu cool; midi dma still runs
  }
}
