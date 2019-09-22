/*
 * GeoModelReadIn.cpp
 *
 *  Created on: May 20, 2016
 *      Author: riccardo.maria.bianchi@cern.ch
 *
 * major updates: Feb 2019 rbianchi
 */

// local includes
#include "GeoModelRead/ReadGeoModel.h"

// TFPersistification includes
#include "TFPersistification/TransFunctionInterpreter.h"

// GeoModelKernel
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoGraphNode.h"

// GeoModel shapes
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoTessellatedSolid.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'

//VP1Base
// TODO: we should get rid of VP1Base::VP1Msg dependency, since GeoModelRead should not depend on VP1 packages. Maybe we can move VP1Msg to a standalone package.
//#include "VP1Base/VP1Msg.h"

// Qt includes
#include <QDebug>

// C++ includes
#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <vector>     /* exit, EXIT_FAILURE */



using namespace GeoGenfun;
using namespace GeoXF;


namespace GeoModelIO {

ReadGeoModel::ReadGeoModel(GMDBManager* db, unsigned long* progress) : m_progress(nullptr), m_deepDebug(false)
{
	qDebug() << "DumpGeoModelAction: constructor";

	#ifdef GEOMODELREAD_DEEP_DEBUG
	  m_deepDebug = true;
 	#endif

        if ( progress != nullptr) {
	    m_progress = progress;
	}

	// set the geometry file
	m_dbManager = db;
	if (m_dbManager->isOpen()) {
		qDebug() << "OK! Database is open!";
	}
	else {
		qWarning() << "ERROR!! Database is NOT open!";
		return;
	}
}

ReadGeoModel::~ReadGeoModel() {
	// TODO Auto-generated destructor stub
}


GeoPhysVol* ReadGeoModel::buildGeoModel()
{
	qDebug() << "ReadGeoModel::buildGeoModel()";

	// return buildGeoModelByCalls();
	GeoPhysVol* rootVolume = buildGeoModelOneGo();

	if (m_unknown_shapes.size() > 0) {
		qWarning() << "\tWARNING!! There were unknwon shapes:";
		for ( auto it = m_unknown_shapes.begin(); it != m_unknown_shapes.end(); it++ ) {
			std::cout << "\t\t---> " << *it << std::endl;
		}
		std::cout << "\tRemember: unknown shapes are rendered with a dummy cube of 30cm side length.\n\n" << std::endl;
		}

	return rootVolume;
}


GeoPhysVol* ReadGeoModel::buildGeoModelOneGo()
{
	qDebug() << "ReadGeoModel::buildGeoModelOneGo()";

	// get all objects from the DB
	_physVols = m_dbManager->getTableFromNodeType("GeoPhysVol");
	std::cout << "GeoPhysVol, read in." << std::endl;
	_fullPhysVols = m_dbManager->getTableFromNodeType("GeoFullPhysVol");
	std::cout << "GeoFullPhysVol, read in." << std::endl;
	_logVols = m_dbManager->getTableFromNodeType("GeoLogVol");
	std::cout << "GeoLogVol, read in." << std::endl;
	_shapes = m_dbManager->getTableFromNodeType("GeoShape");
	std::cout << "GeoShape, read in." << std::endl;
	_materials = m_dbManager->getTableFromNodeType("GeoMaterial");
	std::cout << "GeoMaterial, read in." << std::endl;
	_functions = m_dbManager->getTableFromNodeType("Function");
	std::cout << "Function, read in." << std::endl;
	_serialDenominators = m_dbManager->getTableFromNodeType("GeoSerialDenominator");
	std::cout << "GeoSerialDenominator, read in." << std::endl;
	_serialTransformers = m_dbManager->getTableFromNodeType("GeoSerialTransformer");
	std::cout << "GeoSerialTransformer, read in." << std::endl;
	_alignableTransforms = m_dbManager->getTableFromNodeType("GeoAlignableTransform");
	std::cout << "GeoAlignableTransform, read in." << std::endl;
	_transforms = m_dbManager->getTableFromNodeType("GeoTransform");
	std::cout << "GeoTransform, read in." << std::endl;
	_nameTags = m_dbManager->getTableFromNodeType("GeoNameTag");
	std::cout << "GeoNameTag, read in." << std::endl;
	// qDebug() << "physVols: " << _physVols;
	// qDebug() << "fullPhysVols: " << _fullPhysVols;

	// get DB metadata
	_tableid_tableName = m_dbManager->getAll_TableIDsNodeTypes();
	std::cout << "DB metadata, read in." << std::endl;

	// get the children table from DB
	_allchildren = m_dbManager->getChildrenTable();
	// qDebug() << "all children from DB:" << _allchildren;
	std::cout << "children positions, read in." << std::endl;

	// get the root volume data
	_root_vol_data = m_dbManager->getRootPhysVol();
	std::cout << "root volume data, read in." << std::endl;

	return loopOverAllChildren();
}


//----------------------------------------
GeoPhysVol* ReadGeoModel::loopOverAllChildren()
{

	std::cout << "Looping over all children to build the GeoModel tree..." << std::endl;

	int nChildrenRecords = _allchildren.size();

	// This should go in VP1Light, not in this library. The library could be used by standalone apps without a GUI
        /*
	  QProgressDialog progress("Loading the geometry...", "Abort Loading", 0, nChildrenRecords, 0);
  	  progress.setWindowModality(Qt::WindowModal);
	  progress.show();
	*/

	// loop over parents' keys
	int counter = 0;
	foreach (const QString &parentKey, _allchildren.keys() ) {

		 /* //This should go in VP1Light as well!
		  if (progress.wasCanceled()) {
			std::cout << "You aborted the loading of the geometry." << std::endl;

			QMessageBox msgBox;
			msgBox.setText("You aborted the loading of the geometry.");
			msgBox.setInformativeText("Do you want to really abort it?");
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			int ret = msgBox.exec();

			switch (ret) {
			   case QMessageBox::Yes:
			      // Abort the loading of the geometry
			      break;
			   case QMessageBox::No:
			      // Continue with the loading of the geometry
			      progress.reset();
			  default:
			      // should never be reached
			      break;
			}
		  }
		*/



		++counter;
		std::cout.precision(0);
		if ( nChildrenRecords < 10000 && counter % 500 == 0 ) {
			float done = ( (float)counter / nChildrenRecords) * 100;
			std::cout << "\t" << std::fixed << counter << "children records processed [" << done << "%]" << std::endl;
			if ( m_progress != nullptr  ) {
			  //progress.setValue(counter); // This should go in VP1Light
			  *m_progress = counter;
			}
		}
		else if ( nChildrenRecords > 10000 && counter % 2000 == 0 ) {
			float done = ( (float)counter / nChildrenRecords) * 100;
			std::cout << "\t" << std::fixed << counter << " children records processed [" << done << "%]" << std::endl;
			if ( m_progress != nullptr ) {
			  //progress.setValue(counter); // This should go in VP1Light
			  *m_progress = counter;
			}
		}
		if (m_deepDebug) qDebug() << "\nparent: " << parentKey << ':' << _allchildren.value(parentKey) << "[parentId, parentType, parentCopyNumber, childPos, childType, childId, childCopyN]";

		// get the parent's details
		QStringList parentKeyItems = parentKey.split(":");
		QString parentId = parentKeyItems[0];
		QString parentTableId = parentKeyItems[1];
		QString parentCopyN = parentKeyItems[2];
		if (m_deepDebug) qDebug() << "parent ID:" << parentId << ", parent table ID:" << parentTableId << ", parent copy number:" << parentCopyN;

		bool isRootVolume = false;
		if (parentId == "NULL") {
		  isRootVolume = true;
		}

		GeoVPhysVol* parentVol = nullptr;

		// build or get parent volume.
		// Using the parentCopyNumber here, to get a given instance of the parent volume
		if (!isRootVolume) {
			if (m_deepDebug) qDebug() << "get the parent volume...";
		 	parentVol = buildVPhysVol( parentId, parentTableId, parentCopyN);
	 	}


		// get the parent's children
		QMap<unsigned int, QStringList> children = _allchildren.value(parentKey);



		// loop over children, sorted by child position automatically
		// "id", "parentId", "parentTable", "parentCopyNumber", "position", "childTable", "childId", "childCopyNumber"
		if (m_deepDebug) qDebug() << "parent volume has " << children.size() << "children. Looping over them...";
		foreach(QStringList child, children) {

			if (m_deepDebug) qDebug() << "child:" << child;

			// build or get child node
			if (child.length() < 8) {
				std::cout <<  "ERROR!!! Probably you are using an old geometry file..." << std::endl;
				exit(EXIT_FAILURE);
			}
			QString childTableId = child[5];
			QString childId = child[6];
			QString childCopyN = child[7];

			QString childNodeType = _tableid_tableName[childTableId.toUInt()];

			if (m_deepDebug) qDebug() << "childTableId:" << childTableId << ", type:" << childNodeType << ", childId:" << childId;

			if (childNodeType.isEmpty()) {
				qWarning("ERROR!!! childNodeType is empty!!! Aborting...");
				exit(EXIT_FAILURE);
			}

			if (childNodeType == "GeoPhysVol") {
				if (m_deepDebug) qDebug() << "GeoPhysVol child...";
				GeoVPhysVol* childNode = dynamic_cast<GeoPhysVol*>(buildVPhysVol(childId, childTableId, childCopyN));
				if (!isRootVolume) volAddHelper(parentVol, childNode);
			}
			else if (childNodeType == "GeoFullPhysVol") {
				if (m_deepDebug) qDebug() << "GeoFullPhysVol child...";
				GeoVPhysVol* childNode = dynamic_cast<GeoFullPhysVol*>(buildVPhysVol(childId, childTableId, childCopyN));
				if (!isRootVolume) volAddHelper(parentVol, childNode);
			}
			else if (childNodeType == "GeoSerialDenominator") {
				if (m_deepDebug) qDebug() << "GeoSerialDenominator child...";
				GeoSerialDenominator* childNode = buildSerialDenominator(childId);
				if (!isRootVolume) volAddHelper(parentVol, childNode);
			}
			else if (childNodeType == "GeoAlignableTransform") {
				if (m_deepDebug) qDebug() << "GeoAlignableTransform child...";
				GeoAlignableTransform* childNode = buildAlignableTransform(childId);
				if (!isRootVolume) volAddHelper(parentVol, childNode);
			}
			else if (childNodeType == "GeoTransform") {
				if (m_deepDebug) qDebug() << "GeoTransform child...";
				GeoTransform* childNode = buildTransform(childId);
				if (!isRootVolume) volAddHelper(parentVol, childNode);
			}
			else if (childNodeType == "GeoSerialTransformer") {
				if (m_deepDebug) qDebug() << "GeoSerialTransformer child...";
				GeoSerialTransformer* childNode = buildSerialTransformer(childId);
				if (!isRootVolume) volAddHelper(parentVol, childNode);
			}
			else if (childNodeType == "GeoNameTag") {
				if (m_deepDebug) qDebug() << "GeoNameTag child...";
				GeoNameTag* childNode = buildNameTag(childId);
				if (!isRootVolume) volAddHelper(parentVol, childNode);
	                }
			else {
				QString msg = "[" + childNodeType + "]" + QString(" ==> ERROR!!! - The conversion for this type of child node needs to be implemented, still!!!");
				qFatal("%s", msg.toLatin1().constData());
			}

		} // loop over all children
	} // loop over childrenPositions records

	// return the root volume
	return getRootVolume();
}


void ReadGeoModel::volAddHelper(GeoVPhysVol* vol, GeoGraphNode* volChild)
{
	if (dynamic_cast<GeoPhysVol*>(vol)) {
		GeoPhysVol* volume = dynamic_cast<GeoPhysVol*>(vol);
		volume->add(volChild);
	} else if (dynamic_cast<GeoFullPhysVol*>(vol)) {
		GeoFullPhysVol* volume = dynamic_cast<GeoFullPhysVol*>(vol);
		volume->add(volChild);
	}
}


void ReadGeoModel::checkInputString(QString input)
{
	if (input.isEmpty() || input.isNull() || input == "NULL") {
		qWarning() << "ERROR!!! Input QString is empty or equal to 'NULL'!!! Aborting...";
		exit(1);
	}
}

// Instantiate a PhysVol and get its children
GeoVPhysVol* ReadGeoModel::buildVPhysVol(QString id, QString tableId, QString copyN)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildVPhysVol()" << id << tableId << copyN;

