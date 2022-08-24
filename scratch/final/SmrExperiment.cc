#include "SmrExperiment.h"

#include "ns3/internet-module.h"
#include "UdpApplication.h"
#include <iostream>
#include <string>

using std::ofstream;
using std::string;

NS_LOG_COMPONENT_DEFINE ("SmrExperiment");

SmrExperiment::SmrExperiment (int run, int nSender, int nReceiver) : LispExperiment (run, nSender, nReceiver)
{
  this->m_mobilityHandler = &SmrExperiment::MobilityHandler;
  this->m_prefix = "-smr";
  this->m_mobilityTime = Seconds (35);
  this->m_defaultMappingTTL = 60;
}

void SmrExperiment::SetConfig ()
{
  this->LispExperiment::SetConfig ();
}

void SmrExperiment::InstallApplications ()
{
  this->LispExperiment::InstallApplications ();

  for (int j = 0; j < this->m_nSender; j++)
    {
      string js = std::to_string (j);
      InstallUdpReceiver (this->m_topology->GetNode ("host1_" + js), Seconds (1.0), this->m_endTime);
      for (int i = 0; i < this->m_nReceiver; i++)
        {
          string is = std::to_string (i);
          InstallUdpSender (this->m_topology->GetNode ("host0_" + is), this->m_topology->GetNode ("host1_" + js),
                            Seconds (2.0), this->m_endTime);
        }
    }
}

void SendSmr (Ptr<LispTopology> topology, int receiverId, Ptr<const Packet> packet)
{
  string is = std::to_string (receiverId);
  Ptr<LispTopologyNode> xTR0_a = topology->GetNode ("xTR0a_" + is);
  uint32_t n_app = xTR0_a->GetNode ()->GetNApplications ();
  for (uint32_t i = 0; i < n_app; i++)
    {
      Ptr<Application> app = xTR0_a->GetNode ()->GetApplication (i);
      Ptr<LispEtrItrApplication> xTR = DynamicCast<LispEtrItrApplication, Application> (app);
      if (xTR != nullptr)
        {
          xTR->SendSmrMsgForEID (Create<EndpointId> (topology->GetNode ("host0_" + is)->GetAddress (), Ipv4Mask ("255.255.255.0")));
        }
    }
}

void SmrExperiment::MobilityHandler (Ptr<LispTopology> topology, ofstream *log, int nReceiver)
{
  LispExperiment::MobilityHandler (topology, log, nReceiver);

  for (int i = 0; i < nReceiver; i++)
    {
      string is = std::to_string (i);
      Callback<void, Ptr<const Packet> > callback =
        Callback<void, Ptr<LispTopology>, int, Ptr<const Packet> > (&SendSmr, false, false)
        .Bind (topology).Bind (i);

      Ptr<LispTopologyNode> xTR0_b = topology->GetNode ("xTR0b_" + is);
      for (uint32_t j = 0; j < xTR0_b->GetNode ()->GetNApplications (); j++)
        {
          Ptr<Application> app = xTR0_b->GetNode ()->GetApplication (j);
          auto xtr = DynamicCast<LispEtrItrApplication, Application> (app);
          if (xtr != nullptr)
            {
              xtr->TraceConnectWithoutContext ("MapNotifyRx", callback);
            }
        }
    }
}
