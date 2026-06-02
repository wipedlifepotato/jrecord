# a test program for record screen / streaming / ... from openbsd + ffmpeg
for usage you need to set sysctl kern.audio.record=1
after run sndiod like this: sndiod -f rsnd/1 -d -s default or sndiod -f rsnd/0 -d -s default.

# usage
```
Usage: ./jrecord [-f output_file] [-a audio_device] [-r if your voice is bad try to use -r 94100. for a while idk why so. need to think]
```
for example 

```
$ make
clang++ main.cpp -lsndio -o jrecord
$ ./jrecord -f ./record.mp4 -a snd/0 -r 94100 
open_record snd/0
 set_audio recorder 
ffmpeg version 8.0.1 Copyright (c) 2000-2025 the FFmpeg developers
  built with OpenBSD clang version 19.1.7
  configuration: --enable-shared --arch=amd64 --cc=cc --cxx=c++ --enable-debug --disable-stripping --disable-indev=jack --disable-vulkan --enable-fontconfig --enable-frei0r --enable-gpl --enable-ladspa --enable-libaom --enable-libass --enable-libdav1d --enable-libfontconfig --enable-libfreetype --enable-libfribidi --enable-libgsm --enable-libharfbuzz --enable-libmp3lame --enable-libopus --enable-libspeex --enable-libsvtav1 --enable-libtheora --enable-libv4l2 --enable-libvorbis --enable-libvpx --enable-libwebp --enable-libx264 --enable-libx265 --enable-libxml2 --enable-libxvid --enable-libzimg --enable-nonfree --enable-openssl --enable-libvidstab --extra-cflags='-I/usr/local/include -I/usr/X11R6/include' --extra-libs='-L/usr/local/lib -L/usr/X11R6/lib' --extra-ldsoflags= --mandir=/usr/local/man --objcc=/usr/bin/false --optflags='-O2 -pipe -g -Wno-redundant-decls'
  libavutil      60.  8.100 / 60.  8.100
  libavcodec     62. 11.100 / 62. 11.100
  libavformat    62.  3.100 / 62.  3.100
  libavdevice    62.  1.100 / 62.  1.100
  libavfilter    11.  4.100 / 11.  4.100
  libswscale      9.  1.100 /  9.  1.100
  libswresample   6.  1.100 /  6.  1.100
[x11grab @ 0xecba3815c00] Stream #0: not enough frames to estimate rate; consider increasing probesize
Input #0, x11grab, from ':0.0':
  Duration: N/A, start: 1780362613.510421, bitrate: 1988667 kb/s
  Stream #0:0: Video: rawvideo (BGR[0] / 0x524742), bgr0, 1920x1080, 1988667 kb/s, 29.97 fps, 1000k tbr, 1000k tbn, start 1780362613.510421
[aist#1:0/pcm_s16le @ 0xecba382b000] Guessed Channel Layout: mono
Input #1, s16le, from 'pipe:0':
  Duration: N/A, bitrate: 1505 kb/s
  Stream #1:0: Audio: pcm_s16le, 94100 Hz, mono, s16, 1505 kb/s
Stream mapping:
  Stream #0:0 -> #0:0 (rawvideo (native) -> h264 (libx264))
  Stream #1:0 -> #0:1 (pcm_s16le (native) -> aac (native))
[libx264 @ 0xecba3839400] using cpu capabilities: MMX2 SSE2Fast SSSE3 SSE4.2 AVX FMA3 BMI2 AVX2
[libx264 @ 0xecba3839400] profile High 4:4:4 Predictive, level 4.0, 4:4:4, 8-bit
[libx264 @ 0xecba3839400] 264 - core 165 - H.264/MPEG-4 AVC codec - Copyleft 2003-2025 - http://www.videolan.org/x264.html - options: cabac=0 ref=1 deblock=0:0:0 analyse=0:0 me=dia subme=0 psy=1 psy_rd=1.00:0.00 mixed_ref=0 me_range=16 chroma_me=1 trellis=0 8x8dct=0 cqm=0 deadzone=21,11 fast_pskip=1 chroma_qp_offset=6 threads=3 lookahead_threads=1 sliced_threads=0 nr=0 decimate=1 interlaced=0 bluray_compat=0 constrained_intra=0 bframes=0 weightp=0 keyint=250 keyint_min=25 scenecut=0 intra_refresh=0 rc=crf mbtree=0 crf=23.0 qcomp=0.60 qpmin=0 qpmax=69 qpstep=4 ip_ratio=1.40 aq=0
Output #0, mp4, to './record.mp4':
  Metadata:
    encoder         : Lavf62.3.100
  Stream #0:0: Video: h264 (avc1 / 0x31637661), yuv444p(tv, progressive), 1920x1080, q=2-31, 29.97 fps, 30k tbn
    Metadata:
      encoder         : Lavc62.11.100 libx264
    Side data:
      cpb: bitrate max/min/avg: 0/0/0 buffer size: 0 vbv_delay: N/A
  Stream #0:1: Audio: aac (LC) (mp4a / 0x6134706D), 96000 Hz, mono, fltp, 128 kb/s
    Metadata:
      encoder         : Lavc62.11.100 aac
[x11grab @ 0xecba3815c00] Could not get shared memory buffer.
[x11grab @ 0xecba3815c00] Continuing without shared memory.
^C[out#0/mp4 @ 0xecba382f200] video:2112KiB audio:115KiB subtitle:0KiB other streams:0KiB global headers:0KiB muxing overhead: 0.258303%
frame=   86 fps= 24 q=-1.0 Lsize=    2233KiB time=00:00:07.90 bitrate=2313.8kbits/s dup=0 drop=57 speed= 2.2x elapsed=0:00:03.58    
[libx264 @ 0xecba3839400] frame I:1     Avg QP:20.00  size:385363
[libx264 @ 0xecba3839400] frame P:85    Avg QP:16.08  size: 20904
[libx264 @ 0xecba3839400] mb I  I16..4: 100.0%  0.0%  0.0%
[libx264 @ 0xecba3839400] mb P  I16..4:  3.0%  0.0%  0.0%  P16..4:  9.2%  0.0%  0.0%  0.0%  0.0%    skip:87.8%
[libx264 @ 0xecba3839400] coded y,u,v intra: 27.4% 3.3% 3.0% inter: 5.1% 0.8% 0.8%
[libx264 @ 0xecba3839400] i16 v,h,dc,p: 54% 43%  2%  1%
[libx264 @ 0xecba3839400] kb/s:2169.09
[aac @ 0xecba381f400] Qavg: 21919.611
Exiting normally, received signal 2.
libc++abi: terminating
```

Control+C for stop record

for support:
XMR: 89huUbiqEw64tN6DdFN6vpdvVGm9WJzfJCJ3JDHc7bsri4bfcNoZAGBZmarThFXZnrPPAVZYo6fTYDyLSf8RpJ539Btm39o
