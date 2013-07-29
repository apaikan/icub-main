// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2008 RobotCub Consortium
 * Author: Lorenzo Natale
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */


#include "RobotInterfaceRemap.h"
#include "extractPath.h"

#include <sstream>

#include <yarp/os/Thread.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Os.h>

#include <iCub/FactoryInterface.h>

using namespace yarp::dev;
using namespace yarp::os;
using namespace std;

inline void printNoDeviceFound(const char *str)
{
    printf("==========\n");
    printf("Warning: skipping device %s, the automatic procedure failed to detect associated id on the CAN network.\n", str);
    printf("Possible causes could be:\n");
    printf("- The corresponding CAN device is not working\n");
    printf("- No valid descriptor was found on the firmware. Run the canLoader app and ");
    printf("set the 'additional info' string in the firmware so that it starts with %s (case sensitive).\n", str);
}

RobotNetworkEntry *RobotNetwork::find(const string &id)
{
    RobotNetworkIt it=begin();
    while(it!=end())
    {
        if ((*it)->id==id)
            return (*it);
        it++;
    }
    return 0;
}

RobotPartEntry::RobotPartEntry()
{
    iwrapper=0;
}

RobotPartEntry::~RobotPartEntry()
{
    close();
}

bool RobotPartEntry::open(Property &p)
{
    driver.open(p);
    if (!driver.isValid())
        return false;

    driver.view(iwrapper);

    if (iwrapper)
        return true;
    else
        return false;
}

void RobotPartEntry::close()
{
    if (driver.isValid())
    {
        iwrapper=0;
        driver.close();
    }
}

RobotPartEntry *RobotParts::find(const string &pName)
{
    RobotPartIt it=begin();
    for(;it!=end(); it++)
    {
        if ((*it)->id==pName)
            {
                return (*it);
            }
    }

    return 0;
}

SkinPartEntry::SkinPartEntry()
{
    analogServer=0;
    analog=0;
}

SkinPartEntry::~SkinPartEntry()
{

}

void SkinPartEntry::calibrate()
{
	if (analog)
	{
		analog->calibrateSensor();
	}
}

bool SkinPartEntry::open(yarp::os::Property &deviceP, yarp::os::Property &partP){
    bool correct=true;
    correct=correct&&partP.check("device");
    correct=correct&&partP.check("robot");
    correct=correct&&partP.check("canbusdevice");
    //correct=correct&&partP.check("ports");		// list of the ports where to send the tactile data
    
    if (!correct)
        return false;

    int period=20;
    if (partP.check("period"))
    {
        period=partP.find("period").asInt();
    }
    else
    {
        std::cout<<"Warning: part "<<id<<" using default period ("<<period<<")\n";
    }

	// Open the device
    std::string devicename=partP.find("device").asString().c_str();
    deviceP.put("device", devicename.c_str());

    std::string canbusdevice=partP.find("canbusdevice").asString().c_str();
    deviceP.put("canbusdevice", canbusdevice.c_str());

    std::string physdevice=partP.find("physdevice").asString().c_str();
    deviceP.put("physdevice", physdevice.c_str());

    driver.open(deviceP);
    if (!driver.isValid())
        return false;

    driver.view(analog);

    if (!analog)
    {
        std::cerr<<"Error: part "<<id<<" device " << devicename << " does not implement analog interface"<<endl;
        driver.close();
        return false;
    }

	// Read the list of ports
    std::string robotName=partP.find("robot").asString().c_str();
    std::string root_name;
    root_name+="/";
    root_name+=robotName;
    root_name+="/skin/";

	std::vector<TBR_AnalogPortEntry> skinPorts;
	if(!partP.check("ports")){	
		// if there is no "ports" section take the name of the "skin" group as the only port name
		skinPorts.resize(1);
		skinPorts[0].offset = 0;
		skinPorts[0].length = -1;
		skinPorts[0].port_name = root_name + this->id;
	}
	else{
		Bottle *ports=partP.find("ports").asList();

		if (!partP.check("total_taxels", "number of taxels of the part"))
			return false;
		int total_taxels=partP.find("total_taxels").asInt();
		int nports=ports->size();
		int totalT = 0;
		skinPorts.resize(nports);
	    
		for(int k=0;k<ports->size();k++)
		{
			Bottle parameters=partP.findGroup(ports->get(k).asString().c_str());

			if (parameters.size()!=5)
			{
				cerr<<"Error: check skin port parameters in part description"<<endl;
				cerr<<"--> I was expecting "<<ports->get(k).asString().c_str() << " followed by four integers"<<endl;
				return false;
			}

			int wBase=parameters.get(1).asInt();
			int wTop=parameters.get(2).asInt();
			int base=parameters.get(3).asInt();
			int top=parameters.get(4).asInt();

			cout<<"--> "<<wBase<<" "<<wTop<<" "<<base<<" "<<top<<endl;

			//check consistenty
			if(wTop-wBase != top-base){
				cerr<<"Error: check skin port parameters in part description"<<endl;
				cerr<<"Numbers of mapped taxels do not match.\n";
				return false;
			}
			int taxels=top-base+1;

			skinPorts[k].length = taxels;
			skinPorts[k].offset = wBase;
			skinPorts[k].port_name = root_name+string(ports->get(k).asString().c_str());
	        
			totalT+=taxels;
		}

		if (totalT!=total_taxels)
		{
			cerr<<"Error total number of mapped taxels does not correspond to total taxels"<<endl;
			return false;
		}
	}

    analogServer = new AnalogServer(skinPorts);
    analogServer->setRate(period);
    analogServer->attach(analog);
    analogServer->start();

    return true;
}

