#include "midi_uart_monitor.h"

using namespace daisy;

void MidiUartMonitor::Init(const Config &cfg, DaisySeed *hw) {
  hw_ = hw;
  monitor_enabled_ = cfg.monitor_enabled;
  log_interval_ms_ = 1000.0f / cfg.log_rate_hz;

  // set up midi uart handler
  midi_.Init(cfg.midi);
  midi_.StartReceive();

  // clear state
  event_log_.Clear();
  dropped_ = 0;
  last_log_ms_ = 0;
}

bool MidiUartMonitor::CanLogNow(uint32_t now_ms) const {
  return (now_ms - last_log_ms_) >= log_interval_ms_;
}

// begin rx (safe if called more than once)
void MidiUartMonitor::Start() { midi_.StartReceive(); }

void MidiUartMonitor::Poll() {
  // keep uart alive in case of overrun
  midi_.Listen();

  // drain uart -> local fifo (channel-voice only)
  while (midi_.HasEvents()) {
    auto ev = midi_.PopEvent();
    if (ev.type >= daisy::NoteOff && ev.type <= daisy::PitchBend) {
      if (!event_log_.IsFull()) {
        event_log_.PushBack(ev);
      } else {
        dropped_++;
      }
    }
  }

  // dispatch + rate-limited log
  const uint32_t now = daisy::System::GetNow();
  while (event_log_.GetNumElements() > 0) {
    daisy::MidiEvent ev = event_log_.PopFront();

    // user callback (not rate-limited)
    if (cb_) {
      cb_(ev);
    }

    // serial log (rate-limited)
    if (monitor_enabled_) {
      char out[96];
      const char *tstr = daisy::MidiEvent::GetTypeAsString(ev);
      // time, type, ch, d1, d2
      std::snprintf(out, sizeof(out), "time:%lu\ttype:%s\tch:%d\td1:%d\td2:%d",
                    (unsigned long)now, tstr, ev.channel, ev.data[0],
                    ev.data[1]);
      if (hw_) {
        hw_->PrintLine(out);
      }
      last_log_ms_ = now;
      // print only once per interval; leave any remaining to next Poll
      break;
    }
    // if not allowed to print yet, we already invoked cb_ and continue
    // no re-queue to keep flow forward
  }
}

// control
void MidiUartMonitor::SetMonitorEnabled(bool on) { monitor_enabled_ = on; }

void MidiUartMonitor::SetRateLimitHz(float hz) {
  // clamp; hz <= 0 disables logging by making interval huge
  log_interval_ms_ = (hz > 0.f) ? (1000.0f / hz) : 1e9f;
}

void MidiUartMonitor::SetMessageCallback(MessageCallback cb) { cb_ = cb; }

// diagnostic
uint32_t MidiUartMonitor::DroppedBytes() const { return dropped_; }

uint32_t MidiUartMonitor::QueueDepth() const {
  return event_log_.GetNumElements();
}

void MidiUartMonitor::ResetStats() { dropped_ = 0; }
