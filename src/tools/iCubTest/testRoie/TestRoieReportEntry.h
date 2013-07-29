// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/* 
 * Copyright (C) 2013 iCub Facility, Istituto Italiano di Tecnologia
 * Author: Marco Randazzo
 * email: marco.randazzo@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#ifndef __ICUB_TEST_ROIE_REPORT_ENTRY__
#define __ICUB_TEST_ROIE_REPORT_ENTRY__

#include "TestReportEntry.h"

class iCubTestRoieReportEntry : public iCubTestReportEntry
{
public:
    iCubTestRoieReportEntry(){}
    virtual ~iCubTestRoieReportEntry(){}

    virtual void print(XMLPrinter& printer);
    
    virtual void printStdio();

    std::string m_Name;
    std::string m_Result;

    std::string m_Cycles;
    std::string m_Variation;
    std::string m_Tolerance;
    std::string m_RefVel;
    std::string m_MinVal;
    std::string m_MaxVal;
};

#endif
