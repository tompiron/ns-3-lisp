#ifndef GUARD_SMR_PROXY_EXPERIMENT
#define GUARD_SMR_PROXY_EXPERIMENT

#include "ns3/core-module.h"

#include "SmrExperiment.h"
#include "LispTopology.h"

using namespace ns3;

class SmrProxyExperiment : public SmrExperiment
{
public:
  SmrProxyExperiment (int run, int nSender, int nReceiver);
  virtual ~SmrProxyExperiment ()
  {
  }

protected:
  virtual void SetConfig ();
};

#endif
