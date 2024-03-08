/*
 * main.cpp
 * Copyright (C) 2022 fireclouu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "include/main.hpp"

int main(int argc, char **argv) {
    std::string testpaths[] = {
        "01-special.gb",
        "02-interrupts.gb",
        "03-op sp,hl.gb",
        "04-op r,imm.gb",
        "05-op rp.gb",
        "06-ld r,r.gb",
        "07-jr,jp,call,ret,rst.gb",
        "08-misc instrs.gb",
        "09-op r,r.gb",
        "10-bit ops.gb",
        "11-op a,(hl).gb",
    };
    Host *host = new Host(argc, argv);
    uint8_t *romData = NULL;
    if (host->loadFileOnArgument()) {
        romData = host->getRomData();
        // init modules
        Cpu *cpu = new Cpu();
        Mmu *mmu = new Mmu(romData);
        // // init system
        Gameboy *gameboy = new Gameboy(cpu, mmu);
        gameboy->start();
    } else {
      std::string dirPath = "gb-test-roms/cpu_instrs/individual/";
        for (int i = 0; i < 11; i++) {
          std::string testRomFilePath = dirPath + testpaths[i];
            if (host->loadFile(testRomFilePath)) {
                romData = host->getRomData();
            } else {
                continue;
            }
            // init modules
            Cpu *cpu = new Cpu();
            Mmu *mmu = new Mmu(romData);
            // init system
            Gameboy *gameboy = new Gameboy(cpu, mmu);
            gameboy->start();
        }
    }
    return 0;
}
