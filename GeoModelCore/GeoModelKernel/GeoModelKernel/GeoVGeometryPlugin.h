/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOVGEOMETRYPLUGIN_H
#define GEOMODELKERNEL_GEOVGEOMETRYPLUGIN_H

/**
 * @class: GeoVGeometryPlugin
 *
 * @brief Abstact Base Class for a geometry plugin.
 *	  The geometry plugin builds the raw geometry
 */

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoPublisher.h" // to publish lists of FullPhysVol and AlignableTransform nodes in create()

#include <memory> // smart pointers

class GeoPhysVol;
class GeoVGeometryPlugin 
{
 public:

  //! Default constructor.
  GeoVGeometryPlugin() {};
  
  //! Constructor used when publishing FullPhysVol/AlignableTransform nodes is needed.
  /// The constructor is a 'sink': it takes ownership of the pointer to the GeoPublisher instance. You should use the constructor by passing the ownership of a unique pointer to it:
  /// @code
  /// class ToyGeometryPlugin : public GeoVGeometryPlugin  {
  ///   public:
  ///     ToyGeometryPlugin( std::string pluginName, std::unique_ptr<GeoPublisher> publisher ) : GeoVGeometryPlugin( pluginName, std::move(publisher) ) {};
  ///      ...
  ///  };
  ///
  ///  int main() {
  ///    auto publisher = std::make_unique<GeoPublisher>();
  ///    ToyGeometryPlugin* toy = new ToyGeometryPlugin( "ToyGeometryPlugin", std::move(publisher) );
  ///    ...
  ///  }
  /// @endcode
  GeoVGeometryPlugin(std::string name, std::unique_ptr<GeoPublisher> publisher) : m_pluginName( name ) { m_publisher = std::move(publisher);  m_publisher->setName(m_pluginName); } 
  
  virtual ~GeoVGeometryPlugin() {};

  //! Create the system geometry.
  // Note: this is a pure virtual method, so you need to implement it in your derived plugin class
  virtual void create (GeoPhysVol* world, GeoPublisher* publisher = nullptr) = 0;

  // Get the plugin's name
  std::string getName() { return m_pluginName; };

  // Get the Publisher that publishes the lists of the GeoFullPhysVol and AlignableTransform nodes
  GeoPublisher* getPublisher() { return m_publisher.get(); };

 private:

  GeoVGeometryPlugin(const GeoVGeometryPlugin &right)=delete;
  GeoVGeometryPlugin & operator=(const GeoVGeometryPlugin &right)=delete;

  //! The name of the plugin, used to store plugin's published nodes. 
  std::string m_pluginName;

  //! A GeoPublisher instance is used to publish lists of nodes.
  //GeoPublisher* m_publisher;
  std::unique_ptr<GeoPublisher> m_publisher;

};

#endif
