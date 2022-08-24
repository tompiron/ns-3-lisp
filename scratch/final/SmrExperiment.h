#ifndef GUARD_SMR_EXPERIMENT
#define GUARD_SMR_EXPERIMENT

#include <iostream>

#include "ns3/core-module.h"

#include "LispExperiment.h"
#include "LispTopology.h"

using namespace ns3;

class SmrExperiment : public LispExperiment
{
public:
  SmrExperiment (int run, int nSender, int nReceiver);
  virtual ~SmrExperiment ()
  {
  }

protected:
  virtual void SetConfig ();
  virtual void InstallApplications ();

  static void MobilityHandler (Ptr<LispTopology> topology, std::ofstream *log, int nReceiver);
};

#endif
