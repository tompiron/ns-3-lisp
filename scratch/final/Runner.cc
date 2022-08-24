#include "Runner.h"

#include "ns3/core-module.h"

#include "LispExperiment.h"
#include "SmrExperiment.h"
#include "SmrProxyExperiment.h"
#include "PubSubExperiment.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Runner");

Runner::Runner (int nRuns, int seed, int nSender, int nReceiver) :
  m_nRuns (nRuns), m_seed (seed), m_nSender (nSender), m_nReceiver (nReceiver)
{
}

template <class E>
int runExperiment (int run, int nSender, int nReceiver)
{
  Ptr<Experiment> experiment = Create<E> (run, nSender, nReceiver);
  int result = experiment->Run ();
  if (result == 0)
    {
      return 0;
    }
  NS_LOG_ERROR ("Experiment number " << run << " failed.");
  return result;
}

int Runner::Run ()
{
  RngSeedManager::SetSeed (m_seed);
  for (int i = 1; i <= m_nRuns; i++)
    {
      NS_LOG_INFO ("Starting run " << i << ".");
      RngSeedManager::SetRun (i);
      int result;

      NS_LOG_INFO ("Starting LispExperiment.");
      result = runExperiment<LispExperiment> (i, m_nSender, m_nReceiver);
      if (result != 0)
        {
          return result;
        }

      NS_LOG_INFO ("Starting SmrExperiment.");
      result = runExperiment<SmrExperiment> (i, m_nSender, m_nReceiver);
      if (result != 0)
        {
          return result;
        }

      NS_LOG_INFO ("Starting SmrProxyExperiment.");
      result = runExperiment<SmrProxyExperiment> (i, m_nSender, m_nReceiver);
      if (result != 0)
        {
          return result;
        }

      NS_LOG_INFO ("Starting PubSubExperiment.");
      result = runExperiment<PubSubExperiment> (i, m_nSender, m_nReceiver);
      if (result != 0)
        {
          return result;
        }
    }
  return 0;
}
