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

int main(int argc, char **argv)
{
  // printf("%s\n", TITLE);

  // testpaths
  std::string testpaths[] = {
      "../testrom/01-special.gb",
      "../testrom/02-interrupts.gb",
      "../testrom/03-op sp,hl.gb",
      "../testrom/04-op r,imm.gb",
      "../testrom/05-op rp.gb",
      "../testrom/06-ld r,r.gb",
      "../testrom/07-jr,jp,call,ret,rst.gb",
      "../testrom/08-misc instrs.gb",
      "../testrom/09-op r,r.gb",
      "../testrom/10-bit ops.gb",
      "../testrom/11-op a,(hl).gb",
  };

  // init host
  Host *host = new Host(argc, argv);
  uint8_t *romData = NULL;

  if (host->loadFileOnArgument())
    romData = host->getRomData();
  else
    return 1;

  // init modules
  Cpu *cpu = new Cpu();
  Mmu *mmu = new Mmu(romData);

  // init system
  Gameboy *gameboy = new Gameboy(cpu, mmu);
  gameboy->start();
  return 0;
}
