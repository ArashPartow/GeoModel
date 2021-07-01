
//--------------------------------------------------------
// geantinoMaps application: 2 April 2020 (README)
//--------------------------------------------------------

#include "G4Types.hh"
#ifdef G4MULTITHREADED
  #include "G4MTRunManager.hh"
#else
  #include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4UIterminal.hh"

#include "G4PhysListFactory.hh"
#include "G4VUserPhysicsList.hh"
#include "G4VModularPhysicsList.hh"

#include "Randomize.hh"
#include "MyDetectorConstruction.hh"
#include "MyGVPhysicsList.hh"

#include "MyActionInitialization.hh"

#include <getopt.h>
#include <err.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>

static bool         parCreateGeantinoMaps  = true;
static bool         parIsPerformance       = false;
static bool         parCreateEtaPhiMaps    = false;
static bool         parCreateDetectorsMaps = false;
static bool         parCreateMaterialsMaps  = false;
static bool         parCreateElementsMaps   = false;
static bool         parRunOverlapCheck = false;
static G4String     parGeometryFileName= "";
static G4String     parMacroFileName   = "geantino.g4";
static G4String     parOutputFileName  = "geantinoMaps.root";
static G4String     parPhysListName    = "FTFP_BERT";
static G4double     parRlimit    = 12500; //r limit in mm, for geantino maps
static G4double     parZlimit    = 23000; //z limit in mm, for geantino maps
static G4double     parXlimit    = 12500; //x limit in mm, for geantino maps
static G4double     parYlimit    = 12500; //y limit in mm, for geantino maps

void GetInputArguments(int argc, char** argv);
void Help();


