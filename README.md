<div class="myWrapper" markdown="1">
<h1>
  <div align="center">
  <p> Gameboy Emulator </p>  
  <img src="https://img.shields.io/badge/stability-wip-lightgrey.svg">
  <img src="https://img.shields.io/github/commit-activity/w/fireclouu/gbemu_v2">
  <img src="https://img.shields.io/github/repo-size/fireclouu/gbemu_v2">
  <img src="https://img.shields.io/github/last-commit/fireclouu/gbemu_v2">
  </div>
</h1>

### Overview
A personal project emulating Gameboy CPU and internals that runs on other systems. :video_game:
###### :bulb: Using C/C++ for flexible memory management feature.

___
### Building
Clone this repository:
``` bash
git clone --recursive https://github.com/fireclouu/gb_emu
```

and build using CMake. Use output binary as :
``` bash
gbemu -i {path/to/file}
```

If no parameters declared, it will automatically runs test provided by [retrio/gb-test-roms](https://github.com/retrio/gb-test-roms/tree/master), or you can explicitly do test mode:
``` bash
gbemu -t
```

It only supports individual test for now.

Sample output:

<img src="https://github.com/fireclouu/gbemu_v2/raw/7fc68e3c9c3d85031ae942650d81591d739847ce/blob/image_tests.png">

___
### :green_book: Status
- [x] Opcodes
- [ ] Timings
- [x] Common Tests
- [ ] Display
- [ ] Sound
- [ ] Interrupts


</div>