void SkinPartEntry::close()
{
    std::cout<<"Closing skin part "<< id << endl;
    if (analogServer)
    {
        analogServer->stop();
        delete analogServer;
    }
    if (analog)
        analog=0;

    driver.close();
}

SkinPartEntry *SkinParts::find(const string &pName)
{
    SkinPartsIt it=begin();
    for(;it!=end(); it++)
    {
        if ((*it)->id==pName)
            {
                return (*it);
            }
    }

    return 0;
}

void SkinParts::close()
{
    SkinPartsIt it=begin();
    while(it!=end())
    {
       (*it)->close();
       it++;
    }
}


// implementation of the RobotInterfaceRemap class

RobotInterfaceRemap::RobotInterfaceRemap() 
{
    gyro_i = 0; 

    initialized = false; 

    isParking=false;
    isCalibrating=false;
    abortF=false;

    #ifdef _USE_INTERFACEGUI
    mServerLogger=NULL;
    #endif
}  

RobotInterfaceRemap::~RobotInterfaceRemap()
{
    #ifdef _USE_INTERFACEGUI
    if (mServerLogger)
    {
        iCubInterfaceGuiServer *deleting=mServerLogger;
        mServerLogger=NULL;

        deleting->stop();
        delete deleting;
    }
    #endif
}

void RobotInterfaceRemap::park(bool wait)
{
    std::cout<<"RobotInterfaceRemap::park\n";

    if (abortF)
        return;

    isParking=true;

    RobotNetworkIt it=networks.begin();
    while(it!=networks.end())
    {
        (*it)->startPark();
        it++;
    }

    it=networks.begin();
    if (wait)
    {
        while(it!=networks.end())
        {
            (*it)->joinPark();
            it++;
        }
    }

    isParking=false;
}    

void RobotInterfaceRemap::calibrate(bool wait)
{
    if (abortF)
        return;

    isCalibrating=true;

    RobotNetworkIt it=networks.begin();
    while(it!=networks.end())
    {
        (*it)->startCalib();
        it++;
    }

    it=networks.begin();
    if(wait)
    {
        while(it!=networks.end())
        {
            (*it)->joinCalib();
            it++;
        }
    }

    isCalibrating=false;
}    

