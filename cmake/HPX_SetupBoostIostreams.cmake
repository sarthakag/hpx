# Copyright (c) 2019 The STE||AR-Group
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

if(NOT TARGET hpx::boost::iostreams)

  find_package(Boost ${Boost_MINIMUM_VERSION} QUIET MODULE COMPONENTS iostreams)

  if(Boost_IOSTREAMS_FOUND)
    hpx_info("  iostreams")
  else()
    hpx_error("Could not find Boost.Iostreams but HPX_WITH_COMPRESSION_BZIP2=On or \
    HPX_WITH_COMPRESSION_LIB=On. Either set it to off or provide a boost installation including \
    the iostreams library")
  endif()

  add_library(hpx::boost::iostreams INTERFACE IMPORTED)

  target_include_directories(hpx::boost::iostreams SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
  target_link_libraries(hpx::boost::iostreams INTERFACE ${Boost_IOSTREAMS_LIBRARIES})

endif()
