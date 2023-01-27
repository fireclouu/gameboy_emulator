/*
 * gameboy.cpp
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

#include "../include/gameboy.hpp"

// isMessagePassed
const char PASSED[] = {0x50, 0x61, 0x73, 0x73, 0x65, 0x64, 0x0a}; // PASSED\n
int passedCount = 0;
bool isPassed = false;
// get first line message
std::string buildMessage = "";
bool isInitialMessageFetched = false;
// islooping
uint16_t lastPc = 0;
uint8_t lastInstruction = 0;

bool isMessagePassed(char msg)
{
  // check if program is building PASSED string
  // and if it is, create custom message
  char passed = PASSED[passedCount];

  if (!isPassed)
  {
    if (msg == passed)
    {
      passedCount++;
      if (sizeof(PASSED) == passedCount)
        return true;
    }
    else
    {
      passedCount = 0;
    }
  }

  return false;
}

void fetchInitialMessage(char msg)
{
  if (msg != 0x0a && !isInitialMessageFetched)
  {
    buildMessage += msg;
  }
  else
  {
    if (!isInitialMessageFetched)
    {
      printf("TEST: %-40s", buildMessage.c_str());
      isInitialMessageFetched = true;
    }
  }
}

// check if pc is the same as pevious pc
// and instruction fetched is the same as previous one
bool isLooping(Cpu *cpu, Mmu *mmu)
{
  if (cpu->cpuRegister.pc == lastPc && (mmu->readByte(cpu->cpuRegister.pc) == lastInstruction) && lastInstruction == 0x18)
  {
    return true;
  }

  lastPc = cpu->cpuRegister.pc;
  lastInstruction = mmu->readByte(lastPc);
  return false;
}

Gameboy::Gameboy(Cpu *cpu, Mmu *mmu)
{
  halt = false;
  ime = false;

  this->cpu = cpu;
  this->mmu = mmu;
  cpu->setMmu(mmu);
  cpu->setHalt(&halt);
}

Gameboy::~Gameboy() {}

void Gameboy::handleInterrupt(uint16_t pc)
{
  if (ime)
  {
    uint8_t const IF = mmu->readByte(INTERRUPT_FLAG);
    uint8_t const IE = mmu->readByte(INTERRUPT_ENABLE);
    if (IE & IF)
    {
      // VBLANK
      cpu->instructionStackPush(pc + 1);
      cpu->cpuRegister.pc = INT_VBLANK;
    }
  }
}

// for blaarg test suite
void testSerialOutput(Mmu *mmu)
{
  if (mmu->readByte(0xff02) == 0x81)
  {
    char c = mmu->readByte(0xff01);
    // printf("%c", c);

    fetchInitialMessage(c);
    isPassed = isMessagePassed(c);

    mmu->writeByte(0xff02, 0);
  }
}

void testAutomation(Cpu *cpu, Mmu *mmu, bool *halt) 
{
  testSerialOutput(mmu);

  // check if looping endlessly
  if (isLooping(cpu, mmu))
  {
    std::string msg = isPassed ? "OK!" : "FAIL!";
    printf("%s\n", msg.c_str());
    *halt = true;
  }
}

void Gameboy::start() {
  // testvars
  // isMessagePassed
  passedCount = 0;
  isPassed = false;
  // get first line message
  buildMessage = "";
  isInitialMessageFetched = false;
  // islooping
  lastPc = 0;
  lastInstruction = 0;

  // debugger attach
  Debug *debug = NULL;
  // debug = new Debug(cpu, mmu);

  // initial setup
  cpu->cpuRegister.pc = 0x0100;
  cpu->cpuRegister.sp = 0xFFFE;
  cpu->cpuRegister.reg_a = 0x11;
  cpu->cpuRegister.reg_f = 0x80;
  cpu->cpuRegister.reg_b = 0x00;
  cpu->cpuRegister.reg_c = 0x00;
  cpu->cpuRegister.reg_pair_de = 0xFF56;
  cpu->cpuRegister.reg_pair_hl = 0x000D;

  while (!this->halt) {
    // debug
    if (debug != NULL) {
      debug->startDebug();
    }
    // serial automation
    testAutomation(cpu, mmu, &this->halt);
    // emulation
    uint16_t pc = cpu->cpuRegister.pc;
    uint8_t opcode = mmu->readByte(pc);
    cpu->decode(opcode);
  }
  if (debug != NULL) {
      debug->endDebug();
  }
}
