/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef natomsHandler_H
#define natomsHandler_H

#include "XMLParser/XMLHandler.h"
#include <string>

class natomsHandler:public XMLHandler {
public:
	natomsHandler(std::string);
	void ElementHandle();

};

#endif
