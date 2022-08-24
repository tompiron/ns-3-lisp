#ifndef GUARD_PUB_SUB_EXPERIMENT
#define GUARD_PUB_SUB_EXPERIMENT

#include "ns3/core-module.h"

#include "LispExperiment.h"
#include "LispTopology.h"

using namespace ns3;

class PubSubExperiment : public LispExperiment
{
public:
  PubSubExperiment (int run, int nSender, int nReceiver);
  virtual ~PubSubExperiment ()
  {
  }

protected:
  virtual void SetConfig ();
};

#endif
