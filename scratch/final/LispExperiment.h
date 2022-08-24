#ifndef GUARD_LISP_EXPERIMENT
#define GUARD_LISP_EXPERIMENT

#include <string>

#include "ns3/core-module.h"

#include "Experiment.h"
#include "LispTopology.h"

using namespace ns3;

class LispExperiment : public Experiment
{
public:
  LispExperiment (int run, int nSender, int nReceiver, int defaultMappingTTL = 60);
  virtual ~LispExperiment ()
  {
  }
  virtual int Run ();

protected:
  virtual void SetConfig ();
  virtual void InitTopology ();
  virtual void InstallLisp ();
  virtual void InitCsmaGroups ();
  virtual void InstallApplications ();

  static void MobilityHandler (Ptr<LispTopology> topology, std::ofstream *log, int nReceiver);
  static void LogEvent (std::ofstream *output, std::string event);
  void LogMappingUpdate (Ptr<const Packet> packet);

protected:
  Ptr<LispTopology> m_topology;
  void (*m_mobilityHandler)(Ptr<LispTopology>, std::ofstream*, int);
  std::string m_prefix;
  std::ofstream m_output;
  Time m_endTime;
  Time m_mobilityTime;
  int m_defaultMappingTTL;
  std::map<std::string, std::vector<uint32_t> > m_updatedSender;
};

#endif