	checkInputString(id);
	checkInputString(tableId);

	// if previously built, return that
	if (isNodeBuilt(id, tableId, copyN)) {
		if (m_deepDebug) qDebug() << "getting the volume from memory...";
		return dynamic_cast<GeoVPhysVol*>(getNode(id, tableId, copyN));
	}

	if (m_deepDebug) qDebug() << "building a new volume...";

	// QString nodeType = m_dbManager->getNodeTypeFromTableId(tableId.toUInt());
	QString nodeType = _tableid_tableName[tableId.toUInt()];

	// get the parent volume parameters
	// here we do not need to use copyN, since the actual volume is the same for all instances
	QStringList values;
	if (nodeType == "GeoPhysVol")
		 values = _physVols[id.toUInt()];
	else if (nodeType == "GeoFullPhysVol")
		 values = _fullPhysVols[id.toUInt()];



	QString volId = values[0];
	QString logVolId = values[1];
	//QString parentId = values[2]; // FIXME: delete it, it is not used any more

	if (m_deepDebug) {
	  qDebug() << "\tPhysVol-ID:" << volId;
	  qDebug() << "\tPhysVol-LogVol:" << logVolId;
	  //qDebug() << "\tPhysVol-parentId:" << parentId;
	  qDebug() << "\tnodeType:" << nodeType;
	}