bool RobotInterfaceRemap::initialize(const std::string &inifile)
{
    std::string filename;
    std::string portname;

    std::string PATH;

    PATH=extractPath(inifile.c_str());

    Property robotOptions;
    printf("Read robot description from %s\n", inifile.c_str());
    robotOptions.fromConfigFile(inifile.c_str());

    if (robotOptions.findGroup("GENERAL").check("automaticIds"))
    {
        Value &v=robotOptions.findGroup("GENERAL").find("automaticIds");

        //try to read as list
        Bottle *pNetList=v.asList();

        Bottle netList;
        if (pNetList==0)
            netList.addString(v.asString().c_str());     //read as single string
        else
            netList=*pNetList;

        std::cout<<"Starting automatic detection of can ids"<<endl;
        std::cout<<"Warning: error messages during this procedure can be ignored"<<endl;

        for(int m=0; m<netList.size() ;m++)
        {
            std::string device=netList.get(m).asString().c_str();
            std::cout<<"I'm probing the icub can network on "<<device<< " this might take a few seconds..." << endl;
            
            ICUB_CAN_IDS tmpIds=idDisc.discover(device.c_str());
            can_ids.push(device, tmpIds);

            int found=tmpIds.size();
            std::cout<<"Found " << found << " networks" << endl;
            ICUB_CAN_IDS::CanIdIterator it=tmpIds.canIdList.begin();
            while(it!=tmpIds.canIdList.end())
            {
                cout << "On " << device << " network " << (*it).key << " has id " << (*it).id << endl;
                it++;
            }

            automaticIds=true;
        }

        std::cout<<"Terminated automatic detection of can ids"<<endl;
    }
    else
    {
        printf("Using can ids from ini file.\n");
        automaticIds=false;
    }

    if (robotOptions.check("fileformat"))
    {
        float format=static_cast<float>(robotOptions.find("fileformat").asDouble());
        if (format==2.0)
            return initialize20(inifile);
        else
        {
            std::cerr<<"Configuration file unrecognized format\n";
            return false;
        }
    }
    else
        return initialize10(inifile);

}

bool RobotInterfaceRemap::initCart(const::string &file)
{
    Property options;
    options.fromConfigFile(file.c_str());

    int nDrivers=options.findGroup("GENERAL").find("NumberOfDrivers").asInt();

    std::cout<<"Initializing controller with parts: ";
    yarp::dev::PolyDriverList plist;
    bool valid=true;
    for(int k=0; k<nDrivers; k++)
    {
        std::string drGroup;
        std::ostringstream tmpStr;
        tmpStr << k;
        drGroup=std::string("DRIVER_").append(tmpStr.str());
        std::string part=options.findGroup(drGroup.c_str()).find("Key").asString().c_str();

        std::cout<<part;
        std::cout<<",";
    
        RobotPartEntry *robPart=parts.find(part);
        if (robPart)
        {
             plist.push(&robPart->driver, part.c_str());
        }        
        else
        {
            valid=false;
        }
    }
    
    std::cout<<"\n";
    
    if (valid)
    {
        CartesianController *cart=new CartesianController;
        options.put("device", "cartesiancontrollerserver");
        if (cart->driver.open(options))
            {
                cart->driver.view(cart->iwrapper);
                cart->iwrapper->attachAll(plist);
                cartesianControllers.push_back(cart);
            }
        else
            {
                std::cerr<<"Sorry, could not open cartesian controller\n";
                return false;
            }
    }
    else
    {   
        std::cerr<<"Sorry, could not detect all devices required by controller\n";
        return false;
    }
    
    return true;
}

bool RobotInterfaceRemap::finiCart()
{
    CartesianControllersIt it=cartesianControllers.begin();

    while(it!=cartesianControllers.end())
    {
        (*it)->close();
        it++;
    }
    return true;
}

