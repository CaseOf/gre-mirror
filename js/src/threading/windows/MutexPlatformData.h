/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef platform_win_MutexPlatformData_h
#define platform_win_MutexPlatformData_h

#include "jswin.h"

#include "threading/Mutex.h"

struct js::detail::MutexImpl::PlatformData
{
  SRWLOCK lock;
};

#endif // platform_win_MutexPlatformData_h
