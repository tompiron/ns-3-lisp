#ifndef GUARD_EXPERIMENT
#define GUARD_EXPERIMENT

#include "ns3/core-module.h"

using namespace ns3;

class Experiment : public SimpleRefCount<Experiment>
{
public:
  Experiment (int run, int nSender, int nReceiver) : m_run (run), m_nSender (nSender), m_nReceiver (nReceiver)
  {
  }
  virtual ~Experiment ()
  {
  }
  virtual int Run () = 0;

protected:
  int m_run, m_nSender, m_nReceiver;
};

#endif
