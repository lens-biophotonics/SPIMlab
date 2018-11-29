# Find Qwt
# ~~~~~~~~
# Copyright (c) 2010, Tim Sutton <tim at linfiniti.com>
#  
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products 
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Once run this will define:
#
# Qwt_FOUND       = system has QWT lib
# Qwt_LIBRARY     = full path to the QWT library
# Qwt_INCLUDE_DIR = where to find headers
#


set(Qwt_LIBRARY_NAMES qwt-qt5 qwt6-qt5 qwt qwt6)

find_library(Qwt_LIBRARY
  NAMES ${Qwt_LIBRARY_NAMES}
  PATHS
    /usr/lib
    /usr/local/lib
    /usr/local/lib/qt5
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

set(_qwt_fw)
if(Qwt_LIBRARY MATCHES "/qwt.*\\.framework")
  string(REGEX REPLACE "^(.*/qwt.*\\.framework).*$" "\\1" _qwt_fw "${Qwt_LIBRARY}")
endif()

FIND_PATH(Qwt_INCLUDE_DIR NAMES qwt.h PATHS
  "${_qwt_fw}/Headers"
  /usr/include
  /usr/local/include
  /usr/local/include/qt5
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  PATH_SUFFIXES qwt-qt5 qwt qwt6
)

IF (Qwt_INCLUDE_DIR AND Qwt_LIBRARY)
  SET(Qwt_FOUND TRUE)
ENDIF (Qwt_INCLUDE_DIR AND Qwt_LIBRARY)

IF (Qwt_FOUND)
  FILE(READ ${Qwt_INCLUDE_DIR}/qwt_global.h qwt_header)
  STRING(REGEX REPLACE "^.*QWT_VERSION_STR +\"([^\"]+)\".*$" "\\1" Qwt_VERSION_STR "${qwt_header}")
  IF (NOT Qwt_FIND_QUIETLY)
    MESSAGE(STATUS "Found Qwt: ${Qwt_LIBRARY} (${Qwt_VERSION_STR})")
  ENDIF (NOT Qwt_FIND_QUIETLY)
ELSE (Qwt_FOUND)
  IF (Qwt_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Qwt")
  ENDIF (Qwt_FIND_REQUIRED)
ENDIF (Qwt_FOUND)
