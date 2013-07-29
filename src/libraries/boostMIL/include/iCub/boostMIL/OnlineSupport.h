/* 
 * Copyright (C) 2010 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Carlo Ciliberto
 * email:  carlo.ciliberto@iit.it
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

/**
 * \defgroup Online Boost
 *  
 * @ingroup boostMIL
 *
 * A strong classifier which is trained online using the algorithm proposed by Oza 
 * (<a href="http://ti.arc.nasa.gov/m/profile/oza/b2hd-oza05.html">PDF</a>) 
 *
 *
 * \author Carlo Ciliberto
 * 
 * Copyright (C) 2010 RobotCub Consortium
 *
 * CopyPolicy: Released under the terms of the GNU GPL v2.0. 
 *  
 */ 



#ifndef __ONLINE_SUPPORT__
#define __ONLINE_SUPPORT__


#include "StrongClassifier.h"


namespace iCub
{

namespace boostMIL
{


class OnlineSupport: public StrongClassifier
{
private:
    /**
    * Maximum size allowed for the list of WeakClassifiers available to train the OnlineClassifier.
    */
    unsigned int            max_function_space_size;

    /**
    * Maximum size allowed for the decision function of the OnlineClassifier.
    */
    unsigned int            weak_classifiers_size;

    /**
    * Initializer method. Should be called whenever the class resource is changed.
    */
    virtual void            initResource();

    std::vector<int>          correct;
    std::vector<int>          wrong;

public:

    /**
    * Constructor.
    * It assumes that the name of the Strong Classifier is 'online_boost'.
    *
    * @param _resource container for the classifier parameters.
    */
    OnlineSupport     (yarp::os::ResourceFinder &_resource);

    /**
    * Constructor.
    *
    * @param _type type of the classifier. It is needed to associate particular inputs and to clone
    *              via the ClassifierFactory.
    * @param _resource container for the classifier parameters.
    */
    OnlineSupport     (const int &_type, yarp::os::ResourceFinder &_resource);

    /**
    * Constructor.
    *
    * @param _type type of the classifier. It is needed to associate particular inputs and to clone
    *              via the ClassifierFactory.
    * @param _resource container for the classifier parameters.
    */
    OnlineSupport     (const std::string &_type, yarp::os::ResourceFinder &_resource);

    /**
    * Virtual Destructor
    */
    ~OnlineSupport    ()      {clear();}

    /**
    * Clear Method. Returns the classifier to its original condition (Pre-initialization).
    */
    virtual void            clear       ();

    /**
    * Updates the classifier and verifies if it is ready to be trained and to classify.
    *
    * @return true if and only if the classifier is ready.
    */
    virtual bool            isReady     ();
    
    /**
    * Virtual Constructor.
    */
    virtual OnlineSupport*    create      ()                                              const   {return NULL;}

    /**
    * Initializes the online classifier with a bag of positive weak classifiers
    *
    * @param initializer the Input containing the weak classifiers.
    */
    virtual void                        initialize  (const Inputs *initializer);

    /**
    * Initializes the online classifier with a list of bags of positive weak classifiers
    *
    * @param initializer the Input list containing the weak classifiers.
    */
    virtual void                        initialize  (const std::list<Inputs*> &initializer);

    /**
    * Encodes the classifier in a single string
    *
    */
    virtual void                         toStream    (std::ofstream &fout) const;

    /**
    * Loads a classifier from a string
    *
    * @param the string encoding the classifier to be loaded
    */
    virtual void                         fromStream  (std::ifstream &str);


    /**
    * Loads a classifier from a string
    *
    * @param the string encoding the classifier to be loaded
    */
    virtual void                         fromString  (const std::string &str);

    /**
    * Clears the function space.
    *
    */
    virtual void                        clearFunctionSpace       ();

    /**
    * Updates the classifier with respect to an input with associated weight. Usually an online method.
    *
    * @param input the input.
    * @param the relevance weight associated to the given input.
    */
    virtual void                        update      (const Inputs *input, double &weight);

    /**
    * Trains the classifier with respect to a list of inputs.
    *
    * @param training_set the list of inputs over which train the classifier.
    */
    virtual void                        train       (const std::list<Inputs*> &training_set);

    /**
    * Trains the classifier with respect to a single input. Usually an online method.
    *
    * @param input the inputs over which train the classifier.
    */
    virtual void                        train       (const Inputs *input, double weight=1.0);

    
    /**
    * Randomizes the function space
    *
    */
    virtual void                        randomizeFuncSpace       ();

    /**
    * Computes the margin of the given input. The margin is the response of the StrongClassifier
    * decision function to the given input.
    *
    * @param input the input provided to the classifier.
    * @return the margin of the given input scaled between 0 and 1.
    */
    double              margin      (const Inputs *input, Output *output=NULL)       const;



};

}

}

#endif