int main(int argc, char** argv) {
    
    // Get input arguments
    GetInputArguments(argc, argv);
    
    G4cout
    << " =============== Running geantinoMaps ================ "      << G4endl
    << "   Geant4 macro               =  " << parMacroFileName        << G4endl
    << "   Geometry file              =  " << parGeometryFileName     << G4endl
    << "   Output file                =  " << parOutputFileName       << G4endl
    << "   Create eta-phi maps        =  " << parCreateEtaPhiMaps     << G4endl
    << "   Create Detectors maps      =  " << parCreateDetectorsMaps  << G4endl
    << "   Create Materials maps      =  " << parCreateMaterialsMaps  << G4endl
    << "   Create Elements maps       =  " << parCreateElementsMaps   << G4endl
    << " ===================================================== "      << G4endl;
    
    // JFB: Check that the macro file exists and is readable:
    if (access(parMacroFileName,R_OK)) {
      G4cout << G4endl;
      G4cout << "INFO: Macro file " << parMacroFileName << " does not exist" << G4endl;
      G4cout << "INFO: Possible locations: /usr/share/FullSimLight or /usr/local/share/FullSimLight" << G4endl;
      G4cout << "INFO: Use the -m option or make a local copy" << G4endl;
      G4cout << "INFO: Exiting" <<G4endl;
      return 1;
    }

        // JFB: Check that the macro file exists and is readable:
    if (!getenv("G4ENSDFSTATEDATA")) {
      G4cout << G4endl;
      G4cout << "INFO: It appears that your Geant4 environment is not set up" << G4endl;
      G4cout << "INFO: Please source the script geant4.sh" << G4endl;
      G4cout << "INFO: Possible locations: /usr/bin or /usr/local/bin" << G4endl;
      G4cout << "INFO: Exiting" <<G4endl;
      return 1;
    }

#if G4VERSION_NUMBER<1040
    G4cout << "INFO: geantinoMaps creation is not supported for Geant4 versions < 10.4" << G4endl;
    G4cout << "INFO: Please update your Geant4 version" << G4endl;
    G4cout << "INFO: Exiting" <<G4endl;
    return 1;
#endif
    
    //choose the Random engine: set to MixMax explicitely (default form 10.4)
    G4Random::setTheEngine(new CLHEP::MixMaxRng);
    // set seed and print info
    G4Random::setTheSeed(12345678);
    G4cout << G4endl
    << " ===================================================== "      << G4endl
    << "   Random engine      = " << G4Random::getTheEngine()->name() << G4endl
    << "   Initial seed       = " << G4Random::getTheSeed()           << G4endl
    << " ===================================================== "      << G4endl
    << G4endl;
    
    // Construct the default run manager
#ifdef G4MULTITHREADED
    G4MTRunManager* runManager = new G4MTRunManager;
    // Number of threads can be defined via macro command
    G4int nThreads = 4;
    runManager->SetNumberOfThreads(nThreads);
#else
    G4RunManager* runManager = new G4RunManager;
#endif
    
    // set mandatory initialization classes
    // 1. Detector construction
    MyDetectorConstruction* detector = new MyDetectorConstruction;
    
    if (parRunOverlapCheck) detector->SetRunOverlapCheck(true);
        
    detector->SetGeometryFileName (parGeometryFileName);
    runManager->SetUserInitialization(detector);
  
    // 2. Physics list
    G4PhysListFactory factory;
    if (factory.IsReferencePhysList(parPhysListName)) {
        G4VModularPhysicsList* physList = factory.GetReferencePhysList(parPhysListName);
        runManager->SetUserInitialization(physList);
    } else if (parPhysListName==G4String("GV")) {
        G4VUserPhysicsList* physList = new MyGVPhysicsList();
        runManager->SetUserInitialization(physList);
    } else {
        G4cerr << "ERROR: Physics List " << parPhysListName << " UNKNOWN!" << G4endl;
        return -1;
    }
  
    // 3. User action
    MyActionInitialization* myAct = new MyActionInitialization(parIsPerformance, parCreateGeantinoMaps,parOutputFileName);
    myAct->SetRlimit(parRlimit);
    myAct->SetZlimit(parZlimit);
    myAct->SetXlimit(parXlimit);
    myAct->SetYlimit(parYlimit);
    myAct->SetCreateEtaPhiMaps(parCreateEtaPhiMaps);
    myAct->SetCreateDetectorsMaps(parCreateDetectorsMaps);
    myAct->SetCreateMaterialsMaps(parCreateMaterialsMaps);
    myAct->SetCreateElementsMaps(parCreateElementsMaps);
    runManager->SetUserInitialization(myAct);


  
    // 4. Run the simulation in batch mode
    G4UImanager* UI = G4UImanager::GetUIpointer();
    G4String command = "/control/execute ";
    UI->ApplyCommand(command+parMacroFileName);
  
    // Print out the final random number
    G4cout << G4endl
           << " ================================================================= " << G4endl
           << " Final random number = " << CLHEP::HepRandom::getTheEngine()->flat() << G4endl
      	   << " ================================================================= " << G4endl
           << G4endl;
    //
    // Delete the RunManager
    delete runManager;
    G4cout
    << " =============== Running geantinoMaps: DONE! ================ "      << G4endl;
    return 0;
}

static struct option options[] = {
    {"geometry file name    "  , required_argument, 0, 'g'},
    {"macro file            "  , required_argument, 0, 'm'},
    {"rlimit                "  , required_argument, 0, 'r'},
    {"zlimit                "  , required_argument, 0, 'z'},
    {"xlimit                "  , required_argument, 0, 'x'},
    {"ylimit                "  , required_argument, 0, 'y'},
    {"output ROOT file name "  , required_argument, 0, 'o'},
    {"etaphiMap             "  , no_argument      , 0, 'e'},
    {"detectorsMap          "  , no_argument      , 0, 'd'},
    {"materialsMap          "  , no_argument      , 0, 'a'},
    {"elementsMap           "  , no_argument      , 0, 'l'},
    {"help"                    , no_argument      , 0, 'h'},
    {0, 0, 0, 0}
};


