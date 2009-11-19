/*******************************************************************************
 * Copyright (C) 2009 Christian Wressnegger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *******************************************************************************/

/**
 * @ingroup module
 *
 * \defgroup module_handCtrl abstractHandCtrl
 *
 * This module allows one to control the iCub's hand based on predefined motion configurations
 * similar to those generated by the robotMotorGui. Besides this it is also possible to directly
 * provide joint configurations for the hand using a external port.
 *
 *
 * \section lib_sec Libraries
 *
 * YARP (YARP_{OS,dev,sig,math})
 *   ACE
 * OpenVislab (libvislab, libvislab_YARP): http://OpenVislab.sf.net
 *
 * \section parameters_sec Parameters
 *
 * Command-line Parameters
 *
 * The following key-value pairs can be specified as command-line parameters by prefixing -- to the key
 * (e.g. --from conf.ini). The value part can be changed to suit your needs; the default values are shown below.
 *
 * --from <STRING>
 *   specifies the configuration file.
 *   default: "conf.ini"
 *
 * --context <STRING>
 *   specifies the sub-path from $ICUB_ROOT/icub/app to the configuration file.
 *   default: "eye2world"
 *
 * --name <STRING>
 *   specifies the name of the module (used to form the stem of module port names).
 *   default: "eye2world"
 *
 * --robot <STRING>
 *   specifies the name of the robot (used to form the root of robot port names).
 *   default: "icub"
 *
 *
 * Configuration File Parameters
 *
 * The following key-value pairs can be specified as parameters in the configuration file
 * (they can also be specified as command-line parameters).
 * The value part can be changed to suit your needs; the default values are shown below.
 *
 * --part <STRING>
 *   specifies name of the arm to use:.
 *   default: "right_arm"
 *
 * --handType <STRING>
 *   specifies the type of the hand of the iCub: "general" | "v1"
 *   default: "general"
 *
 * --motionSpec <FILE>
 *   specifies the file name of the configuration file containing the motions
 *   default: "motion_specification.ini"
 *
 * --sensingCalib <FILE>
 *   specifies the file name of the calibration file containing hand's the sensing constants.
 *   default: "object_sensing.ini"
 *
 * --control <PORT>
 *   specifies the communication port for communicating with the controlboard of the arm.
 *   default: /abstractHandCtrl/control
 *
 * --q <PORT>
 *   specifies the port to directly receive joint configurations as Vector(12) to control the hand.
 *   default: /q:i
 *
 * --iKin <PORT>
 *   specifies the rpc port of iKinArmCtrl in order to check the status of that module.
 *   default: /<PART>/rpc
 *
 *
 * \section portsa_sec Ports Accessed
 *
 * - /q:o
 *   The coordinates are expected to be wrapped in a Bottle object as double values:
 *   Bottle(double:yaw, double:roll, double:pitch)
 *
 * \section portsc_sec Ports Created
 *
 *  Input ports
 *
 *  - /abstractHandCtrl
 *    This port is used to change the parameters of the module at run time or stop the module
 *    The following commands are available:
 *    - set [ <id> <value> ]
 *      available options:
 *      - direct control (on/off)
 *        Enables/ Disables the direct control of the hand using the port specified above.
 *
 *      - *recording (on/off)
 *        Enables/ Disables the recording of the performed motion. This feature isn't available/
 *        ready to use yet and it is therefore disabled
 *
 *    - echo <str>
 *    - help
 *    - quit
 *
 *    Note that the name of this port mirrors whatever is provided by the --name parameter value
 *    The port is attached to the terminal so that you can type in commands and receive replies.
 *    The port can be used by other modules but also interactively by a user through the yarp rpc
 *    directive: yarp rpc /eye2world This opens a connection from a terminal to the port and allows
 *    the user to then type in commands and receive replies.
 *
 *
 * Output ports
 *
 *  - /eye2world
 *    see above
 *
 *
 * \section in_files_sec Input Data Files
 *
 * Sensing Constants:
 *
 * \code
 *	 thresholds    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
 *	 offsets       0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
 *	 springs       0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
 *	 springs       0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
 *	 springs       0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
 * 	 derivate_gain 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
 * \endcode
 *
 * Motion Specification
 * \code
 *   (...)
 *   [include X "y.ini"]
 *   (...)
 * \endcode
 * where X is the identifier of the motion which can be used for the "do" command and y is some
 * file name where the file should have a structure like the following:
 *
 * \code
 *   [POSITION0]
 *   jointPositions   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0
 *   jointVelocities 10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0
 *   timing 0.0
 *
 *   [POSITION1]
 *   jointPositions   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0
 *   jointVelocities 10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0  10.0
 *   timing 0.0
 *
 *   [DIMENSIONS]
 *   numberOfPoses   2
 *   numberOfJoints 16
 * \endcode
 * The number of "POSITION${x}" entries is controlled by the "numberOfPoses" value in the "DIMENSIONS"
 * section. The size of each vector in those section is specified by "numberOfJoints": 16 in this case.
 * For a more extensive description of the file format, check the documenation of the robotMotorGui.
 *
 * Right now only the position value are taken into account. The velocities an timing are supposed
 * to follow soon!
 *
 * \section out_data_sec Output Data Files
 *
 * None
 *
 * \section conf_file_sec Configuration Files
 *
 * conf.ini  in $ICUB_ROOT/icub/app/AbstractHandControl/
 *
 * \section tested_os_sec Tested OS
 *
 * most extensively on
 * Linux version 2.6.30-gentoo-r8 (gcc version 4.3.4 (Gentoo 4.3.4 p1.0, pie-10.1.5) )
 *
 * \section example_sec Example Instantiation of the Module
 *
 * abstractHandCtrl --part left_arm
 *                  --handType v1
 *                  --motionSpec /home/demo/res/motion_specification.ini"
 *                  --sensingCalib "/home/demo/res/object_sensing.ini"
 *
 *
 * \author Christian Wressnegger
 *
 * Copyright (C) 2009 Christian Wressnegger
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 */

