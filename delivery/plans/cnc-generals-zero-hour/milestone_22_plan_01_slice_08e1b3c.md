# M22 08E1B3C: generated decoded-frame stream-to-buffer contract

Compose the accepted original generic stream registry with the accepted
portable buffer using project-owned generated frame bytes only. Prove bounded
open/ready/decompress/render/advance/end/error order, type/extent/pitch
validation, allocation and render failure rollback, reset/retry, two
generations, provider removal and zero ownership. No Bink, W3D texture,
retail media, layout or pixel claim. Depends on 08E1B3A and 08E1B3B.

The generated stream produces a bounded single byte-frame sequence only. Its
observable order is `open -> ready -> decompress -> render -> next -> end ->
close`; no decoded media format is claimed. The probe rejects unknown names,
duplicate streams, unready/decompress or render-before-decompress, absent,
foreign, invalid or locked buffers, malformed frame length, injected render
failure, and stale streams/buffers. Original reset closes the stream after a
failure and permits a clean retry in each of two generations.