bool RobotInterfaceRemap::initialize10(const std::string &inifile)
{
    fprintf(stderr, "Going to initialize the robot with a file\n");

    std::string PATH;
    PATH=extractPath(inifile.c_str());

    Property robotOptions;
    fprintf(stderr, "Read robot description from %s\n", inifile.c_str());
    robotOptions.fromConfigFile(inifile.c_str());

    robotName=robotOptions.findGroup("GENERAL").find("name").asString().c_str();
    Bottle *pNets=robotOptions.findGroup("GENERAL").find("networks").asList();
    Bottle nets;
    if (pNets==0)
    {
        // This is to maintain compatibility with old ini files
        fprintf(stderr, "Warning parsing %s could not find a valid network description\n",inifile.c_str());
        fprintf(stderr, "Assuming old style ini file\n");
        nets.fromString("HEAD RIGHT_ARM LEFT_ARM LEGS");
    }
    else
    {
        nets=*pNets;
    }

    int nnets=nets.size();
    std::cout<<"Using " <<nnets <<" networks"<<endl;

    //std::cout<<robotOptions.toString()<<endl;

    int n=0;
    for(n=0;n<nnets;n++)
    {
        std::string netid=nets.get(n).asString().c_str();
        RobotNetworkEntry *netEntry;
        if (robotOptions.check(netid.c_str()))
        {
            netEntry=new RobotNetworkEntry;

            Property tmpProp;
            tmpProp.fromString(robotOptions.findGroup(netid.c_str()).toString());

            if (!instantiateNetwork(PATH, tmpProp, *netEntry))
            {
                std::cerr<< "Troubles instantiating "<< netid.c_str() << endl;
                delete netEntry;   
            }
            else
            {
                netEntry->id=netid;
                networks.push_back(netEntry);

                Bottle *partsList=robotOptions.findGroup(netid.c_str()).find("parts").asList();
                if (partsList!=0)
                {
                    for(int p=0;p<partsList->size();p++)
                    {
                        Property tmpProp;
                        tmpProp.fromString(robotOptions.findGroup(partsList->get(p).asString()).toString());
                        tmpProp.put("device", "controlboardwrapper");
                        std::string prefix=robotName;
                        prefix+="/";
                        prefix+=partsList->get(p).asString();
                        tmpProp.put("name", prefix.c_str());
                        std::cout<<"-->"<<tmpProp.toString()<<endl;

                        RobotPartEntry *partEntry=new RobotPartEntry;
                        partEntry->id=partsList->get(p).asString().c_str();

                        if (partEntry->open(tmpProp))
                        {
                            PolyDriverList p;
                            p.push(&netEntry->driver, "");
                            partEntry->iwrapper->attachAll(p);
                            parts.push_back(partEntry);
                        }
                    }
                }
                else
                {
                    cout<<"No part list specified exporting whole device"<<endl;
                    Property tmpProp;
                    std::string prefix=robotName;
                    prefix+="/";
                    prefix+=netid.c_str();

                    //this is to maintain compatibility with old ini file/code
                    for(unsigned int k=0;k<prefix.length();k++)
                    {
                        prefix[k]=tolower(prefix[k]);
                    }
                    ///

                    tmpProp.put("name", prefix.c_str());
                    tmpProp.put("device", "controlboardwrapper");
                    std::cout<<"--> " << tmpProp.toString()<<endl;
                    RobotPartEntry *partEntry=new RobotPartEntry;
                    partEntry->id=netid;
 
                    if (partEntry->open(tmpProp))
                        {
                            PolyDriverList p;
                            p.push(&netEntry->driver, netEntry->id.c_str());
                            partEntry->iwrapper->attachAll(p);
                            parts.push_back(partEntry);
                        }
                }
            }
        }
        else
        {
            std::cout<<"not found, skipping"<<endl;
            netEntry=0;
        }
    }

    fprintf(stderr, "RobotInterface::now opening inertial\n");
    if (robotOptions.check("INERTIAL")) 
    {
        Property tmpProp;
        //copy parameters verbatim from relative section
        tmpProp.fromString(robotOptions.findGroup("INERTIAL").toString());
        fprintf(stderr, "RobotInterface:: inertial sensor is in the conf file\n");
        if (!instantiateInertial(PATH, tmpProp))
            fprintf(stderr, "RobotInterface::warning troubles instantiating inertial sensor\n");
    }
    else
        fprintf(stderr, "RobotInterface::no inertial sensor defined in the config file\n");

    std::cout<<"Starting robot calibration!"<<endl;
    calibrate();
    std::cout<<"Finished robot calibration!"<<endl;

    return true;
}

