#pragma once
#include "daisy_seed.h"

class MidiUartMonitor {
public:
  using MessageCallback = void (*)(const daisy::MidiEvent &);

  struct Config {
    daisy::MidiUartHandler::Config midi; // default seed UART Pin (D14)
    // expect UART rx at 31250 baud (default midi rate)
    float log_rate_hz = 200.0f;  // max prints per second to serial
    bool monitor_enabled = true; // enable/disable printing
  };

  // pass seed to log
  void Init(const Config &cfg, daisy::DaisySeed *hw);

  // begin rx
  void Start();
  void Stop();

  // NON BLOCKING. drains events, filters to channel-voice, prints
  // with rate limit, and invokes the user callback if set.
  void Poll();

  // control
  void SetMonitorEnabled(bool on);
  void SetRateLimitHz(float hz);
  void SetMessageCallback(MessageCallback cb);

  // diagnostic
  uint32_t DroppedBytes() const; // bytes/events unable to process/queue
  uint32_t QueueDepth() const;   // current pending MIDI events
  void ResetStats();

private:
  daisy::DaisySeed *hw_ = nullptr;
  daisy::MidiUartHandler midi_;
  MessageCallback cb_ = nullptr;
  uint32_t last_log_ms_ = 0;
  float log_interval_ms_ = 0.0f;
  bool monitor_enabled_ = true;
  uint32_t dropped_ = 0;

  // small FIFO for incoming events ready to print/dispatch.
  daisy::FIFO<daisy::MidiEvent, 128> event_log_;

  // rate limit helper
  bool CanLogNow(uint32_t now_ms) const;
};
