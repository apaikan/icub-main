/* 
 * Copyright (C) 2010 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Ugo Pattacini
 * email:  ugo.pattacini@iit.it
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

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <string>
#include <set>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>
#include <yarp/math/Math.h>

#include <iCub/ctrl/minJerkCtrl.h>
#include <iCub/ctrl/pids.h>
#include <iCub/utils.h>

#define GAZECTRL_SWOFFCOND_DISABLETIME      0.100   // [s]
#define GAZECTRL_MOTIONDONE_NECK_QTHRES     0.500   // [deg]
#define GAZECTRL_MOTIONDONE_EYES_QTHRES     0.100   // [deg]
#define GAZECTRL_MOTIONSTART_XTHRES         0.001   // [m]

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;
using namespace iCub::ctrl;
using namespace iCub::iKin;


// The thread launched by the application which is
// in charge of computing the velocities profile.
class Controller : public GazeComponent, public RateThread
{
protected:
    iCubHeadCenter   *neck;
    iKinChain        *chainNeck, *chainEyeL, *chainEyeR;
    PolyDriver       *drvTorso,  *drvHead;
    IPositionControl *posHead;
    IVelocityControl *velHead;
    exchangeData     *commData;
    xdPort           *port_xd;

    minJerkVelCtrl   *mjCtrlNeck;
    minJerkVelCtrl   *mjCtrlEyes;
    Integrator       *Int;    

    Port  port_x;
    Port  port_q;
    Port  port_event;
    Stamp txInfo_x;
    Stamp txInfo_q;
    Stamp txInfo_pose;

    Semaphore mutexChain;
    Semaphore mutexCtrl;
    Semaphore mutexData;
    unsigned int period;
    bool tiltDone;
    bool panDone;
    bool verDone;
    bool unplugCtrlEyes;
    bool Robotable;
    bool headV2;
    int nJointsTorso;
    int nJointsHead;
    double ctrlActiveRisingEdgeTime;
    double saccadeStartTime;
    double printAccTime;
    double neckTime;
    double eyesTime;
    double minAbsVel;
    double q_stamp;
    double Ts;

    Matrix lim;
    Vector q0deg,qddeg,qdeg,vdeg;
    Vector v,vNeck,vEyes,vdegOld;
    Vector qd,qdNeck,qdEyes;
    Vector fbTorso,fbHead,fbNeck,fbEyes;

    multiset<double> motionOngoingEvents;
    multiset<double> motionOngoingEventsCurrent;

    void notifyEvent(const string &event, const double checkPoint=-1.0);
    void motionOngoingEventsHandling();
    void motionOngoingEventsFlush();

public:
    Controller(PolyDriver *_drvTorso, PolyDriver *_drvHead, exchangeData *_commData,
               const double _neckTime, const double _eyesTime, const double _minAbsVel,
               const unsigned int _period);

    void   findMinimumAllowedVergence();
    void   minAllowedVergenceChanged();
    void   resetCtrlEyes();
    void   doSaccade(Vector &ang, Vector &vel);
    void   stopLimbsVel();
    void   set_xdport(xdPort *_port_xd) { port_xd=_port_xd; }
    void   printIter(Vector &xd, Vector &fp, Vector &qd, Vector &q, Vector &v, double printTime);
    bool   threadInit();
    void   afterStart(bool s);
    void   run();
    void   threadRelease();
    void   suspend();
    void   resume();
    double getTneck() const;
    double getTeyes() const;
    void   setTneck(const double execTime);
    void   setTeyes(const double execTime);
    bool   isMotionDone() const;
    void   setTrackingMode(const bool f);
    bool   getTrackingMode() const;
    bool   getDesired(Vector &des);
    bool   getVelocity(Vector &vel);
    bool   getPose(const string &poseSel, Vector &x, Stamp &stamp);
    bool   registerMotionOngoingEvent(const double checkPoint);
    bool   unregisterMotionOngoingEvent(const double checkPoint);
    Bottle listMotionOngoingEvents();
};


#endif


