
# GeoModel --- A user-friendly C++ Toolkit for HEP Detector Description

GeoModel is a user-friendly C++ Toolkit and Suite for HEP Detector Description with minimal dependencies.

## Dependencies

The philosophy of the whole GeoModel Suite is to keep the external dependencies at the minimum.

_list of all the dependencies coming soon_

**NOTE:** Please note that you can build some of the dependencies as part of the main build. This is useful for platforms where the versions installed by the system package managers are old. See [here below](#building-dependencies-as-part-of-the-main-build) for instructions.

## Build

### Build tree

By default, only the base classes are built:

- GeoModelCore
  - GeoModelKernel
  - GeoGenericFunctions
- GeoModelIO
  - GeoModelDBManager
  - GeoModelWrite
  - GeoModelRead
  - TFPersistification
- GeoModelTools
  - ExpressionEvaluator
  - GeoModelJSONParser
  - GeoModelXMLParser
  - GMCAT

On request, additional packages, libraries, and tools can be built:

- GeoModelVisualization (`gmex`)
- GeoModelExamples
- GeoModelG4
  - GeoMaterial2G4
  - GeoModel2G4
- FullSimLight

**Note:**
Some of the optional packages bring in additional dependencies, for example libraries to handle 3D graphics and Geant4 for standalone detector simulations.
In particular, these are the additional dependencies and the packages that bring them:

- GeoModelVisualization: Qt5, Coin3D, SoQt
- Examples about the classes handling the GeoModel->Geant4 translation: Geant4
- GeoModelG4: Geant4

### Build options

While configuring the build, you can enable the build of the optional packages, as explained in the following.


#### Build the visualization tools

```
cmake -DGEOMODEL_BUILD_VISUALIZATION=1 ../GeoModel
```

will enable the build of the geometry visualization tool, `GeoModelExplorer` (`gmex`) and of all the base classes. This brings in additional dependencies for the GUI and to handle 3D graphics: Qt5, Coin3D, and SoQt.


#### Build the examples


```
cmake -DGEOMODEL_BUILD_EXAMPLES=1 ../GeoModel
```

will enable the build of all the examples, except those requiring Geant4, and of all the base classes.

While this:

```
cmake  -DGEOMODEL_BUILD_EXAMPLES_W_GEANT4=1 ../GeoModel
```

will enable the build of all the examples, also those requiring Geant4. This option will also build `GeoModelG4`, the interface between GeoModel and GEant4, and brings in the dependency on a Geant4 installation.


#### Build the GeoModel --> Geant4 interface

This CMake command

```
cmake  -DGEOMODEL_BUILD_GEOMODELG4=1 ../GeoModel
```

will build the interface classes which translate GeoModel nodes into Geant4 entities, as well as all the base GeoModel classes.
This also brings in an additional dependency on a Geant4 installation.


#### Build the standalone Geant4-based detector simulation application

This CMake command

```
cmake  -DGEOMODEL_BUILD_FULLSIMLIGHT=1 ../GeoModel
```

will build `FullSimLight`, the Geant4-based application which let users run standalone simulations on a complete detector geometry or on a piece of that, as well as `GeoModelG4` and all the base classes. This also brings in an additional dependency on a Geant4 installation.



**Note:**
When used during the CMake configuration step, all the variables must be prefixed by `-D`, like in the first example of this section. You can also combine them.

For example, this CMake command:

```
cmake -DGEOMODEL_BUILD_VISUALIZATION=1 -DGEOMODEL_BUILD_EXAMPLES=1 ../GeoModel
```

will build the base classes, the visualization tool (`gmex`), and the examples (the ones that not require Geant4).


## Building dependencies as part of the main build

You can use CMake compilation flags to configure a built-in build of some of the dependencies. That options will download and build a tested version of those dependencies as part of the build process of GeoModel.

This is especially useful on platforms where the version of those dependencies installed by the system package manager is old, like on Ubuntu.

*Note:* You don't need to use those options on macOS, where the versions installed by the `brew` package manager are often the latest.

### nlohmann_json

You can enable the built-in build of `nlohmann_json` by using the CMake option:

```
-DGEOMODEL_USE_BUILTIN_JSON=TRUE
```

### Xerces-C

You can enable the built-in build of `Xerces-C` by using the CMake option:

```
-DGEOMODEL_USE_BUILTIN_XERCESC=TRUE
```

### Examples

For example, you can build the base GeoModel packages, plus the GeoModelVisualization (`gmex`), and a built-in version of nlohmann_json and Xerces-C by configuring the build with:

```
cmake -DCMAKE_INSTALL_PREFIX=../install/ -DGEOMODEL_USE_BUILTIN_JSON=TRUE -DGEOMODEL_BUILD_VISUALIZATION=1 -DGEOMODEL_USE_BUILTIN_XERCESC=TRUE ../GeoModel/
```
