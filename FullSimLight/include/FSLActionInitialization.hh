
#ifndef FSLActionInitialization_h
#define FSLActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "G4String.hh"
#include "GeantinoMapsConfigurator.hh"

class FSLActionInitialization : public G4VUserActionInitialization {

public:

  FSLActionInitialization(bool isperformance=false);
 ~FSLActionInitialization() override;

  void BuildForMaster() const override;
  void Build() const override;

  void SetPerformanceModeFlag(bool val) { fIsPerformance = val; }
  void  SetSpecialScoringRegionName(const G4String& rname) { fSpecialScoringRegionName = rname; }

private:
  GeantinoMapsConfigurator* fGeantinoMapsConfig;
  bool     fIsPerformance;
  G4String fSpecialScoringRegionName;


};

#endif