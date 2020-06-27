// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

/*
 * HelloGeo.cpp
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Nov, 2018
 *
 */

// GeoModel includes
#include "GeoModel2G4/ExtParameterisedVolumeBuilder.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"

// Geant4 includes
#include "G4LogicalVolume.hh"

// C++ includes
#include <iostream>
#include <fstream>
#include <cstdlib> // EXIT_FAILURE


// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'


// GeoModelExperiment* createTheExperiment(GeoPhysVol* world)
// {
//   if (world == nullptr)
//   {
//     // Setup the 'World' volume from which everything else will be suspended
//     double densityOfAir=0.1;
//     const GeoMaterial* worldMat = new GeoMaterial("std::Air", densityOfAir);
//     const GeoBox* worldBox = new GeoBox(1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm);
//     const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, worldMat);
//     world = new GeoPhysVol(worldLog);
//   }
//   // Setup the 'Experiment' manager
//   GeoModelExperiment* theExperiment = new GeoModelExperiment(world);
//   return theExperiment;
// }



// TODO: int main(int argc, char *argv[])
int main()
{

  // Set a valid local geometry db path before first run
  static const std::string path = "../geometry.db";

  std::cout << "Using this DB file:" << path << std::endl;

  // check if DB file exists. If not, return.
  // FIXME: TODO: this check should go in the 'GMDBManager' constructor.
  std::ifstream infile(path.c_str());
    if ( infile.good() ) {
      std::cout << "\n\tERROR!! A '" << path << "' file exists already!! Please, remove, move, or rename it before running this program. Exiting...";
        exit(EXIT_FAILURE);
  }
  infile.close();

  // open the DB
  GMDBManager* db = new GMDBManager(path);

  /* Open database */
  if (db->checkIsDBOpen()) {
    std::cout << "OK! Database is open!\n";
  }
  else {
    std::cout << "Database is not open!\n";
    exit(EXIT_FAILURE);
  }

  // -- testing the input database
  std::cout << "Printing the list of all GeoMaterial nodes" << std::endl;
  db->printAllMaterials();

  /* setup the GeoModel reader */
  GeoModelIO::ReadGeoModel readInGeo = GeoModelIO::ReadGeoModel(db);
  std::cout << "ReadGeoModel set.\n";


  /* build the GeoModel geometry */
  GeoPhysVol* world = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory and get an handle to the 'world' volume
  std::cout << "ReadGeoModel::buildGeoModel() done.\n";



  // --- testing the imported ATLAS Geometry

  // get the GeoLogVol used for the 'world' volume
  std::cout << "Getting the GeoLogVol used by the 'world' volume" << std::endl;
  const GeoLogVol* logVol = world->getLogVol();
  std::cout << "'world' GeoLogVol name: " << logVol->getName() << std::endl;
  std::cout << "'world' GeoMaterial name: " << logVol->getMaterial()->getName() << std::endl;

  // get number of children volumes
  unsigned int nChil = world->getNChildVols();
  std:: cout << "'world' number of children: " << nChil << std::endl;

  // loop over all children nodes
  std::cout << "Looping over all 'volume' children (i.e., GeoPhysVol and GeoFullPhysVol)..." << std::endl;
  for (unsigned int idx=0; idx<nChil; ++idx) {
	PVConstLink nodeLink = world->getChildVol(idx);

	if ( dynamic_cast<const GeoVPhysVol*>( &(*( nodeLink ))) ) {
		std::cout << "\t" << "the child n. " << idx << " ";
		const GeoVPhysVol *childVolV = &(*( nodeLink ));
		if ( dynamic_cast<const GeoPhysVol*>(childVolV) )

        {
			const GeoPhysVol* childVol = dynamic_cast<const GeoPhysVol*>(childVolV);
			std::cout << "is a GeoPhysVol, whose GeoLogVol name is: " << childVol->getLogVol()->getName() << std::endl;
            std::cout<< " and it has  "<<childVol->getNChildVols()<<" child volumes\n";

		} else if ( dynamic_cast<const GeoFullPhysVol*>(childVolV) ) {
			const GeoFullPhysVol* childVol = dynamic_cast<const GeoFullPhysVol*>(childVolV);
			std::cout << "is a GeoFullPhysVol, whose GeoLogVol name is: " << childVol->getLogVol()->getName() << std::endl;
            std::cout<< " and it has  "<<childVol->getNChildVols()<<" child volumes\n";
		}
        } else if ( dynamic_cast<const GeoNameTag*>( &(*( nodeLink ))) ) {
		std::cout << "\t" << "the child n. " << idx << " is a GeoNameTag\n";
		const GeoNameTag *childVol = dynamic_cast<const GeoNameTag*>(&(*( nodeLink )));
		std::cout << "\t\tGeoNameTag's name: " << childVol->getName() << std::endl;
        //std::cout<< " and it has  "<<childVol->getNChildVols()<<" child volumes\n";
        }
  }

  // build the Geant4 geometry and get an hanlde to the world' volume
  ExtParameterisedVolumeBuilder* builder = new ExtParameterisedVolumeBuilder("ATLAS");
  std::cout << "Building G4 geometry."<<std::endl;
  G4LogicalVolume* g4World = builder->Build(world);

  std::cout << "This is the newly-created Geant4 G4LogicalVolume, ready to be used: " << g4World << std::endl;

  std::cout << "Everything done.\n";

  return 0;
}
