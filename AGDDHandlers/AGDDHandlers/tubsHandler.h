/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef tubsHandler_H
#define tubsHandler_H

#include "XMLParser/XMLHandler.h"

#include <string>

class tubsHandler:public XMLHandler {
public:
	tubsHandler(std::string);
	void ElementHandle();

};

#endif