	// GET LOGVOL
	GeoLogVol* logVol = buildLogVol(logVolId);

	// a pointer to the VPhysVol
	GeoVPhysVol* vol = nullptr;

	// BUILD THE PHYSVOL OR THE FULLPHYSVOL
	if (nodeType == "GeoPhysVol")
		vol = new GeoPhysVol(logVol);
	else if (nodeType == "GeoFullPhysVol")
		vol = new GeoFullPhysVol(logVol);
	else
		qWarning() << "ERROR!!! Unkonwn node type!! : " << nodeType;

	// storing the address of the newly built node
	storeNode(id, tableId, copyN, vol);

	return vol;
}


// Get the root volume
GeoPhysVol* ReadGeoModel::getRootVolume()
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::getRootVolume()";
	QString id = _root_vol_data[1];
	QString tableId = _root_vol_data[2];
	QString copyNumber = "1"; // the Root volume has only one copy by definition
	return dynamic_cast<GeoPhysVol*>(buildVPhysVol(id, tableId, copyNumber));
}


GeoMaterial* ReadGeoModel::buildMaterial(QString id)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildMaterial()";
	QStringList values = _materials[id.toUInt()];

	QString matId = values[0];
	QString matName = values[1];

	if (m_deepDebug) qDebug() << "\tMaterial-ID:" << matId << ", Material-name:" << matName;

	// TODO: Bogus densities.  Later: read from database.
	double densityOfAir=0.1;

	return new GeoMaterial(matName.toStdString(),densityOfAir);

}


