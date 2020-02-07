/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "ModuleFanCalculator.h"

#include "GeoSpecialShapes/LArWheelCalculator.h"
#include "GeoModelKernel/GeoDefinitions.h"

// Physical constants
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits

#ifdef HARDDEBUG
#undef HARDDEBUG
#endif

namespace LArWheelCalculator_Impl
{
  ModuleFanCalculator::ModuleFanCalculator(LArWheelCalculator* lwc)
    : m_lwc(lwc)
  {
  }

  double ModuleFanCalculator::DistanceToTheNearestFan(GeoTrf::Vector3D &p, int & out_fan_number) const
  {
    //int fan_number = int((p.phi() - SYSTEM_OF_UNITS::halfpi - lwc()->m_ZeroFanPhi_ForDetNeaFan) / lwc()->m_FanStepOnPhi);
    int fan_number = int((std::atan2(p(1),p(0)) - SYSTEM_OF_UNITS::halfpi - lwc()->m_ZeroFanPhi_ForDetNeaFan) / lwc()->m_FanStepOnPhi);
    double angle = lwc()->m_FanStepOnPhi * fan_number + lwc()->m_ZeroFanPhi_ForDetNeaFan;
#ifdef HARDDEBUG
    printf("DistanceToTheNearestFan: initial FN %4d\n", fan_number);
#endif
    //p.rotateZ(-angle);
    p=GeoTrf::RotateZ3D(-angle)*p;
    
    // determine search direction
    double d0 = lwc()->DistanceToTheNeutralFibre(p, lwc()->adjust_fan_number(fan_number));
    double d1 = d0;
    int delta = 1;
    if(d0 < 0.) delta = -1; // search direction has been determined
    angle = - lwc()->m_FanStepOnPhi * delta;
    do { // search:
      //p.rotateZ(angle);
      p = GeoTrf::RotateZ3D(angle)*p;
      fan_number += delta;
      d1 = lwc()->DistanceToTheNeutralFibre(p, lwc()->adjust_fan_number(fan_number));
    } while(d0 * d1 > 0.);
    // if signs of d1 and d0 are different, the point is between current pair
    if(delta > 0) fan_number --;

    int adj_fan_number = fan_number;
    if(adj_fan_number < lwc()->m_FirstFan) {
      //p.rotateZ((adj_fan_number - lwc()->m_FirstFan) * lwc()->m_FanStepOnPhi);
      p = GeoTrf::RotateZ3D((adj_fan_number - lwc()->m_FirstFan) * lwc()->m_FanStepOnPhi)*p;
      adj_fan_number = lwc()->m_FirstFan;
    } else if(adj_fan_number >= lwc()->m_LastFan) {
      //p.rotateZ((adj_fan_number - lwc()->m_LastFan + 1) * lwc()->m_FanStepOnPhi);
      p = GeoTrf::RotateZ3D((adj_fan_number - lwc()->m_LastFan + 1) * lwc()->m_FanStepOnPhi)*p;
      adj_fan_number = lwc()->m_LastFan - 1;
    }

    //p.rotateZ(-0.5 * angle);
    p = GeoTrf::RotateZ3D(-0.5 * angle)*p;
    out_fan_number = adj_fan_number;
    return lwc()->DistanceToTheNeutralFibre(p, adj_fan_number);
  }

  int ModuleFanCalculator::PhiGapNumberForWheel(int i) const {
    i += lwc()->m_ZeroGapNumber;
    i -= lwc()->m_LastFan / 2;
    if(i < 0) i += lwc()->m_NumberOfFans;
    if(i >= lwc()->m_NumberOfFans) i -= lwc()->m_NumberOfFans;
    return i;
  }

  std::pair<int, int> ModuleFanCalculator::GetPhiGapAndSide(const GeoTrf::Vector3D &p) const
  {
    // Note: this object was changed from static to local for thread-safety.
    // If this is found to be too costly we can re-evaluate.
    GeoTrf::Vector3D p1 = p;
    //int fan_number = int((p.phi() - SYSTEM_OF_UNITS::halfpi - lwc()->m_ZeroFanPhi) / lwc()->m_FanStepOnPhi);
    int fan_number = int((std::atan2(p(1),p(0)) - SYSTEM_OF_UNITS::halfpi - lwc()->m_ZeroFanPhi) / lwc()->m_FanStepOnPhi);

    double angle = lwc()->m_FanStepOnPhi * fan_number + lwc()->m_ZeroFanPhi;
    //p1.rotateZ(-angle);
    p1 =GeoTrf::RotateZ3D(-angle) * p1;
    
    double d0 = lwc()->DistanceToTheNeutralFibre(p1, lwc()->adjust_fan_number(fan_number));
    double d1 = d0;

    int delta = 1;
    if(d0 < 0.) delta = -1;
    angle = - lwc()->m_FanStepOnPhi * delta;
    do {
      //p1.rotateZ(angle);
      p1 = GeoTrf::RotateZ3D(angle)*p1;
      fan_number += delta;
      d1 = lwc()->DistanceToTheNeutralFibre(p1, lwc()->adjust_fan_number(fan_number));
    } while(d0 * d1 > 0.);
    if(delta > 0) fan_number --;
    if(!lwc()->m_isElectrode) fan_number ++;

    int adj_fan_number = fan_number;
    if(adj_fan_number < lwc()->m_FirstFan) adj_fan_number = lwc()->m_FirstFan - 1;
    else if(adj_fan_number > lwc()->m_LastFan) adj_fan_number = lwc()->m_LastFan;

    //p1.rotateZ(-0.5 * angle);
    p1 = GeoTrf::RotateZ3D(-0.5 * angle) * p1;
    double dd = lwc()->DistanceToTheNeutralFibre(p1, adj_fan_number);
    int side = dd < 0.? -1: 1;
#ifdef HARDDEBUG
    printf("GetPhiGapAndSide: MFN %4d\n", adj_fan_number);
#endif
    return std::pair<int, int>(adj_fan_number, side);
  }

}
