# P1010 - Camera Remote Shutter Project V2
#### Rev 2

This is an open-source camera remote shutter.

This was the second revision of the [original project](https://github.com/Electro707/camera_remote_shutter), but is now treated as a seperate project due to scope creep.

~~There are many issue with Rev 1, still WIP.~~
Currently working on Rev 2, which is smaller and fixed some issues with V1 (I think!)

## PCB Artwork Revision
As this project was initially the second revision of my original shutter controller, I put Rev2 as the rev on the PCB. As this is now a seperate project, it is really Rev 1.

## PCB Hardware Bugs
The following hardware bugs are present on the PCB, and are required to be re-worked before usage

- Rev 1 (Deprecated)
    - Op-Amps were flipped
- Rev 2
    - PA11 (Pin 22) and PA9 (Pin 19) should be flipped.

## Enclosure
WIP

There is a work-in-progress enclosure for the PCB under the [CAD](CAD) folder. The enclosure is made with FreeCAD 1.xx

## Firmware

WIP

### Compiling and Programming
To build any of the test or main applications, go into that directory and run the following:
```bash
mkdir -p build
cd build
cmake ../
make
```

To upload the program, run the following stlink command
```bash
st-flash write <file_to_flash>.bin 0x08000000
```

Where the <file_to_flash> is the bin file inside of the build folder. `0x08000000` is the starting flash address for the STM32G0 family.

## KiCAD 3D Models
The 3D models for some components in the directory `PCB/3d_model/` are not included due to licensing reasons. You can grab the step files yourself and put it in that directory from the manufacturer. The models are
- CUI_DEVICES_SJ-2509N.step
- 615006138421_Download_STP_615006138421_rev1.stp
- c-1-1478035-0-b-3d.stp
- pec12r-4025f-n0024.stp

# License
This project is licensed under GPLv3. See [LICENSE.md](LICENSE.md) for details.