GeoShape* ReadGeoModel::buildShape(QString shapeId)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildShape()";
	QStringList paramsShape = _shapes[ shapeId.toUInt() ];

	QString id = paramsShape[0];
	QString type = paramsShape[1];
	QString parameters = paramsShape[2];

	if (m_deepDebug) qDebug() << "\tShape-ID:" << id << ", Shape-type:" << type;

	if (type == "Box") {
			// shape parameters
			double XHalfLength = 0.;
			double YHalfLength = 0.;
			double ZHalfLength = 0.;
			// get parameters from DB string
			QStringList shapePars = parameters.split(";");
			foreach( QString par, shapePars) {
					QStringList vars = par.split("=");
					QString varName = vars[0];
					QString varValue = vars[1];
					if (varName == "XHalfLength") XHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "YHalfLength") YHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "ZHalfLength") ZHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			}
		return new GeoBox(XHalfLength, YHalfLength, ZHalfLength);
	}
	else if (type == "Cons") {
		// shape parameters
		double RMin1 = 0.;
		double RMin2 = 0.;
		double RMax1 = 0.;
		double RMax2 = 0.;
		double DZ = 0.;
		double SPhi = 0.;
		double DPhi = 0.;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "RMin1") RMin1 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "RMin2") RMin2 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "RMax1") RMax1 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "RMax2") RMax2 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "DZ") DZ = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "SPhi") SPhi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "DPhi") DPhi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
		}
		return new GeoCons (RMin1, RMin2, RMax1, RMax2, DZ, SPhi, DPhi);
	}
	else if (type == "Torus") {
		// Member Data:
		// * Rmax - outside radius of the torus tube
		// * Rmin - inside radius  of the torus tube (Rmin=0 if not hollow)
		// * Rtor - radius of the torus itself
		// *
		// * SPhi - starting angle of the segment in radians
		// * DPhi - delta angle of the segment in radians
		//
		// shape parameters
		double Rmin = 0.;
		double Rmax = 0.;
		double Rtor = 0.;
		double SPhi = 0.;
		double DPhi = 0.;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "Rmin") Rmin = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "Rmax") Rmax = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "Rtor") Rtor = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "SPhi") SPhi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "DPhi") DPhi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
		}
		return new GeoTorus (Rmin, Rmax, Rtor, SPhi, DPhi);
	}
	else if (type == "Para") {
		// shape parameters
		double XHalfLength = 0.;
		double YHalfLength = 0.;
		double ZHalfLength = 0.;
		double Alpha = 0.;
		double Theta = 0.;
		double Phi = 0.;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "XHalfLength") XHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "YHalfLength") YHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "ZHalfLength") ZHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "Alpha") Alpha = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "Theta") Theta = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "Phi") Phi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
		}
		return new GeoPara (XHalfLength, YHalfLength, ZHalfLength, Alpha, Theta, Phi);
	}
	else if (type == "Pcon") {
		// shape parameters
		double SPhi = 0.;
		double DPhi = 0.;
		unsigned int NZPlanes = 0;

		// get parameters from DB string
		QStringList shapePars = parameters.split(";");

		bool error = 0;
		QString par;
		QStringList vars;
		QString varName;
		QString varValue;

		GeoPcon* pcon = nullptr;

		int sizePars = shapePars.size();
		// check if we have more than 3 parameters
		if (sizePars > 3) {

			// get the three first GeoPcon parameters: the SPhi and DPhi angles, plus the number of Z planes
			for( int it=0; it < 3; it++) {
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				if (varName == "SPhi") SPhi = varValue.toDouble();
				if (varName == "DPhi") DPhi = varValue.toDouble();
				if (varName == "NZPlanes") NZPlanes = varValue.toDouble();
			}
			// build the basic GeoPcon shape
			pcon = new GeoPcon(SPhi, DPhi);

			// and now loop over the rest of the list, to get the parameters of all Z planes
			for (int it=3; it < sizePars; it++)
			{
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				// qInfo() << "it:" << it << "par:" << par << "varName:" << varName << "varValue:" << varValue;

				if (varName == "ZPos") {

					double zpos = varValue.toDouble();
					double rmin=0., rmax=0.;

					it++; // go to next variable

					par = shapePars[it];
					vars = par.split("=");
					varName = vars[0];
					varValue = vars[1];
					if (varName == "ZRmin") rmin = varValue.toDouble();
					else error = 1;
					it++; // go to next variable

					par = shapePars[it];
					vars = par.split("=");
					varName = vars[0];
					varValue = vars[1];
					if (varName == "ZRmax") rmax = varValue.toDouble();
					else error = 1;

					if(error) qWarning() << "ERROR! GeoPcon 'ZRmin' and 'ZRmax' values are not at the right place! --> " << shapePars;

					// add a Z plane to the GeoPcon
					pcon->addPlane(zpos, rmin, rmax);
				} else {
					error = 1;
					qWarning() << "ERROR! GeoPcon 'ZPos' value is not at the right place! --> " << shapePars;
				}
			}

			// sanity check on the resulting Pcon shape
			if( pcon->getNPlanes() != NZPlanes) {
				error = 1;
				qWarning() << "ERROR! GeoPcon number of planes: " << QString::number(pcon->getNPlanes()) << " is not equal to the original size! --> " << shapePars;
			}
			if(!pcon->isValid()) {
				error = 1;
				qWarning() << "ERROR! GeoPcon shape is not valid!! -- input: " << shapePars;
			}
	  } // end if (size>3)
		else {
			qWarning() << "ERROR!! GeoPcon has no Z planes!! --> shape input parameters: " << shapePars;
			error = 1;
		}

		if(error) qFatal("GeoPcon shape error!!! Aborting...");

		return pcon;
	}
	else if (type == "Pgon") {
		// shape parameters
		double SPhi = 0.;
		double DPhi = 0.;
		unsigned int NSides = 0;
		unsigned int NZPlanes = 0;

		bool error = false;
		GeoPgon* pgon = nullptr;
		QString par;
		QStringList vars;
		QString varName;
		QString varValue;

		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		// qInfo() << "shapePars: " << shapePars; // debug

		int sizePars = shapePars.size();
		// check if we have more than 3 parameters
		if (sizePars > 3) {

			// get the first four GeoPgon parameters: the SPhi and DPhi angles, plus the number of Z planes
			for( int it=0; it < 4; it++) {
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				// qInfo() << "vars: " << vars; // for debug only
				if (varName == "SPhi") SPhi = varValue.toDouble();
				if (varName == "DPhi") DPhi = varValue.toDouble();
				if (varName == "NSides") NSides = varValue.toUInt();// * SYSTEM_OF_UNITS::mm;
				if (varName == "NZPlanes") NZPlanes = varValue.toDouble();

			}
			// build the basic GeoPgon shape
			pgon = new GeoPgon(SPhi, DPhi, NSides);

			// and now loop over the rest of the list, to get the parameters of all Z planes
			for (int it=4; it < sizePars; it++)
			{
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				// qInfo() << "it:" << it << "par:" << par << "varName:" << varName << "varValue:" << varValue;

				if (varName == "ZPos") {

					double zpos = varValue.toDouble();
					double rmin=0., rmax=0.;

					it++; // go to next variable

					par = shapePars[it];
					vars = par.split("=");
					varName = vars[0];
					varValue = vars[1];
					if (varName == "ZRmin") rmin = varValue.toDouble();
					else error = 1;
					it++; // go to next variable

					par = shapePars[it];
					vars = par.split("=");
					varName = vars[0];
					varValue = vars[1];
					if (varName == "ZRmax") rmax = varValue.toDouble();
					else error = 1;

					if(error) qWarning() << "ERROR! GeoPgon 'ZRmin' and 'ZRmax' values are not at the right place! --> " << shapePars;

					// add a Z plane to the GeoPgon
					pgon->addPlane(zpos, rmin, rmax);
				} else {
					error = 1;
					qWarning() << "ERROR! GeoPgon 'ZPos' value is not at the right place! --> " << shapePars;
				}
			}

			// sanity check on the resulting Pgon shape
			if( pgon->getNPlanes() != NZPlanes) {
				error = 1;
				qWarning() << "ERROR! GeoPgon number of planes: " << QString::number(pgon->getNPlanes()) << " is not equal to the original size! --> " << shapePars;
			}
			if(!pgon->isValid()) {
				error = 1;
				qWarning() << "ERROR! GeoPgon shape is not valid!! -- input: " << shapePars;
			}
		} // end if (size>3)
		else {
			qWarning() << "ERROR!! GeoPgon has no Z planes!! --> shape input parameters: " << shapePars;
			error = 1;
		}
		if(error) qFatal("GeoPgon shape error!!! Aborting...");
		return pgon;
	}
	else if (type == "SimplePolygonBrep") {
		//qInfo() << "Reading-in: SimplePolygonBrep: "; // debug
		// shape parameters
		double DZ = 0.;
		unsigned int NVertices = 0;
		double xV = 0.;
		double yV = 0.;

		bool error = 0;
		GeoSimplePolygonBrep* sh = nullptr;
		QString par;
		QStringList vars;
		QString varName;
		QString varValue;

		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		//qInfo() << "shapePars: " << shapePars; // debug

		int sizePars = shapePars.size();
		// check if we have more than 2 parameters
		if (sizePars > 2) {

			// get the first two GeoSimplePolygonBrep parameters: DZ and the number of vertices.
			for( int it=0; it < 2; it++) {
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				// qInfo() << "vars: " << vars; // for debug only
				if (varName == "DZ") DZ = varValue.toDouble();
				if (varName == "NVertices") NVertices = varValue.toDouble();
				//else if (varName == "NVertices") NVertices = varValue.toDouble();
				//else error = 1;
				//if(error) qWarning() << "ERROR! GeoSimplePolygonBrep parameters are not correctly stored! -->" << vars;

			}
			// build the basic GeoSimplePolygonBrep shape
			sh = new GeoSimplePolygonBrep(DZ);

			// and now loop over the rest of the list, to get the parameters of all vertices
			for (int it=2; it < sizePars; it++)
			{
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				if (varName == "xV") xV = varValue.toDouble();
				else error = 1;

				it++; // go to next variable (they come in pairs)

				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				if (varName == "yV") yV = varValue.toDouble();
				else error = 1;

				if(error) qWarning() << "ERROR! GeoSimplePolygonBrep 'xVertex' and 'yVertex' values are not at the right place! --> " << shapePars;

				// add a Z plane to the GeoSimplePolygonBrep
				sh->addVertex(xV, yV);
			}
			// sanity check on the resulting shape
			if( sh->getNVertices() != NVertices) {
				error = 1;
				qWarning() << "ERROR! GeoSimplePolygonBrep number of planes: " << QString::number(sh->getNVertices()) << " is not equal to the original size! --> " << shapePars;
			}
			if(!sh->isValid()) {
				error = 1;
				qWarning() << "ERROR! GeoSimplePolygonBrep shape is not valid!! -- input: " << shapePars;
			}
		} // end if (size>3)
		else {
			qWarning() << "ERROR!! GeoSimplePolygonBrep has no vertices!! --> shape input parameters: " << shapePars;
			error = 1;
		}
		if(error) qFatal("GeoSimplePolygonBrep shape error!!! Aborting...");
		return sh;

	}
	else if (type == "TessellatedSolid") {
		// Tessellated pars example: "nFacets=1;TRI;vT=ABSOLUTE;nV=3;xV=0;yV=0;zV=1;xV=0;yV=1;zV=0;xV=1;yV=0;zV=0"
		qInfo() << "Reading-in: TessellatedSolid: "; // debug
		// Facet type
		QString facetType = "";
		// shape parameters
		unsigned int nFacets = 0;

		bool error = 0;
		GeoTessellatedSolid* sh = nullptr;
		QString par;
		QStringList vars;
		QString varName;
		QString varValue;

		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		qInfo() << "shapePars: " << shapePars; // debug

		int sizePars = shapePars.size();
		// check if we have at least 13 parameters,
		// which is the minimum for a shape
		// with a single triangular facet
		if (sizePars >= 13) {

			// get the first parameter
			par = shapePars[0];
			vars = par.split("=");
			varName = vars[0];
			varValue = vars[1];
			if (varName == "nFacets") nFacets = varValue.toInt();
			else {
				qWarning("ERROR!! - GeoTessellatedSolid - nFacets is not defined!!");
				error = true; // TODO: check "error.h" functionalities and replace with that if useful
			}

			// build the basic GeoTessellatedSolid shape
			sh = new GeoTessellatedSolid();

			// and now loop over the rest of the list,
			// to get the parameters of the vertices of each facet
			// and to build the full GeoTessellatedSolid with all the facets
			for (int it=1; it < sizePars; it++)
			{
				// get facet type
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				if (varName == "QUAD") {
					facetType = "QUAD";
				}
				else if (varName == "TRI") {
					facetType = "TRI";
				}
				else {
					qWarning() << "ERROR!! - GeoTessellatedSolid - Facet type is not defined! [got: '" << varName << "']";
					error = true;
				}

				it++; // advance to the next parameter

				// get the type of the vertexes composing the facet
				bool isAbsolute = true;
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				if (varName == "vT") {
					if (varValue == "ABSOLUTE") isAbsolute = true;
					else if (varValue == "RELATIVE") isAbsolute = false;
					else {
						qWarning() << "ERROR! - GeoTessellatedSolid - Vertex type not defined!";
						error=true;
					}
				}
				else error = 1;

				it++; // advance to the next parameter

				unsigned int nVertexes = 0;
				par = shapePars[it];
				vars = par.split("=");
				varName = vars[0];
				varValue = vars[1];
				if (varName == "nV") nVertexes = varValue.toUInt();
				else {
					qWarning() << "ERROR! - GeoTessellatedSolid - nVertices not defined!";
					error=true;
				}


				qDebug() << "it:" << it;
				// if we get a QUAD ==> GeoQuadrangularFacet
				if (facetType=="QUAD") {

					qDebug() << "Handling a QUAD facet...";
					// to store the 4 vertices of the GeoQuadrangularFacet
					auto vV = std::vector<std::unique_ptr<GeoFacetVertex>>{};

					// we look for 4 vertices for QUAD;
					// for each of them, we get 3 coordinates
					//					vertStart = it;
					for (unsigned int iV=0; iV<4; ++iV) {

						it++; // advance to the first of the facet's vertices' coordinates

						double xV=0.;
						par = shapePars[it];
						vars = par.split("=");
						varName = vars[0];
						varValue = vars[1];
						if (varName == "xV") xV = varValue.toDouble();
						else {
							qWarning() << "ERROR! Got '" << varName << "' instead of 'xV'!";
							error = 1;
						}

						it++; // go to the next coordinate

						double yV=0.;
						par = shapePars[it];
						vars = par.split("=");
						varName = vars[0];
						varValue = vars[1];
						if (varName == "yV") yV = varValue.toDouble();
						else {
							qWarning() << "ERROR! Got '" << varName << "' instead of 'yV'!";
							error = 1;
						}

						it++; // go to the next coordinate

						double zV=0.;
						par = shapePars[it];
						vars = par.split("=");
						varName = vars[0];
						varValue = vars[1];
						if (varName == "zV") zV = varValue.toDouble();
						else {
							qWarning() << "ERROR! Got '" << varName << "' instead of 'zV'!";
							error = 1;
						}

						if(error) qWarning() << "ERROR! GeoTessellatedSolid 'xV', 'yV', and 'zV' values are not at the right place! --> " << shapePars;

						// build the facet's vertex and store it
						vV.push_back(std::make_unique<GeoFacetVertex>( GeoFacetVertex(xV,yV,zV)) );
					}

					// build the facet and add it to the GeoTessellatedSolid
					GeoQuadrangularFacet* quadFacet = new GeoQuadrangularFacet(*vV[0], *vV[1], *vV[2], *vV[3], (isAbsolute ? GeoFacet::ABSOLUTE : GeoFacet::RELATIVE));
					sh->addFacet(quadFacet);
				}
				// if we get a TRI ==> GeoTriangularFacet
				else if (facetType=="TRI") {

					qDebug() << "Handling a TRI facet...";
//					std::vector<GeoFacetVertex*> vV(3, 0); // to store the 3 vertices of the GeoTriangularFacet
					auto vV = std::vector<std::unique_ptr<GeoFacetVertex>>{};

					// we look for 3 vertices for GeoTriangularFacet;
					// for each of them, we get 3 coordinates
//					vertStart = it;
					for (unsigned int iV=0; iV<3; ++iV) {

						it++; // advance to the first of the facet's vertices' coordinates

						double xV=0.;
						par = shapePars[it];
						vars = par.split("=");
						varName = vars[0];
						varValue = vars[1];
						if (varName == "xV") xV = varValue.toDouble();
						else error = 1;

						it++; // go to the next coordinate

						double yV=0.;
						par = shapePars[it];
						vars = par.split("=");
						varName = vars[0];
						varValue = vars[1];
						if (varName == "yV") yV = varValue.toDouble();
						else error = 1;

						it++; // go to the next coordinate

						double zV=0.;
						par = shapePars[it];
						vars = par.split("=");
						varName = vars[0];
						varValue = vars[1];
						if (varName == "zV") zV = varValue.toDouble();
						else error = 1;

						if(error) qWarning() << "ERROR! GeoTessellatedSolid 'xV', 'yV', and 'zV' values are not at the right place! --> " << shapePars;

						// build the facet's vertex and store it
						vV.push_back(std::make_unique<GeoFacetVertex>( GeoFacetVertex(xV,yV,zV)) );
					}

					// build the facet and add it to the GeoTessellatedSolid
					GeoTriangularFacet* triFacet = new GeoTriangularFacet(*vV[0], *vV[1], *vV[2], (isAbsolute ? GeoFacet::ABSOLUTE : GeoFacet::RELATIVE));
					sh->addFacet(triFacet);
				}


			}

			// sanity check on the resulting shape
			if( sh->getNumberOfFacets() != nFacets) {
				error = 1;
				qWarning() << "ERROR! GeoTessellatedSolid number of facets: " << QString::number(sh->getNumberOfFacets()) << " is not equal to the original size! --> " << shapePars;
			}
			/*
			 * TODO: uncomment it, when the isValid() method will be implemented for GeoTessellatedSolid
			if(!sh->isValid()) {
				error = 1;
				qWarning() << "ERROR! GeoTessellatedSolid shape is not valid!! -- input: " << shapePars;
			}
			*/
		} // END OF if (size>13)
		else {
			qWarning() << "ERROR!! GeoTessellatedSolid has no facets!! --> shape input parameters: " << shapePars;
			error = 1;
		}
		if(error) qFatal("GeoTessellatedSolid shape error!!! Aborting...");
		return sh;

	}
	else if (type == "Trap") {
			// shape constructor parameters
			double ZHalfLength = 0.;
			double Theta = 0.;
			double Phi = 0.;
			double Dydzn = 0.;
			double Dxdyndzn = 0.;
			double Dxdypdzn = 0.;
			double Angleydzn = 0.;
			double Dydzp = 0.;
			double Dxdyndzp = 0.;
			double Dxdypdzp = 0.;
			double Angleydzp = 0.;
			// get parameters from DB string
			QStringList shapePars = parameters.split(";");
			foreach( QString par, shapePars) {
					QStringList vars = par.split("=");
					QString varName = vars[0];
					QString varValue = vars[1];
					if (varName == "ZHalfLength") ZHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Theta") Theta = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Phi") Phi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Dydzn") Dydzn = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Dxdyndzn") Dxdyndzn = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Dxdypdzn") Dxdypdzn = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Angleydzn") Angleydzn = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Dydzp") Dydzp = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Dxdyndzp") Dxdyndzp = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Dxdypdzp") Dxdypdzp = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "Angleydzp") Angleydzp = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			}
		return new GeoTrap (ZHalfLength, Theta, Phi, Dydzn, Dxdyndzn, Dxdypdzn, Angleydzn, Dydzp, Dxdyndzp, Dxdypdzp, Angleydzp);
	}
	else if (type == "Trd") {
			// shape constructor parameters
			double XHalfLength1 = 0.;
			double XHalfLength2 = 0.;
			double YHalfLength1 = 0.;
			double YHalfLength2 = 0.;
			double ZHalfLength = 0.;
			// get parameters from DB string
			QStringList shapePars = parameters.split(";");
			foreach( QString par, shapePars) {
					QStringList vars = par.split("=");
					QString varName = vars[0];
					QString varValue = vars[1];
					if (varName == "XHalfLength1") XHalfLength1 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "XHalfLength2") XHalfLength2 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "YHalfLength1") YHalfLength1 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "YHalfLength2") YHalfLength2 = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
					if (varName == "ZHalfLength") ZHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			}
		return new GeoTrd (XHalfLength1, XHalfLength2, YHalfLength1, YHalfLength2, ZHalfLength);
	}
	else if (type == "Tube") {
		// shape parameters
		double RMin = 0.;
		double RMax = 0.;
		double ZHalfLength = 0.;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "RMin") RMin = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "RMax") RMax = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "ZHalfLength") ZHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
		}
		return new GeoTube(RMin, RMax, ZHalfLength);
	}
	else if (type == "Tubs") {
		// shape parameters
		double RMin = 0.;
		double RMax = 0.;
		double ZHalfLength = 0.;
		double SPhi = 0.;
		double DPhi = 0.;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "RMin") RMin = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "RMax") RMax = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "ZHalfLength") ZHalfLength = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "SPhi") SPhi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
			if (varName == "DPhi") DPhi = varValue.toDouble();// * SYSTEM_OF_UNITS::mm;
		}
		return new GeoTubs (RMin, RMax, ZHalfLength, SPhi, DPhi);
	}
	else if (type == "Intersection") {
		// shape parameters
		unsigned int opA = 0;
		unsigned int opB = 0;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "opA") opA = varValue.toUInt();
			if (varName == "opB") opB = varValue.toUInt();
		}
		// get the referenced shape
		const GeoShape* shapeA = buildShape( QString::number(opA) );
		const GeoShape* shapeB = buildShape( QString::number(opB) );
		// build and return the GeoShapeShift instance
		return new GeoShapeIntersection(shapeA, shapeB);
	}
	else if (type == "Shift") {
		// shape parameters
		unsigned int shapeId = 0;
		unsigned int transfId = 0;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "A") shapeId = varValue.toUInt();
			if (varName == "X") transfId = varValue.toUInt();
		}
		// get the referenced shape
		// const GeoShape* shapeA = getShape( QString::number(shapeId) );
		const GeoShape* shapeA = buildShape( QString::number(shapeId) );
		// get the referenced Transform
		QStringList transPars = m_dbManager->getItemFromTableName("Transforms", transfId);
		if (m_deepDebug) qDebug() << "child:" << transPars;
		GeoTransform* transf = parseTransform(transPars);
		const GeoTrf::Transform3D transfX = transf->getTransform();

		// qWarning() << "GeoShift:";
		// printTrf(transfX);

		// build and return the GeoShapeShift instance
		return new GeoShapeShift(shapeA, transfX);
	}
	else if (type == "Subtraction") {
		// shape parameters
		unsigned int opA = 0;
		unsigned int opB = 0;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "opA") opA = varValue.toUInt();
			if (varName == "opB") opB = varValue.toUInt();
		}
		// get the referenced shape

		//if (VP1Msg::debug())
    		//VP1Msg::messageDebug("GeoSubstraction - building shape A...\n");

		const GeoShape* shapeA = buildShape( QString::number(opA) );

		/*
		if (VP1Msg::debug()) {
			QString msg = QString::fromStdString(shapeA->type());
    			VP1Msg::messageDebug("GeoSubstraction - built shape A: "+msg);
    			VP1Msg::messageDebug("GeoSubstraction - building shape B...\n");
    		}
		*/

		const GeoShape* shapeB = buildShape( QString::number(opB) );

		/*
		if (VP1Msg::debug()) {
			QString msg = QString::fromStdString(shapeB->type());
    			VP1Msg::messageDebug("GeoSubstraction - built shape B: "+msg+"\n");
    		}
		*/

		// build and return the GeoShapeShift instance
		return new GeoShapeSubtraction(shapeA, shapeB);
	}
	else if (type == "Union") {
		// shape parameters
		unsigned int opA = 0;
		unsigned int opB = 0;
		// get parameters from DB string
		QStringList shapePars = parameters.split(";");
		foreach( QString par, shapePars) {
			QStringList vars = par.split("=");
			QString varName = vars[0];
			QString varValue = vars[1];
			if (varName == "opA") opA = varValue.toUInt();
			if (varName == "opB") opB = varValue.toUInt();
		}
		if (opA == 0 || opB == 0) std::cout << "ERROR! 'GeoUnion' shape: opA or opB have not been properly intialized!" << std::endl;
		// get the referenced shape
		const GeoShape* shapeA = buildShape( QString::number(opA) );
		const GeoShape* shapeB = buildShape( QString::number(opB) );
		// build and return the GeoShapeShift instance
		return new GeoShapeUnion(shapeA, shapeB);
	}
	else {
		// QString msg = "WARNING!! - Shape '" + type + "' not implemented yet!!! Returning a dummy cube.";
		// qWarning(msg.toStdString().c_str());
		m_unknown_shapes.insert(type.toStdString()); // save unknwon shapes for later warning message
		return new GeoBox(30.0*SYSTEM_OF_UNITS::cm, 30*SYSTEM_OF_UNITS::cm, 30*SYSTEM_OF_UNITS::cm); // FIXME: bogus shape. Use actual shape!
	}

}


