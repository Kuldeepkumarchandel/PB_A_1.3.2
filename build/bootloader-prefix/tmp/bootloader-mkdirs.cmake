# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/chand/esp/v5.2.1/esp-idf/components/bootloader/subproject"
  "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader"
  "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader-prefix"
  "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader-prefix/tmp"
  "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader-prefix/src"
  "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Alwin_Projects/Pill-Box/PB_A_1.3.2/PB_A_1.3.2/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
