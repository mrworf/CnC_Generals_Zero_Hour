# Synthetic audio fixtures

These tiny files contain generated sine waves and no retail content. They were
created with distribution FFmpeg using `sine` inputs, bit-exact mux/codec flags,
no copied metadata, mono 22050 Hz output, and the named encoders:

| File | Encoding | SHA-256 |
|---|---|---|
| `pcm.wav` | PCM signed 16-bit WAV | `b4a1d491c87929c9b2e566f43741d65691b59452c48948d74f6da0d3168524b0` |
| `ms-adpcm.wav` | Microsoft ADPCM WAV | `7cee0abdf2bfdfcb971beb02c5ec196c3364afcbea93dfc5407b11cf26fd6d31` |
| `ima-adpcm.wav` | IMA ADPCM WAV | `7eee51127e03fc25f3bf511fd315b2b03d95762489069137a199f7c4ff3338d2` |
| `sample.mp3` | MPEG Layer III, 64 kbit/s | `40b5e07576561d524c7f269891fd20c4db9a927a42a44d192e90f641099911e0` |

They are committed so ordinary tests require neither FFmpeg CLI execution nor a
network connection. FFmpeg remains a distribution dependency for the later
video path; it is not used by the miniaudio decoder tests.