GeoLogVol* ReadGeoModel::buildLogVol(QString logVolId)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildLogVol()";

	// get logVol properties from the DB
	QStringList values = _logVols[logVolId.toUInt()];
	if (m_deepDebug) qDebug() << "params:" << values;

	// build the LogVol
	QString logVolName = values[1];

	// GET LOGVOL SHAPE
	QString shapeId = values[2];
	GeoShape* shape = buildShape(shapeId);

	// GET LOGVOL MATERIAL
	QString matId = values[3];
	if (m_deepDebug) qDebug() << "material Id:" << matId;
	GeoMaterial* mat = buildMaterial(matId);

	return new GeoLogVol(logVolName.toStdString(), shape, mat);

}


GeoSerialDenominator* ReadGeoModel::buildSerialDenominator(QString id)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildSerialDenominator()";
	return parseSerialDenominator( _serialDenominators[id.toUInt()] );
}

GeoSerialDenominator* ReadGeoModel::parseSerialDenominator(QStringList values)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::parseSerialDenominator()";
	QString id = values[0];
	QString baseName = values[1];
	if (m_deepDebug) qDebug() << "\tID:" << id << ", base-name:" << baseName;
	return new GeoSerialDenominator(baseName.toStdString());
}

// TODO: this should be moved to an Utilities class!
void ReadGeoModel::printTrf(GeoTrf::Transform3D t) {
	std::cout << "transformation: " << std::endl;
	std::cout << "[[" << t(0, 0) << " , ";
	std::cout <<         t(0, 1) << " , ";
	std::cout <<         t(0, 2) << " , ";
	std::cout <<         t(0, 3) << " ]";
	std::cout << "["  << t(1, 0) << " , ";
	std::cout <<         t(1, 1) << " , ";
	std::cout <<         t(1, 2) << " , ";
	std::cout <<         t(1, 3) << " ]";
	std::cout << "["  << t(2, 0) << " , ";
	std::cout <<         t(2, 1) << " , ";
	std::cout <<         t(2, 2) << " , ";
	std::cout <<         t(2, 3) << " ]";
	std::cout << "["  << t(3, 0) << " , ";
	std::cout <<         t(3, 1) << " , ";
	std::cout <<         t(3, 2) << " , ";
	std::cout <<         t(3, 3) << " ]]" << std::endl;
}

