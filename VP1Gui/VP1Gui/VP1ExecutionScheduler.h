/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         //
//  Header file for class VP1ExecutionScheduler            //
//                                                         //
//  Author: Thomas Kittelmann <Thomas.Kittelmann@cern.ch>  //
//                                                         //
//  Initial version: April 2007                            //
//                                                         //
/////////////////////////////////////////////////////////////

#ifndef VP1EXECUTIONSCHEDULER_H
#define VP1EXECUTIONSCHEDULER_H

// include C++
//#include <stddef.h> // this to fix the 'ptrdiff_t' does not name a type error with Qt (http://qt-project.org/forums/viewthread/16992)

// include VP1
#include "VP1Gui/VP1QtApplication.h"

// include Qt
#include <QObject>
#include <QStringList>


class IVP1System;
class IVP1ChannelWidget;

class StoreGateSvc;
class IToolSvc;
class ISvcLocator;
class VP1AvailEvents;

class VP1ExecutionScheduler : public QObject {

  Q_OBJECT

public:





  //init/cleanup:
  static VP1ExecutionScheduler* init();
  static void cleanup(VP1ExecutionScheduler*);

  //Call when new event data are available (returns false when the user closes the program)
  bool interact();//

  VP1ExecutionScheduler(QObject * parent,
			VP1AvailEvents * availEvents);
  virtual ~VP1ExecutionScheduler();

  void bringFromConstructedToReady(IVP1ChannelWidget*);
  void uncreateAndDelete(IVP1ChannelWidget*);

  bool isRefreshing() const;

  bool hasAllActiveSystemsRefreshed(IVP1ChannelWidget*) const;


  QStringList userRequestedFiles();

  QString saveSnaphsotToFile(IVP1System* s, bool batch = false);


signals:
  void refreshingStatusChanged(bool);



private:
  class Imp;
  Imp * m_d;
  void refreshSystem(IVP1System*);
  void eraseSystem(IVP1System*);
  void actualUncreateAndDelete(IVP1ChannelWidget*);

  #if defined BUILDVP1LIGHT
    xAOD::TEvent* m_event;
    ::TFile* m_ifile;
    QList<QStringList> m_list;
    int m_evtNr = 0;
    int m_totEvtNr = -1;
    bool m_goBackFlag = false;
    bool firstlaunch = true;
  #endif // BUILDVP1LIGHT

private slots:
  void processSystemForRefresh();
  void updateProgressBarDuringRefresh();
  void channelCreated(IVP1ChannelWidget*);
  void channelUncreated(IVP1ChannelWidget*);
  void startRefreshQueueIfAppropriate();
  void systemNeedErase();


  #if defined BUILDVP1LIGHT
    void passEvent(IVP1System*);
  #endif // BUILDVP1LIGHT
};

#endif