bool RobotInterfaceRemap::initialize20(const std::string &inifile)
{
    fprintf(stderr, "Initialization from file, new version 2.0\n");

    std::string PATH;
    PATH=extractPath(inifile.c_str());

    Property robotOptions;
    fprintf(stderr, "Read robot description from %s\n", inifile.c_str());
    robotOptions.fromConfigFile(inifile.c_str());

    robotName=robotOptions.findGroup("GENERAL").find("name").asString().c_str();
    Bottle *reqParts=robotOptions.findGroup("GENERAL").find("parts").asList();
    int nparts=0;
    if (reqParts!=0)
        nparts=reqParts->size();
   
    std::cout<<"Found " << nparts <<" parts"<<endl;

    //std::cout<<robotOptions.toString()<<endl;

    int n=0;
    for(n=0;n<nparts;n++)
    {
        std::string partid=reqParts->get(n).asString().c_str();
        std::cout<<"--> Processing "<<partid<<endl;

        RobotPartEntry *partEntry=new RobotPartEntry;
        partEntry->id=partid;

        Bottle *nets=robotOptions.findGroup(partid.c_str()).find("networks").asList();
        if (nets==0)
        {
            std::cerr<< "Error, missing network list in inifile for part " << partid << endl;
            std::cerr<< "Should be something like networks (net1 net2)"<<endl;
            return false;
        }

        for(int n=0;n<nets->size();n++)
        {
            std::string netId=nets->get(n).asString().c_str();
            RobotNetworkEntry *net=networks.find(netId);

            if (!net)
            {
                //create it and push into networks
                net=new RobotNetworkEntry;
                net->id=netId;
                networks.push_back(net);
            }

            //add to list of networks for part
            partEntry->push(net);   
        }

        //add to list of parts
        parts.push_back(partEntry);
    }

    //now go through list of networks and initialize it

    RobotNetworkIt netit=networks.begin();
    while(netit!=networks.end())
    {
        RobotNetworkEntry *netEntry=(*netit);
        std::string netid=netEntry->id;

        Property tmpProp;
        tmpProp.fromString(robotOptions.findGroup(netid.c_str()).toString());

        cout << "Instantiating network " << netid.c_str() << "...";
        if (!instantiateNetwork(PATH, tmpProp, *netEntry))
        {
            cerr << endl << "ERROR: troubles instantiating " << netid.c_str() << endl;
        }
        else
            cout << "Network "<< netid.c_str() << " instantiated correctly"<< endl;

        netit++;
    }

    std::cout<<"--> Starting robot calibration!"<<endl;
    calibrate();
    std::cout<<"Finished robot calibration!"<<endl;

    //now iterate through list of parts to see if all networks have been created correctly
    std::cout<<"--> Now I will go through the list of parts to create the wrappers"<<endl;

    RobotPartIt partit=parts.begin();

    while(partit!=parts.end())
    {
        RobotPartEntry *tmp=*partit;    

        //create the wrappers
        Property tmpProp;
        //copy parameters verbatim from relative section
        tmpProp.fromString(robotOptions.findGroup(tmp->id.c_str()).toString());

        //add device name
        tmpProp.put("device", "controlboardwrapper2");
        //append robot name
        std::string prefix=robotName;
        prefix+="/";
        prefix+=tmp->id.c_str();
        tmpProp.put("name", prefix.c_str());
        //open wrapper
        std::cout<<"Opening wrapper for " << tmp->id << endl;
        //std::cout<<"Parameter list:"<<endl;
        //std::cout<<tmpProp.toString();
        //std::cout<<endl;

        if (!tmp->open(tmpProp))
        {
            partit++;
            continue;
        }

        RobotNetworkIt netIt=tmp->networks.begin();

        //now attach all network devices
        PolyDriverList polylist;
        while(netIt!=tmp->networks.end())
        {
            RobotNetworkEntry *net=(*netIt);

            if (net->isValid())
            {
                std::cout<<"Attaching " << net->id.c_str() << endl;
                polylist.push(&net->driver, net->id.c_str());
                //tmp->wrapper.attach(&net->driver, net->id.c_str()); 
            }
            else
            {
                std::cout<<"Skipping " << net->id.c_str() << " for part " << tmp->id << endl;
            }
            netIt++;
        }

        tmp->iwrapper->attachAll(polylist);

        std::cout<<endl;
        partit++;
    }

    fprintf(stderr, "RobotInterface::now opening inertial\n");
    if (robotOptions.check("INERTIAL")) 
    {
        Property tmpProp;
        //copy parameters verbatim from relative section
        tmpProp.fromString(robotOptions.findGroup("INERTIAL").toString());
        fprintf(stderr, "RobotInterface:: inertial sensor is in the conf file\n");
        if (!instantiateInertial(PATH, tmpProp))
            fprintf(stderr, "RobotInterface::warning troubles instantiating inertial sensor\n");
    }
    else
    {
        fprintf(stderr, "RobotInterface::no inertial sensor defined in the config file\n");
    }



    // now go thourgh list of networks and create analog interface
    Bottle *analogNets=robotOptions.findGroup("GENERAL").find("analog").asList();
    std::cout<<"--> Checking if I need to create analog wrappers"<<std::endl;
    if (analogNets)
    {       
        int nanalog=analogNets->size();

        int n=0;
        for(n=0;n<nanalog;n++)
        {
            std::string analogid=analogNets->get(n).asString().c_str();

            std::string netid=robotOptions.findGroup(analogid.c_str()).find("network").asString().c_str();
            int period=20;
            if (robotOptions.findGroup(analogid.c_str()).check("period")) 
                period = robotOptions.findGroup(analogid.c_str()).find("period").asInt();
            else
                std::cout<<"Warning: could not find period using default value ("<<period<<")\n";

            Bottle *analogIds=robotOptions.findGroup(analogid.c_str()).find("deviceId").asList();
            if (analogIds!=0)
                for(int k=0;k<analogIds->size();k++)
                {
                    std::string deviceId=analogIds->get(k).asString().c_str();
                    std::cout<<"Instantiating analog device " << deviceId << " on "<< netid << endl;

                    RobotNetworkEntry *selectedNet=networks.find(netid);
                    if (selectedNet==0)
                    {
                        std::cerr<<"Sorry "<<netid<<" has not been instantiated, skipping"<<endl;
                    }
                    else
                    {
                        DeviceDriver *dTmp;
                        yarp::dev::IFactoryInterface *iFactory;
                        selectedNet->driver.view(iFactory);
                        if (iFactory==0)
                        {
                            std::cout<<"CanBus device does not support iFactory interface\n";
                        }
                        else
                        {
                            yarp::os::Property prop;
                            prop.put("device", "analog");
                            prop.put("deviceid", deviceId.c_str());
                            dTmp=iFactory->createDevice(prop);

                            IAnalogSensor *iTmp=dynamic_cast<IAnalogSensor *>(dTmp);

                            selectedNet->analogSensors.push_back(iTmp);
                            if (iTmp)
                            {
                                std::string name;
                                name+="/";
                                name+=robotName;
                                name+="/";
                                name+=deviceId.c_str();
                                name+="/analog:o";

                                AnalogServer *tmp=new AnalogServer(name.c_str());
                                tmp->setRate(period);
                                tmp->attach(iTmp);
                                tmp->start();

                                selectedNet->analogServers.push_back(tmp);
                            }
                        }
                    }
                }
        }
    }
    else
    {
        std::cout<<"No analog wrappers requested\n";
    }


    Bottle *skinParts=robotOptions.findGroup("GENERAL").find("skinParts").asList();
    std::cout<<"--> Checking if I need to create skin parts"<<std::endl;
    if (skinParts)
    {       
        int nskin=skinParts->size();
        cout<< "I have found " << nskin << " parts\n";
        int n=0;
        for (n=0;n<nskin;n++)
        {
            std::string partId=skinParts->get(n).asString().c_str();
            std::cout<<"Opening " << partId << "\n";

            Property partOptions;
            partOptions.fromString(robotOptions.findGroup(partId.c_str()).toString());
            partOptions.put("robot", robotName.c_str());

            SkinPartEntry *tmp=new SkinPartEntry;
            tmp->setId(partId);

            if (partOptions.check("file"))
            {
                std::string filename=PATH+partOptions.find("file").asString().c_str();

                Property deviceParams;
                deviceParams.fromConfigFile(filename.c_str());

                if (tmp->open(deviceParams, partOptions))
                {
                    skinparts.push_back(tmp);
                }
                else
                {
                    std::cerr<<"Error instantiating skin part " << partId << "check parameters"<<endl;
                    delete tmp;
                }
            }
        }
    }

    #ifdef _USE_INTERFACEGUI
    mServerLogger=new iCubInterfaceGuiServer;
    mServerLogger->config(PATH,robotOptions);

    for (RobotNetworkIt netIt=networks.begin(); netIt!=networks.end(); ++netIt)
    {
        IClientLogger* pCL=NULL;
        (*netIt)->driver.view(pCL);

        if (pCL)
        {
            pCL->setServerLogger(mServerLogger);
        }
    }

    for (std::list<SkinPartEntry*>::iterator skinIt=skinparts.begin(); skinIt!=skinparts.end(); ++skinIt)
    {
        IClientLogger* pCL=NULL;
        (*skinIt)->driver.view(pCL);

        if (pCL)
        {
            pCL->setServerLogger(mServerLogger);
        }
    }

    mServerLogger->start();
    #endif

    return true;
}