void Help() {
  std::cout <<"\n " << std::setw(100) << std::setfill('=') << "" << std::setfill(' ') << std::endl;
    G4cout <<" gmgeantino Geant4 application: runs geantino scans.\n"
            << std::endl
            <<"  **** Parameters: \n\n"
            <<"      -g :   [REQUIRED] the Geometry file name (supported extensions: .db/.gdml/.dylib/.so) \n"
            <<"      -m :   [OPTIONAL] the standard Geant4 macro file name (default: 'geantino.g4') \n"
            <<"      -r :   [OPTIONAL] r limit for geantino maps in mm (default: '12500') \n"
            <<"      -z :   [OPTIONAL] z limit for geantino maps in mm (default: '23000') \n"
            <<"      -x :   [OPTIONAL] x limit for geantino maps in mm (default: '12500') \n"
            <<"      -y :   [OPTIONAL] y limit for geantino maps in mm (default: '12500') \n"
            <<"      -o :   [OPTIONAL] output ROOT file name  (supported extention: .root - default: 'geantinoMaps.root') \n"
            <<"      -e :   [FLAG]     use this flag to create eta-phi radiation-interaction length 1D profile histograms (caveat: the process might run out of memory!)\n"
            <<"      -d :   [FLAG]     use this flag to create xy-rz   radiation-interaction length 2D profile histograms for 'detectors' (caveat: the process might run out of memory!)\n"
            <<"      -a :   [FLAG]     use this flag to create xy-rz   radiation-interaction length 2D profile histograms for 'materials' (caveat: the process might run out of memory!)\n"
            <<"      -l :   [FLAG]     use this flag to create xy-rz   radiation-interaction length 2D profile histograms for 'elements'  (caveat: the process might run out of memory!)\n"
            << std::endl;
  std::cout <<"\nUsage: ./gmgeantino [OPTIONS] -g <geometry-file-name> \n" <<std::endl;
  for (int i=0; options[i].name!=NULL; i++) {
    printf("\t-%c  --%s\t\n", options[i].val, options[i].name);
  }
  std::cout<<"\n "<<std::setw(100)<<std::setfill('=')<<""<<std::setfill(' ')<<std::endl;
}


void GetInputArguments(int argc, char** argv) {
  // process arguments
  if (argc == 1) {
   Help();
   exit(0);
  }
  while (true) {
   int c, optidx = 0;
   c = getopt_long(argc, argv, "g:m:o:r:z:x:y:edalh", options, &optidx);
   if (c == -1)
     break;
   //
   switch (c) {
   case 0:
     c = options[optidx].val;
     break;
   case 'm':
     parMacroFileName = optarg;
     break;
   case 'g':
     parGeometryFileName = optarg;
     break;
   case 'o':
     parOutputFileName = optarg;
     break;
   case 'r':
     parRlimit = atof(optarg);
     break;
   case 'z':
     parZlimit = atof(optarg);
     break;
   case 'x':
     parXlimit = atof(optarg);
     break;
   case 'y':
     parYlimit = atof(optarg);
     break;
   case 'e':
     parCreateEtaPhiMaps = true;
     break;
   case 'd':
     parCreateDetectorsMaps = true;
     break;
   case 'a':
     parCreateMaterialsMaps = true;
     break;
   case 'l':
     parCreateElementsMaps = true;
     break;
   case 'h':
     Help();
     exit(0);
   default:
     Help();
     errx(1, "unknown option %c", c);
   }
  }
  // check if output filename has .root extension
    if (!parOutputFileName.contains(".root")) {
        G4cout << "  *** ERROR!!! Output file must have the .root extension. Please type a valid filename." << G4endl;
        Help();
        exit(-1);
    }
    // check if mandatory geometry file was provided
    if (parGeometryFileName=="") {
        G4cout << "  *** ERROR!!! Geometry file is required. Please use the -g option. " << G4endl;
        Help();
        exit(-1);
  }
}
