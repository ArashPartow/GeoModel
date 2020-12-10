/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "AGDDHandlers/snake_pointHandler.h"

#include <iostream>
#include <vector>

GeoTrf::Vector3D snake_pointHandler::s_point(GeoTrf::Vector3D::Identity());

snake_pointHandler::snake_pointHandler(std::string s):XMLHandler(s)
{
//	std::cout<<"Creating handler for snake_point"<<std::endl;
}

void snake_pointHandler::ElementHandle()
{
	bool res;
	std::vector<double> vvv;
	vvv=getAttributeAsVector("X_Y_Z",res);
	s_point = GeoTrf::Vector3D(vvv[0], vvv[1], vvv[2]);
}
