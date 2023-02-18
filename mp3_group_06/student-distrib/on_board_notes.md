## 19 OCT CP2

----

kb driver; rtc driver; read-only FS driver

api: read/write/open/close for all drivers (can leave blank for now) -> map to syscall; return 0 for good and -1 for bad

t-kb driver: specified output on screen; ctrl+l = clrscrn; scrolling; cursor manipulate

kb writes to screen and buff at the same time

terminal reads from buffer on pressing enter

----

rtc - 2Hz; read() blocks until next interrupt; write() can change freq (p of 2)

helpful to virtualize - RTC can be set to different freqs and not effect program behavior. Set freq = 1024Hz and feed differently acccording to different programs (call rtc interrupt not on every r.e. but on numbers / every other)

when is the interrupt happening?

----

file sys

-appendix A; filesys_img; load on boot

flat. r-only; addr in kernel.c

boot: (4kB - 12B metadata - 52B reserve - 1 * 64B for dir entry)/64B per file entry = 62 files

!! do not assume contiguous datablocks