// TODO: should go in a QtUtils header-only class, to be used in other packages
QList<double> ReadGeoModel::convertQstringListToDouble(QStringList listin) {
	QList<double> listout;
  foreach (const QString &s, listin) {
      listout.append(s.toDouble());
  }
	return listout;
}

void ReadGeoModel::printTransformationValues(QStringList values) {
	QList<double> t = convertQstringListToDouble(values);
	std::cout << "transformation input values: " << std::endl;
	qWarning() << "[[" << t[0] << "," << t[1] << "," << t[2] << "]["
	                   << t[3] << "," << t[4] << "," << t[5] << "]["
	                   << t[6] << "," << t[7] << "," << t[8] << "]["
	                   << t[9] << "," << t[10] << "," << t[11] << "]]";
}


GeoAlignableTransform* ReadGeoModel::buildAlignableTransform(QString id)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildAlignableTransform()";
	return parseAlignableTransform( _alignableTransforms[id.toUInt()] );
}

GeoAlignableTransform* ReadGeoModel::parseAlignableTransform(QStringList values)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::parseAlignableTransform()";

	QString id = values.takeFirst(); // it pops out the first element, leaving the other items in the list

	// printTransformationValues(values); // DEBUG

	// get the 12 matrix elements
	double xx = values[0].toDouble();
	double xy = values[1].toDouble();
	double xz = values[2].toDouble();

	double yx = values[3].toDouble();
	double yy = values[4].toDouble();
	double yz = values[5].toDouble();

	double zx = values[6].toDouble();
	double zy = values[7].toDouble();
	double zz = values[8].toDouble();

	double dx = values[9].toDouble();
	double dy = values[10].toDouble();
	double dz = values[11].toDouble();

	GeoTrf::Transform3D txf;
	// build the rotation matrix with the first 9 elements
	txf(0,0)=xx;
	txf(0,1)=xy;
	txf(0,2)=xz;

	txf(1,0)=yx;
	txf(1,1)=yy;
	txf(1,2)=yz;

	txf(2,0)=zx;
	txf(2,1)=zy;
	txf(2,2)=zz;

	// build the translation matrix with the last 3 elements
	txf(0,3)=dx;
	txf(1,3)=dy;
	txf(2,3)=dz;

	// printTrf(txf); // DEBUG
	return new GeoAlignableTransform(txf);
}