bool RobotInterfaceRemap::instantiateNetwork(std::string &path, Property &robotOptions, RobotNetworkEntry &net)
{
    std::string file=robotOptions.find("file").asString().c_str();
    std::string fullFilename;
    fullFilename=path;
    if (fullFilename.length()!=0 && fullFilename[fullFilename.length()-1]!='/')
    {
        fullFilename.append("/",1);
    }
    fullFilename.append(file.c_str(), file.length());

    Property deviceParameters;
    deviceParameters.fromConfigFile(fullFilename.c_str());

    Value &device=robotOptions.find("device");
    Value &subdevice=robotOptions.find("subdevice");
    Value &candevice=robotOptions.find("canbusdevice");
    Value &physdevice=robotOptions.find("physdevice");

    deviceParameters.put("robotName",robotName.c_str());
    deviceParameters.put("device", device);
    deviceParameters.put("subdevice", subdevice);
    deviceParameters.put("canbusdevice",candevice);
    deviceParameters.put("physdevice",physdevice);

    ICUB_CAN_IDS *ids=can_ids.find(candevice.asString().c_str());

    int networkN=deviceParameters.findGroup("CAN").find("CanDeviceNum").asInt();
    if (ids)
    {
        string netid=deviceParameters.findGroup("CAN").find("NetworkId").asString().c_str();

        //std::cerr<<"Netid:"<<netid<<endl;
        //overwriting net id
        networkN=(*ids)[netid];
        if (networkN!=-1)
        {
            forceNetworkId(deviceParameters, networkN); 
        }
        else
        {
            std::cerr<<"Error: requested automatic ids but no id was found for network " << netid << endl;
            return false;
        }
    }

    if (robotOptions.check("calibrator"))
    {
        Value &calibrator=robotOptions.find("calibrator");
        Property pTemp;
        pTemp.fromString(deviceParameters.toString());
        pTemp.put("device",calibrator.toString());
        net.calibrator.open(pTemp);
    }

    if (robotOptions.check("verbose")) 
        deviceParameters.put("verbose", 1);

    std::cout<<"Opening network " << networkN << " on device " << device.asString().c_str() << "... ";

    net.driver.open(deviceParameters);

    if (!net.driver.isValid())
    {
        std::cout<<"failed!"<<endl;
        return false;
    }
    std::cout<<"done!"<<endl;

    ICalibrator *icalibrator;
    IControlCalibration2 *icalib;
    //acquire calibrator int
    net.calibrator.view(icalibrator);
    //acquire calibration int
    net.driver.view(icalib);

    //save interface for later use
    net.iCalib=icalib;
    //set calibrator
    net.iCalib->setCalibrator(icalibrator);

    return true;
} 

