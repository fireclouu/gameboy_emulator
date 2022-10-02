/*
│* host.hpp
│* Copyright (C) 2022 fireclouu
│*
│* This program is free software: you can redistribute it and/or modify
│* it under the terms of the GNU General Public License as published by
│* the Free Software Foundation, either version 3 of the License, or
│* (at your option) any later version.
│*
│* This program is distributed in the hope that it will be useful,
│* but WITHOUT ANY WARRANTY; without even the implied warranty of
│* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
│* GNU General Public License for more details.
│*
│* You should have received a copy of the GNU General Public License
│* along with this program. If not, see <http://www.gnu.org/licenses/>.
│*/

#ifndef SRC_INCLUDE_HOST_HPP_
#define SRC_INCLUDE_HOST_HPP_
#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include "gameboy.hpp"

class Host {
 private:
  bool fileExist(std::string filePath);
  char **argv;
  char *filePath;
  int fileSize;
  int argc;
  Gameboy *gameboy;
  void handleUserArgument();
  void readFileContents(std::string filePath);
  int getFileSize(std::string filePath);

 public:
  explicit Host(int argc, char **argv, Gameboy *gameboy);
  void loadFile(std::string filePath);
  void loadFileOnArgument();
};

#endif  // SRC_INCLUDE_HOST_HPP_
