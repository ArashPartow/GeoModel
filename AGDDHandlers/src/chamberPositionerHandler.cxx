/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "AGDDHandlers/chamberPositionerHandler.h"
#include "AGDDKernel/AGDDDetectorStore.h"
#include "AGDDKernel/AGDDDetectorPositioner.h"
#include "AGDDKernel/AGDDDetector.h"
#include <iostream>

chamberPositionerHandler::chamberPositionerHandler(std::string s):XMLHandler(s)
{
}

void chamberPositionerHandler::ElementHandle()
{
	std::string volume=getAttributeAsString("volume");
	
	//AGDDVolume* theVol=AGDDVolumeStore::GetVolumeStore()->GetVolume(volume);
	
	AGDDDetector* mCham=(AGDDDetectorStore::GetDetectorStore())->GetDetector(volume);
	std::string subType;
	if (!mCham) 
	{
		std::cout<<"chamberPositionerHandler: something wrong! returning..."<<std::endl;
		return;
	}
	else
		subType=mCham->subType();
	
	double radius=getAttributeAsDouble("radius");
	double zPos=getAttributeAsDouble("zPos");
	
	double phi0=getAttributeAsDouble("phi0",0.);
	int iWedge=getAttributeAsInt("wedge_number",8);
	
	std::string zLayout=getAttributeAsString("zLayout","Z_SYMMETRIC");
	std::string type=getAttributeAsString("type","BARREL");
	std::string chType=getAttributeAsString("chamberType");
	std::string iSectors=getAttributeAsString("iSectors","11111111");
	std::string detectorType=getAttributeAsString("detectorType","MDT");
	int etaIndex=getAttributeAsInt("etaIndex",0);
	
	double dPhi=360./iWedge;
	

	const double degrad=M_PI/180.;

	if (iSectors.size()!= (unsigned int) iWedge) throw;
	
	// std::cout<<" =============>> this is chamberPositionerHandler::ElementHandle() "<<volume<<" "<<subType<<std::endl;
	
 	for (int i=0;i<iWedge;i++)
 	{
 		if (iSectors[i]=='0') continue;
		if (zLayout!="Z_NEGATIVE")
		{
			double Wedge=dPhi*i+phi0;
			GeoTrf::Transform3D crot = GeoTrf::Transform3D::Identity();
			if (type=="ENDCAP") 
			{
				//	fix to ensure right order of planes			
				crot = crot*GeoTrf::RotateZ3D(Wedge*degrad)*GeoTrf::RotateY3D(90*degrad)*GeoTrf::RotateZ3D(180.*degrad);
			}
			else crot = crot*GeoTrf::RotateZ3D(Wedge*degrad);
 			double x=radius*cos(Wedge*degrad);
 			double y=radius*sin(Wedge*degrad);
 			double zpos=zPos;
 			GeoTrf::Vector3D cvec=GeoTrf::Vector3D(x,y,zpos);
 			AGDDDetectorPositioner *p __attribute__((__unused__));
 			p=new AGDDDetectorPositioner(volume,crot,cvec);
			p->SensitiveDetector(true);
			p->ID.phiIndex=i;
			p->ID.sideIndex=1;
			p->ID.etaIndex=etaIndex;
			p->ID.detectorType=detectorType;
			
			p->position.Zposition=zpos;
			p->position.Radius=radius;
			p->position.PhiStart=phi0;
			p->position.Phi=Wedge;
			
			mCham->SetAddressAndPosition(p);
			
		}
		if (zLayout!="Z_POSITIVE")
        {
			double Wedge=dPhi*i+phi0;
            GeoTrf::Transform3D crot = GeoTrf::Transform3D::Identity();
            if (type=="ENDCAP")
            {
				//	fix to ensure right order of planes
				crot = crot*GeoTrf::RotateX3D(180.*degrad)*GeoTrf::RotateZ3D(-Wedge*degrad)*GeoTrf::RotateY3D(90*degrad)*GeoTrf::RotateZ3D(180.*degrad);
            }
            else crot = crot*GeoTrf::RotateZ3D(Wedge*degrad);
            double x=radius*cos(Wedge*degrad);
            double y=radius*sin(Wedge*degrad);
            double zpos=zPos;
            GeoTrf::Vector3D cvec=GeoTrf::Vector3D(x,y,-zpos);
            AGDDDetectorPositioner *p __attribute__((__unused__));
            p=new AGDDDetectorPositioner(volume,crot,cvec);
			p->SensitiveDetector(true);
			p->ID.phiIndex=i;
			p->ID.sideIndex=-1;
			p->ID.etaIndex=-etaIndex;
			p->ID.detectorType=detectorType;
			
			p->position.Zposition=-zpos;
			p->position.Radius=radius;
			p->position.PhiStart=phi0;
			p->position.Phi=Wedge;
			
			mCham->SetAddressAndPosition(p);
        }
 	}

}
