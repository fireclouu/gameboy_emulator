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

#include "include/host.hpp"

Host::Host(const string filePath)
{
  this->filePath = filePath;
  fileSize = 0;
}
bool Host::fileExist(string filePath)
{
  bool value = false;
  ifstream stream(filePath, ios::binary | ios::in);
  value = stream.is_open();
  stream.close();
  return value;
}
int Host::getFileSize(string filePath)
{
  int size = 0;
  ifstream stream(filePath, ios::binary | ios::in);
  if (stream.is_open())
  {
    stream.seekg(0, ios::end);
    size = stream.tellg();
  }

  stream.close();
  return size;
}

void Host::readFileContents(string filePath)
{
  ifstream stream;
  stream.open(filePath.c_str(), ios::binary | ios::in);
  if (stream.is_open())
  {
    while (stream.good())
    {
      stream.read(reinterpret_cast<char*>(romData), fileSize);
    }
  }
  stream.close();
}
bool Host::loadFile(const string filePath)
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