GeoTransform* ReadGeoModel::buildTransform(QString id)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildTransform()";
	return parseTransform( _transforms[id.toUInt()] );
}


GeoTransform* ReadGeoModel::parseTransform(QStringList values)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::parseTransform()";
	if (m_deepDebug) qDebug() << "values:" << values;

	QString id = values.takeFirst(); // it pops out the first element, the 'id', leaving the other items in the list

  // printTransformationValues(values); // DEBUG

	// get the 12 matrix elements
	double xx = values[0].toDouble();
	double xy = values[1].toDouble();
	double xz = values[2].toDouble();

	double yx = values[3].toDouble();
	double yy = values[4].toDouble();
	double yz = values[5].toDouble();

	double zx = values[6].toDouble();
	double zy = values[7].toDouble();
	double zz = values[8].toDouble();

	double dx = values[9].toDouble();
	double dy = values[10].toDouble();
	double dz = values[11].toDouble();

	GeoTrf::Transform3D txf;
	// build the rotation matrix with the first 9 elements
	txf(0,0)=xx;
	txf(0,1)=xy;
	txf(0,2)=xz;

	txf(1,0)=yx;
	txf(1,1)=yy;
	txf(1,2)=yz;

	txf(2,0)=zx;
	txf(2,1)=zy;
	txf(2,2)=zz;

	// build the translation matrix with the last 3 elements
	txf(0,3) = dx;
	txf(1,3) = dy;
	txf(2,3) = dz;

	// printTrf(txf); // DEBUG
	return new GeoTransform(txf);
}


