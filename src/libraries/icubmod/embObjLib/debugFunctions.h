// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
 *
 *
 * Copyright (C) 2010 RobotCub Consortium.
 *
 * Author: Alberto Cardellino
 *
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 *
 */

#ifndef __debugfunctions__
#define __debugfunctions__

// usual includes
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <signal.h>
#include <iostream>

// ACE stuff
#include <ace/ACE.h>
#include "ace/SOCK_Dgram.h"
#include "ace/Addr.h"
#include "ace/Thread.h"

// EO includes
#include "EOnv_hid.h"
#include "EOropframe_hid.h"
#include "embObjLibInterface.h"


#define _DEEP_DEBUG_

#ifdef _DEEP_DEBUG_

#define	BOARD_NUM					6
#define SOGLIA						1700
#define MAX_ACQUISITION 			100*1000


bool check_received_pkt(ACE_INET_Addr *sender_addr, void * pkt, ACE_UINT16 pkt_size);
void print_data(void);

static int timeval_subtract(struct timeval *_result, struct timeval *_x, struct timeval *_y)
{
	/* Perform the carry for the later subtraction by updating y. */

	if(_x->tv_usec < _y->tv_usec)
	{
		int nsec    = (_y->tv_usec - _x->tv_usec) / 1000000 + 1;

		_y->tv_usec -= 1000000 * nsec;
		_y->tv_sec  += nsec;
	}

	if(_x->tv_usec - _y->tv_usec > 1000000)
	{
		int nsec    = (_x->tv_usec - _y->tv_usec) / 1000000;

		_y->tv_usec += 1000000 * nsec;
		_y->tv_sec  -= nsec;
	}

	/* Compute the time remaining to wait. tv_usec is certainly positive. */

	_result->tv_sec  = _x->tv_sec  - _y->tv_sec;
	_result->tv_usec = _x->tv_usec - _y->tv_usec;

	/* Return 1 if result is negative. */

	return _x->tv_sec < _y->tv_sec;
}

#endif  //  _DEEP_DEBUG_

#endif  // __debugfunctions__