#ifndef __ICUB_VISLAB_HANDCTRL_MODULE_H__
#define __ICUB_VISLAB_HANDCTRL_MODULE_H__

#include <vislab/util/common.h>
#include <vislab/yarp/util/all.h>

#include <iCub/vislab/Hand.h>
#include <iCub/vislab/HandMetrics.h>
#include <iCub/vislab/HandModule.h>

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>

namespace vislab {
namespace control {

/**
 *
 *
 * @author Christian Wressnegger
 * @date 2009
 */
class HandCtrl: public HandModule {

	struct PortIds {
		unsigned int Input_Q, RPC_iKin;
	} id;

	class DoCommand: public vislab::yarp::util::Command {
		virtual bool execute(const ::yarp::os::Bottle& params, ::yarp::os::Bottle& reply) const;
	public:
		DoCommand(HandCtrl* const parent) :
			Command(parent, "do",
					"Perform the motion specified for the given identifier: \"do POWER_GRASP\"") {
		}
	};
	friend class DoCommand;

	class RecordCommand: public vislab::yarp::util::Command {
		virtual bool execute(const ::yarp::os::Bottle& params, ::yarp::os::Bottle& reply) const;
	public:
		RecordCommand(HandCtrl* const parent) :
			Command(parent, "record", "Dis-/Enables the motion recorder: \"record on\" \"record off\"") {
		}
	};
	friend class RecordCommand;

	class DirectControlCommand: public vislab::yarp::util::Command {
		virtual bool execute(const ::yarp::os::Bottle& params, ::yarp::os::Bottle& reply) const;
	public:
		DirectControlCommand(HandCtrl* const parent) :
			Command(parent, "direct control",
					"Dis-/Enables the direct control of the arm: \"direct control on\" direct control off\"") {
		}
	};
	friend class DirectControlCommand;

	class BlockCommand: public vislab::yarp::util::Command {
		virtual bool execute(const ::yarp::os::Bottle& params, ::yarp::os::Bottle& reply) const;
	public:
		BlockCommand(HandModule* const parent) :
					Command(parent, "block",
							"Blocks the specified joints: \"block 1,2,3\". \"block\" gives you the list of blocked joints. ") {
		}
	};
	friend class BlockCommand;

	class UnBlockCommand: public vislab::yarp::util::Command {
		virtual bool execute(const ::yarp::os::Bottle& params, ::yarp::os::Bottle& reply) const;
	public:
		UnBlockCommand(HandModule* const parent) :
			Command(parent, "unblock", "Unblocks the specified joints: \"unblock 1,2,3\"") {
		}
	};
	friend class UnBlockCommand;

	class WorkerThread: public HandWorkerThread {
	private:
		struct PortIds id;

		::yarp::os::Semaphore mutex;
		::yarp::os::Port* iKinArmCtrl;
		void waitForIKinArmCtrl();
		bool iKinArmCtrlComm(const ::yarp::os::ConstString cmd);

		std::queue< ::yarp::os::ConstString> motionQueue;

	public:

		void doMotion(const ::yarp::os::ConstString type);

		WorkerThread(const vislab::yarp::util::OptionManager& moduleOptions,
				const vislab::yarp::util::Contactables& ports, const struct PortIds id,
				::yarp::os::Searchable& motionSpec, ::yarp::dev::PolyDriver& controlBoard,
				HandModule::HandType t);

		void run();
	};
	friend class WorkerThread;

	WorkerThread *workerThread;

public:

	/** The expected number of axes. */
	static const int numAxes = 16;

	/**
	 * The constructor.
	 */
	HandCtrl();
	/**
	 * The destructor.
	 */
	virtual ~HandCtrl();

	/**
	 * @see HandModule#configure(ResourceFinder)
	 */
	bool configure(::yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
	/**
	 * @see HandModule#updateModule()
	 */
	bool updateModule();
	/**
	 * @see HandModule#close()
	 */
	bool close(); // close and shut down the module
};

}
}

#endif // __ICUB_VISLAB_HANDCTRL_MODULE_H__