GeoSerialTransformer* ReadGeoModel::buildSerialTransformer(QString nodeId)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildSerialTransformer()";

	QStringList values = _serialTransformers[nodeId.toUInt()];
	if (m_deepDebug) qDebug() << "values:" << values;

	// std::cout <<"ST * " << values[0].toStdString() << " " << values[1].toStdString() << " " << values[2].toStdString() << std::endl;
	if (m_deepDebug) qDebug() << "ST * " << values[0] << ", " << values[1] << ", " << values[2] << ", " << values[3] << ", " << values[4];

	QString id = values[0];
	QString functionId = values[1];
	QString physVolId = values[2];
	QString physVolTableIdStr = values[3];
	QString copies = values[4];

	unsigned int physVolTableId = physVolTableIdStr.toUInt();

	QString physVolType = _tableid_tableName[physVolTableId];

	if (m_deepDebug) qDebug() << "\tID:" << id << ", functionId:" << functionId << ", physVolId:" << physVolId << ", physVolTableId:" << physVolTableId << ", copies:" << copies;

	// GET FUNCTION
	if (m_deepDebug) qDebug() << "function Id:" << functionId;
	TRANSFUNCTION func = buildFunction(functionId);

	// GET PHYSVOL
	if (m_deepDebug) qDebug() << "referenced physVol - Id:" << physVolId << ", type:" << physVolType << "tableId:" << physVolTableIdStr;
	const GeoVPhysVol* physVol = buildVPhysVol(physVolId, physVolTableIdStr, "1"); // we use "1" as default copyNumber: taking the first copy of the VPhysVol as the referenced volume
	//qDebug() << "physVol:" << physVol << ", function:" << &func;

	// get PhysVol or FullPhysVol pointer and return the SerialTransformer
	if (dynamic_cast<const GeoFullPhysVol*>(physVol)) {
		const GeoFullPhysVol* vol = dynamic_cast<const GeoFullPhysVol*>(physVol);
		return new GeoSerialTransformer(vol, &func, copies.toUInt() );
	}
	const GeoPhysVol* vol = dynamic_cast<const GeoPhysVol*>(physVol);
	return new GeoSerialTransformer(vol, &func, copies.toUInt() );

}

TRANSFUNCTION ReadGeoModel::buildFunction(QString id)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildFunction()";

	// return parseFunction( _functions[id.toUInt()] );

	// return parseFunction( _functions[id.toUInt()] );

	QStringList values = _functions[id.toUInt()];
	// return parseFunction( values[0].toUInt(), values[1].toStdString() );
	return parseFunction( values[1].toStdString() );


}

TRANSFUNCTION ReadGeoModel::parseFunction(const std::string& expr)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::parseFunction(const std::string& expr)";
	// qDebug() << "id:" << Qstring::number(id) << " - expression: " << QString::fromStdString(expr);
	if (m_deepDebug) qDebug() << "expression: " << QString::fromStdString(expr);

	if (expr.empty()) {
			qFatal("FATAL ERROR!! Function expression is empty!! Aborting...");
			abort();
	}

	TransFunctionInterpreter interpreter;
	if (m_deepDebug) qDebug() << "expression:" << QString::fromStdString(expr);
	TFPTR func=interpreter.interpret( expr );
	if (m_deepDebug) qDebug() << "expression interpreted";
	return *(func.release()); // make func returns a pointer to the managed object and releases the ownership, then get the object dereferencing the pointer
}

GeoNameTag* ReadGeoModel::buildNameTag(QString id)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::buildNameTag()";
	return parseNameTag( _nameTags[id.toUInt()] );
}

GeoNameTag* ReadGeoModel::parseNameTag(QStringList values)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::parseNameTag()";
	QString id = values[0];
	QString name = values[1];
	if (m_deepDebug) qDebug() << "nameTag:" << name;
	return new GeoNameTag(name.toStdString());
}


bool ReadGeoModel::isNodeBuilt(const QString id, const QString tableId, const QString copyNumber)
{
	// qDebug() << "ReadGeoModel::isNodeBuilt(): " << id << tableId << copyNumber;
	QString key = id + ":" + tableId + ":" + copyNumber;
	return _memMap.contains(key);
}


GeoGraphNode* ReadGeoModel::getNode(const QString id, const QString tableId, const QString copyN)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::getNode(): " << id << tableId << copyN;
	QString key = id + ":" + tableId + ":" + copyN;
	return _memMap[key];
}

void ReadGeoModel::storeNode(const QString id, const QString tableId, const QString copyN, GeoGraphNode* node)
{
	if (m_deepDebug) qDebug() << "ReadGeoModel::storeNode(): " << id << tableId << copyN << node;
	QString key = id + ":" + tableId + ":" + copyN;
	_memMap[key] = node;
}


} /* namespace GeoModelIO */
