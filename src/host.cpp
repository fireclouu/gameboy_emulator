/*
 * host.cpp
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

#include "../include/host.hpp"

Host::Host(int argc, char **argv)
{
  this->argc = argc;
  this->argv = argv;
  fileSize = 0;
  handleUserArgument();
}
void Host::handleUserArgument()
{
  while ((++argv)[0])
  {
    if (argv[0][0] == '-')
    {
      switch (argv[0][1])
      {
      case 'i':
        if ((argv[1] == nullptr) || std::string(argv[1]).empty())
        {
          printf("error: provide file path\n");
          exit(1);
        }
        else
        {
          filePath = argv[1];
        }
        break;
      default:
        printf("-%c: Unknown option\n", argv[0][1]);
        exit(1);
      }
    }
  }
}
bool Host::fileExist(std::string filePath)
{
  bool value = false;
  std::ifstream stream(filePath, std::ios::binary | std::ios::in);
  value = stream.is_open();
  stream.close();
  return value;
}
int Host::getFileSize(std::string filePath)
{
  int size = 0;
  std::ifstream stream(filePath, std::ios::binary | std::ios::in);
  if (stream.is_open())
  {
    stream.seekg(0, std::ios::end);
    size = stream.tellg();
  }

  stream.close();
  return size;
}

void Host::readFileContents(std::string filePath)
{
  std::ifstream stream;
  stream.open(filePath.c_str(), std::ios::binary | std::ios::in);
  if (stream.is_open())
  {
    while (stream.good())
    {
      stream.read(reinterpret_cast<char*>(romData), fileSize);
    }
  }
  stream.close();
}
bool Host::loadFile(const std::string filePath)
{
  if (filePath.empty())
  {
    printf("No file defined.\n");
    return false;
  }
  if (!fileExist(filePath))
  {
    printf("%s: File could not be found\n", filePath.c_str());
    return false;
  }
  if (!(fileSize = getFileSize(filePath)))
  {
    printf("%s: File size is invalid!\n", filePath.c_str());
    return false;
  }

  readFileContents(filePath);
  return true;
}
bool Host::loadFileOnArgument()
{
  return loadFile(filePath);
}

uint8_t* Host::getRomData() {
  return this->romData;
}