bool RobotInterfaceRemap::instantiateInertial(Property &options)
{
    fprintf(stderr, "Instantiating an INERTIAL device\n");

	const char *conf = yarp::os::getenv("ICUB_ROOT");
    robotName=options.findGroup("GENERAL").find("name").asString().c_str();

    //////////// inertial
    Value &device=options.findGroup("INERTIAL").find("device");
    Value &subdevice=options.findGroup("INERTIAL").find("subdevice");
    Value &inifile=options.findGroup("INERTIAL").find("file");

    std::string fullFilename;
    std::string portName;
    fullFilename+=conf;
    fullFilename+="/conf/";
    fullFilename+=inifile.asString().c_str();

    Property p;
    p.fromConfigFile(fullFilename.c_str());

    p.put("device", device);
    p.put("subdevice", subdevice);

    portName+="/";
    portName+=robotName.c_str();
    portName+="/inertial";
    p.put("name", portName.c_str());

    // create a device for the arm 
    gyro.open(p);
    if (!gyro.isValid()) 
    {  
        return false;
    } 

    bool ok = gyro.view(gyro_i);

    return ok;
}


bool RobotInterfaceRemap::instantiateInertial(const std::string &path, Property &options)
{
    //    std::cout<<"Path: "<<path<<" Property list: "<<options.toString().c_str();
    std::string file=options.find("file").asString().c_str();
    std::string device=options.find("device").asString().c_str();
    std::string subdevice=options.find("subdevice").asString().c_str();

    std::string fullFilename;
    fullFilename=path;
    if (fullFilename.length()!=0 && fullFilename[fullFilename.length()-1]!='/')
    {
        fullFilename.append("/",1);
    }
    fullFilename.append(file.c_str(), file.length());

    Property deviceParameters;
    deviceParameters.fromConfigFile(fullFilename.c_str());

    std::string portName;
    portName+="/";
    portName+=robotName.c_str();
    portName+="/inertial";
    deviceParameters.put("name", portName.c_str());

    deviceParameters.put("device", device.c_str());
    deviceParameters.put("subdevice", subdevice.c_str());

    // create a device for the arm 
    gyro.open(deviceParameters);
    if (!gyro.isValid()) 
    {  
        return false;
    } 

    bool ok = gyro.view(gyro_i);

    return ok;
}

