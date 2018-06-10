# Altair Teletype

Emulates a teletype machine connected to an Altair 8800 using
[lib8080](https://github.com/GunshipPenguin/lib8080) for 8080 emulation.

Read my series of blog posts about this project and lib8080
[here](https://rhysre.net/2018/06/03/i8080-part-1.html).

## Building

This project uses CMake as its build system, to build, use:

```
cmake .
make
```

## Usage

```
altair_teletype -l <binary> <offset> ...
  -l <binary> <offset>
    Load the specified binary file at the specified offset
  -i <input device number>
    Set the device number for the I/O device (default: 1)
  -c <control device number>
    Set the device number for the control device (default: 0)
  -2
    Emulate an 88-2SIO I/O board (default: 88-SIO)
```

Examples:

To load 4K BASIC from a file called `4kbas32.bin` using the 88-SIO Serial I/O
card:

```
./altair_teletype -l ~/CLionProjects/lib8080/4kbas32.bin 0x00
```

To load a file called `a0.bin` at 0xA000 and a file called `f0.bin` at 0xF000
using input device 3, control device number 4 and the 88-2SIO Serial I/0 card.

```
./altair_teletype -2 -i 3 -c 4 -l a0.bin 0xF000 -l f0.bin 0xA000
```

## License

[MIT](https://github.com/GunshipPenguin/altair-teletype/blob/master/LICENSE) © Rhys Rustad-Elliott
