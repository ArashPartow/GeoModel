/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AGDD2GeoModelBuilder_H
#define AGDD2GeoModelBuilder_H

class GeoMaterial;
class GeoElement;
#include "GeoModelKernel/GeoPhysVol.h"

class AGDDComposition;
class AGDDBox;
class AGDDTubs;
class AGDDElcyl;
class AGDDTrd;
class AGDDCons;
class AGDDPcon;
class AGDDPgon;
class AGDDGvxy;
class AGDDSnake;
class AGDDUnion;
class AGDDIntersection;
class AGDDSubtraction;
class AGDDVolume;

class AGDDBolt;
class AGDDIbeam;
class AGDDUbeam;

class GeoXmlMatManager;

#include "AGDDKernel/AGDDBuilder.h"

#include <string>

class AGDD2GeoModelBuilder:public AGDDBuilder {
public:
	// constructor
	AGDD2GeoModelBuilder();
	~AGDD2GeoModelBuilder() {}
	// elements & materials 
	GeoElement* CreateElement(std::string);
        const GeoMaterial* CreateMaterial(std::string);
	void CreateElements();
	void CreateMaterial();
		
	void CreateBox(AGDDBox*);
	void CreateTrd(AGDDTrd*);
	void CreateTubs(AGDDTubs*);
	void CreateElcyl(AGDDElcyl*);
	void CreateCons(AGDDCons*);
	void CreatePcon(AGDDPcon*);
	void CreatePgon(AGDDPgon*);
	void CreateGvxy(AGDDGvxy*);
	void CreateSnake(AGDDSnake*);
	void CreateUnion(AGDDUnion*);
	void CreateIntersection(AGDDIntersection*);
	void CreateSubtraction(AGDDSubtraction*);
	
	void CreateBolt(AGDDBolt*);
	void CreateIbeam(AGDDIbeam*);
	void CreateUbeam(AGDDUbeam*);
	
	void CreateVolume(AGDDVolume*);
	void CreateComposition(AGDDComposition*);

	void SetMotherVolume(GeoPhysVol *v) {m_mother=v;}
	void BuildAllVolumes();
	void BuildFromSection(std::string);
	void BuildFromVolume(std::string);

//	std::map<std::string, GeoFullPhysVol*>* GetMSdetectors() const;

private:
	GeoPhysVol *m_mother;
	
        const GeoMaterial* GetMMMaterial(std::string);

    /** phi method (cf. EventPrimitives/AmgMatrixBasePlugin.h) */
    inline double phi(const GeoTrf::Vector3D &vec) const {
        if (vec.rows() < 2) return 0.;
        return std::atan2(vec[1],vec[0]);
    }
    /** theta method (cf. EventPrimitives/AmgMatrixBasePlugin.h) */
    inline double theta(const GeoTrf::Vector3D &vec) const {
        if (vec.rows() < 3) return 0.;
        return std::atan2(std::hypot(vec[0],vec[1]),vec[2]);
    }
    
    const GeoXmlMatManager* materialManager=0;

//	std::map<std::string, GeoFullPhysVol*>* m_detectors;

};

#endif
