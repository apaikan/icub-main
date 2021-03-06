/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
 * website: www.robotcub.org
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

/* @file       embObjAnaloSensor_callback.c
    @brief     This file keeps examples for ems / pc104 of the user-defined functions used for all endpoints in as
    @author     marco.accame@iit.it
    @date       05/02/2012
**/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h" 
#include "string.h"
#include "stdio.h"
#include "stdint.h"

#include "EoCommon.h"
#include "EoProtocol.h"
#include "EoProtocolAS.h"
#include "EOnv.h"

#include "EoProtocolAS_overridden_fun.h"

#include "FeatureInterface.h"



static void handle_data(FeatureType f_type, const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_fun_UPDT_as_strain_status_calibratedvalues(const EOnv* nv, const eOropdescriptor_t* rd)
{
    handle_data(AnalogStrain, nv, rd);
}

extern void eoprot_fun_UPDT_as_strain_status_uncalibratedvalues(const EOnv* nv, const eOropdescriptor_t* rd)
{
    handle_data(AnalogStrain, nv, rd);
}


extern void eoprot_fun_UPDT_as_mais_status_the15values(const EOnv* nv, const eOropdescriptor_t* rd)
{
    handle_data(AnalogMais, nv, rd);
}

// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static void handle_data(FeatureType f_type, const EOnv* nv, const eOropdescriptor_t* rd)
{
    FEAT_ID id;
    id.type = f_type;
    id.ep = eo_nv_GetEP8(nv);
    id.boardNum =  nvBoardNum2FeatIdBoardNum(eo_nv_GetBRD(nv));
    handle_AS_data(&id, rd->data, rd->id32);
}

// eof

