# XIAO ESP32S3 Voice Keyboard

This firmware exposes one USB composite device:

- a 48 kHz / 16-bit / mono USB microphone fed by an INMP441;
- a USB HID keyboard that holds Left Ctrl + Left GUI while K1 on D2 is held.

## Wiring

| INMP441 / button | XIAO ESP32S3 |
|---|---|
| VDD | 3V3 |
| GND | GND |
| L/R | GND (left channel) |
| SD | D8 / GPIO7 |
| SCK | D9 / GPIO8 |
| WS | D10 / GPIO9 |
| K1 | D2 / GPIO3, active-high |
| K2 | D1 / GPIO2, active-high (reserved, no action) |
| K3 | D0 / GPIO1, active-high (reserved, no action) |

The I2S receiver remains enabled continuously. Only its initial startup samples
are discarded, which avoids the all-zero second-recording failure caused by
repeatedly stopping and recreating the I2S/DMA channel.

On ESP32-S3, the I2S peripheral receives the two physical I2S slots in stereo
mode and the firmware extracts the left slot. This is required for an INMP441
whose L/R pin is tied to GND; forcing the S3 receiver into a single LEFT slot
can yield only 0/-1 samples.

## Operation

1. Put the text cursor in any editable field.
2. Hold K1. D2/GPIO3 uses its internal pull-down and the button drives it high.
3. Speak into the INMP441. The board presses Left Ctrl first, then Left Windows,
   and holds both while K1 is held, invoking the configured WeChat Input Method
   voice hotkey.
4. Release the button to release Left Windows first and then Left Ctrl, matching
   the event order of a physical keyboard.

The microphone runs as a standard 48 kHz, 16-bit, mono USB Audio 2.0 capture
device named `Voice Keyboard Audio`. The keyboard and microphone are one USB
composite device; no board or microphone selection is needed after Windows has
made this capture endpoint the default.

## Recovery / reflashing

The normal firmware replaces the USB serial port with the audio + keyboard
device. To enter the ROM downloader again, keep USB connected, hold BOOT, tap
RESET, then release BOOT. After flashing, tap RESET once without holding BOOT.
