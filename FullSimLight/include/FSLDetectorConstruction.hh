
#ifndef FSLDetectorConstruction_h
#define FSLDetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"

#include "G4GDMLParser.hh"
#include "G4String.hh"
#include "G4Timer.hh"
#include <nlohmann/json.hpp>

#include "G4Cache.hh"
#include "G4MagneticField.hh"

//G4AnalysisMananager
#include "FSLAnalysis.hh"

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits

using json = nlohmann::json;

class G4VPhysicalVolume;
class G4FieldManager;
class G4UniformMagField;
class G4MagneticField;
class G4VIntegrationDriver;
class G4MagIntegratorStepper;
class FSLDetectorMessenger;
class GeoPhysVol;

class FSLDetectorConstruction : public G4VUserDetectorConstruction {

public:
  FSLDetectorConstruction();
  ~FSLDetectorConstruction();

  G4VPhysicalVolume *Construct();
  void ConstructSDandField();

  void SetGDMLFileName  (const G4String &gdmlfile)   { fGeometryFileName = gdmlfile;}
  void SetRunOverlapCheck(const bool runOvCheck)     { fRunOverlapCheck = runOvCheck; }
  void SetAddRegions(const bool addRegions)          { fAddRegions = addRegions; }     
  void SetRunMassCalculator(const bool runMassCalc)  { fRunMassCalculator = runMassCalc; }
  void SetVerbosity(const int verbosity)            { fVerbosityFlag = verbosity; }
  void SetGeometryFileName(const G4String &geometryFileName) { fGeometryFileName = geometryFileName; }
  void SetPrefixLogicalVolume(const G4String &prefixLV) { fPrefixLogicalVolume = prefixLV; }
  void SetMaterial(const G4String &material) { fMaterial = material; }
  void SetReportFileName(const G4String &reportFileName)     { fReportFileName = reportFileName; }
  void SetGMClashVerbosity(const G4bool flag)     { fGmclashVerbosity = flag; }
  void SetTolerance (const G4double tolerance){fTolerance=tolerance;}
  void SetOutputGDMLFileName(const G4String &outputGDMLFileName)     { fOutputGDMLFileName = outputGDMLFileName; }
  void SetDumpGDML(const bool dumpGDML)              {fDumpGDML=dumpGDML;}
  /// Common method to construct a driver with a stepper of requested type.
  G4VIntegrationDriver*
  createDriverAndStepper(std::string stepperType) const;
  G4MagIntegratorStepper* CreateStepper(std::string name, G4MagneticField* field) const;

  void SetMagFieldValue(const G4double fieldValue)
  {
    fFieldValue = fieldValue;
    gFieldValue = fFieldValue;
    fFieldConstant = true;
  }

  static G4double GetFieldValue() { return gFieldValue; }
  G4double GetTolerance (){return fTolerance;}

  GeoPhysVol* CreateTheWorld(GeoPhysVol* world);

  /// Clean the geometry  from Unidentified volumes before dumping it in GDML format
  void PullUnidentifiedVolumes( G4LogicalVolume* v );
  static G4AnalysisManager* fAnalysisManager;

protected:
  G4Timer fTimer;

private:
  // this static member is for the print out
  static G4double gFieldValue;
  G4bool   fRunOverlapCheck;
  G4bool   fRunMassCalculator;
  G4bool   fAddRegions;
  G4int    fVerbosityFlag;
  G4bool   fDumpGDML;
  G4double fMinStep;
  G4String fGeometryFileName;
  G4String fPrefixLogicalVolume;
  G4String fMaterial;
  G4String fReportFileName;
  G4String fOutputGDMLFileName;
  G4double fFieldValue;
  G4bool   fFieldConstant;
  G4bool   fGmclashVerbosity;
  G4double fTolerance; //tolerance value for gmclash
  G4GDMLParser fParser;
  G4VPhysicalVolume *fWorld;
  FSLDetectorMessenger *fDetectorMessenger;
  G4Cache<G4MagneticField*> fField; //pointer to the thread-local fields
};

#endif