bool RobotInterfaceRemap::detachWrappers()
{
    RobotPartEntry *tmpPart;
    int n=parts.size();
    while(n--)
    {
        tmpPart=parts.back();
        // std::cerr<<"Detaching "<<tmpPart->id<<endl;

        // std::cerr<<"Done detaching " << tmpPart->id<<endl;
        if (tmpPart->iwrapper!=0)
        {
            tmpPart->iwrapper->detachAll();
            tmpPart->close();
        }

        delete tmpPart;
        // std::cerr<<"Deleted object";
        parts.pop_back();
    }
    return true;
}

bool RobotInterfaceRemap::closeNetworks()
{
    RobotNetworkEntry *tmpNet;
    int n=networks.size();

    while(n--)
    {
        tmpNet=networks.back();
        tmpNet->close();
        std::cout<<"Closed network: " << tmpNet->id << endl;
        delete tmpNet;
        networks.pop_back();
    }

    #ifdef _USE_INTERFACEGUI
    if (mServerLogger)
    {
        iCubInterfaceGuiServer *deleting=mServerLogger;
        mServerLogger=NULL;

        deleting->stop();
        delete deleting;
    }
    #endif

    skinparts.close();

    if (!gyro.isValid()) 
        gyro.close();

    initialized = false;
    return true;
}

// check if automatically discovered network id matches the one 
// in the Property, substitute it if necessary.
bool RobotInterfaceRemap::forceNetworkId(yarp::os::Property& op, int autoN)
{
    bool ret=true;
    if (autoN==-1)
        return false;

    Bottle& can = op.findGroup("CAN");
    int networkN=can.find("CanDeviceNum").asInt();
    Bottle &n=can.addList();
    char tmp[80];
    sprintf(tmp, "CanForcedDeviceNum %d", autoN);
    n.fromString(tmp);

    return ret;
}


void RobotInterfaceRemap::abort()
{
    if (isParking)
    {
        RobotNetworkIt it=networks.begin();
        while(it!=networks.end())
        {
            (*it)->abortPark();
            it++;
        }
    }
    if (isCalibrating)
    {
        RobotNetworkIt it=networks.begin();
        while(it!=networks.end())
        {
            (*it)->abortCalibration();
            it++;
        }
    }

    abortF=true;
}

