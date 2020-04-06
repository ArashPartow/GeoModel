
#include "MyActionInitialization.hh"

#include "MyPrimaryGeneratorAction.hh"
#include "MyRunAction.hh"
#include "MyEventAction.hh"
#include "MySteppingAction.hh"
#include "MyTrackingAction.hh"
#include "LengthIntegrator.hh"


MyActionInitialization::MyActionInitialization(bool isperformance, bool createGeantinoMaps)
: G4VUserActionInitialization(), fIsPerformance(isperformance), fCreateGeantinoMaps(createGeantinoMaps) {}


MyActionInitialization::~MyActionInitialization() {}

// called in case of MT
void MyActionInitialization::BuildForMaster() const {
    MyRunAction* masterRunAct = new MyRunAction();
    masterRunAct->SetPerformanceFlag(fIsPerformance);
    SetUserAction(masterRunAct);
}


void MyActionInitialization::Build() const {
  SetUserAction(new MyPrimaryGeneratorAction());
#ifndef G4MULTITHREADED
// in sequential mode the BuildForMaster method is not called:
// - create the only one run action with perfomance flag true i.e. only time is measued
  if (fIsPerformance) {
    MyRunAction* masterRunAct = new MyRunAction();
    masterRunAct->SetPerformanceFlag(fIsPerformance);
    SetUserAction(masterRunAct);
  }
#endif
  // do not create Run,Event,Stepping and Tracking actions in case of perfomance mode
  if (!fIsPerformance) {
    SetUserAction(new MyRunAction());
    MyEventAction* evtact = new MyEventAction();
    SetUserAction(evtact);
    SetUserAction(new MyTrackingAction(evtact));
    SetUserAction(new MySteppingAction(evtact));
      if(fCreateGeantinoMaps){
          std::cout<<"fCreateGeantinoMaps is true"<<std::endl;
          G4UA::LengthIntegrator* myLenghtInt = new G4UA::LengthIntegrator("eccolo");
          SetUserAction((G4UserEventAction*)   myLenghtInt);
          SetUserAction((G4UserSteppingAction*)myLenghtInt);
      }
  }
}
