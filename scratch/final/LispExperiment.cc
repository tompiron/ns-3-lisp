#include "LispExperiment.h"

#include <vector>

#include "ns3/core-module.h"

#include "LispTopology.h"
#include "UdpApplication.h"

#include <iostream>

using namespace ns3;
using std::vector;
using std::ofstream;
using std::string;
using std::unique;
using std::sort;
using std::ostringstream;
using std::map;
using std::endl;
NS_LOG_COMPONENT_DEFINE ("LispExperiment");

LispExperiment::LispExperiment (int run, int nSender, int nReceiver, int defaultMappingTTL) :
  Experiment (run, nSender, nReceiver)
{
  this->m_mobilityHandler = &LispExperiment::MobilityHandler;
  this->m_prefix = "-nothing";
  this->m_endTime = Minutes (5.0);
  this->m_defaultMappingTTL = defaultMappingTTL;
  Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
  var->SetAttribute ("Min", DoubleValue (35));
  var->SetAttribute ("Max", DoubleValue (35 + defaultMappingTTL));
  this->m_mobilityTime = Seconds (var->GetValue ());
}

int LispExperiment::Run ()
{
  m_output = ofstream ("output/" + std::to_string (m_run) + m_prefix + "-log.txt");
  this->SetConfig ();

  this->InitTopology ();
  this->InstallLisp ();
  this->InitCsmaGroups ();
  this->InstallApplications ();

  // this->m_topology->EnablePcapFiles ("output/" + std::to_string (this->m_run) + this->m_prefix);

  Simulator::Schedule (this->m_mobilityTime, this->m_mobilityHandler, this->m_topology, &m_output, this->m_nReceiver);

  Simulator::Run ();
  Simulator::Destroy ();
  m_output.close ();

  return 0;
}

void LispExperiment::SetConfig ()
{
  m_output << "Configuration:" << endl;
  m_output << "Senders: " << m_nSender << endl;
  m_output << "Receivers: " << m_nReceiver << endl;

  PacketMetadata::Enable ();

  Ptr<EmpiricalRandomVariable> mappingSystemRtt = CreateObject<EmpiricalRandomVariable> ();
  mappingSystemRtt->CDF (0.0, 0.0);
  mappingSystemRtt->CDF (0.00369, 0.002);
  mappingSystemRtt->CDF (0.01254, 0.5);
  mappingSystemRtt->CDF (0.02451, 0.62);
  mappingSystemRtt->CDF (0.04453, 0.7);
  mappingSystemRtt->CDF (0.08334, 0.744);
  mappingSystemRtt->CDF (0.12037, 0.81);
  mappingSystemRtt->CDF (0.31020, 0.90);
  mappingSystemRtt->CDF (1.00732, 0.91);
  mappingSystemRtt->CDF (2.80550, 0.952);
  mappingSystemRtt->CDF (9.84967, 0.999);
  mappingSystemRtt->CDF (29.95882, 1.0);
  Config::SetDefault ("ns3::MapServer::MappingSystemRttVariable", PointerValue (mappingSystemRtt));

  Ptr<EmpiricalRandomVariable> xtrToXtr = CreateObject<EmpiricalRandomVariable> ();
  xtrToXtr->CDF (0.0, 0.0);
  xtrToXtr->CDF (0.00323, 0.004);
  xtrToXtr->CDF (0.02929, 0.24);
  xtrToXtr->CDF (0.04107, 0.27);
  xtrToXtr->CDF (0.09083, 0.63);
  xtrToXtr->CDF (0.13061, 0.69);
  xtrToXtr->CDF (0.15180, 0.89);
  xtrToXtr->CDF (0.20113, 0.96);
  xtrToXtr->CDF (0.25172, 0.985);
  xtrToXtr->CDF (0.35139, 0.999);
  xtrToXtr->CDF (1.00026, 1.0);
  Config::SetDefault ("ns3::MapServer::MapServerToXtrDelayVariable", PointerValue (xtrToXtr));
  Config::SetDefault ("ns3::LispEtrItrApplication::XtrToMapServerDelayVariable", PointerValue (xtrToXtr));
  Config::SetDefault ("ns3::LispEtrItrApplication::XtrToXtrDelayVariable", PointerValue (xtrToXtr));

  Config::SetDefault ("ns3::OnOffApplication::DataRate", DataRateValue (DataRate ("160b/s")));
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (10));
  Config::SetDefault ("ns3::PfifoFastQueueDisc::Limit", UintegerValue (10000));

  Config::SetDefault ("ns3::LispEtrItrApplication::EnableProxyMode", BooleanValue (false));
  Config::SetDefault ("ns3::LispEtrItrApplication::EnableSubscribe", BooleanValue (false));

  Config::SetDefault ("ns3::SimpleMapTables::DefaultTTL", UintegerValue (m_defaultMappingTTL));
  m_output << "Default TTL: " << m_defaultMappingTTL << endl;
  m_output << endl;
}

void LispExperiment::InitTopology ()
{
  Ptr<LispTopology> topology = Create<LispTopology>();

  topology->NewNode ("router");
  for (int i = 0; i < this->m_nReceiver; i++)
    {
      string is = std::to_string (i);
      topology->NewNode ("host0_" + is);
      topology->NewNode ("xTR0a_" + is)->ConnectTo (topology->GetNode ("router"), IS_RLOC);
      topology->NewNode ("xTR0b_" + is)->ConnectTo (topology->GetNode ("router"), IS_RLOC);
    }

  for (int i = 0; i < this->m_nSender; i++)
    {
      string is = std::to_string (i);
      topology->GetNode ("router")->ConnectTo (topology->NewNode ("xTR1_" + is), IS_RLOC);
      topology->GetNode ("xTR1_" + is)->ConnectTo (topology->NewNode ("host1_" + is));
    }

  topology->GetNode ("router")->ConnectTo (topology->NewNode ("map_resolver"), IS_RLOC);
  topology->GetNode ("router")->ConnectTo (topology->NewNode ("map_server"), IS_RLOC);

  topology->PopulateRoutingTables (topology->GetNode ("router"));

  this->m_topology = topology;
}

void LispExperiment::InstallLisp ()
{
  Ptr<LispTopology> topology = this->m_topology;
  topology->GetNode ("map_server")->SetMapServer (Seconds (0.0), this->m_endTime);
  topology->GetNode ("map_resolver")->SetMapResolver (Seconds (0.0), this->m_endTime);
  for (int i = 0; i < this->m_nReceiver; i++)
    {
      string is = std::to_string (i);
      topology->GetNode ("xTR0a_" + is)->SetXtr (Seconds (1.0), this->m_endTime);
      topology->GetNode ("xTR0b_" + is)->SetXtr (Seconds (1.0), this->m_endTime);
    }

  Callback<void, Ptr<const Packet> > packetCallback = Callback<void, Ptr<const Packet> > (this, &LispExperiment::LogMappingUpdate);
  for (int i = 0; i < this->m_nSender; i++)
    {
      string is = std::to_string (i);
      Ptr<LispTopologyNode> sender = topology->GetNode ("xTR1_" + is);
      sender->SetXtr (Seconds (1.0), this->m_endTime);
      for (uint32_t j = 0; j < sender->GetNode ()->GetNApplications (); j++)
        {
          Ptr<Application> app = sender->GetNode ()->GetApplication (j);
          auto xtr = DynamicCast<LispEtrItrApplication, Application> (app);
          if (xtr != nullptr)
            {
              xtr->TraceConnectWithoutContext ("MappingUpdateTrace", packetCallback);
            }
        }
    }
}

void LispExperiment::InitCsmaGroups ()
{
  Ptr<LispTopology> topology = this->m_topology;
  for (int i = 0; i < this->m_nReceiver; i++)
    {
      string is = std::to_string (i);
      vector<Ptr<Ipv4TopologyNode> > csmaNodes;
      csmaNodes.push_back (StaticCast<Ipv4TopologyNode, LispTopologyNode> (topology->GetNode ("host0_" + is)));
      csmaNodes.push_back (StaticCast<Ipv4TopologyNode, LispTopologyNode> (topology->GetNode ("xTR0a_" + is)));
      csmaNodes.push_back (StaticCast<Ipv4TopologyNode, LispTopologyNode> (topology->GetNode ("xTR0b_" + is)));
      topology->NewCsmaGroup ("host0net_" + is, csmaNodes);
      topology->GetCsmaGroup ("host0net_" + is)->defaultRoute (topology->GetNode ("xTR0a_" + is));
      topology->GetCsmaGroup ("host0net_" + is)->disconnect (topology->GetNode ("xTR0b_" + is));

      topology->GetNode ("xTR0a_" + is)->AddEntryToMapTables (topology->GetNode ("host0_" + is)->GetAddress ());

      std::ostringstream oss;
      oss << topology->GetNode ("host0_" + is)->GetAddress ();
      string index = oss.str ();
      index = index.substr (0, index.size () - 2); // Truncate the .1 at the end of the address
      this->m_updatedSender.insert ({index, vector<uint32_t>()});
    }
}

void LispExperiment::InstallApplications ()
{
  for (int i = 0; i < this->m_nReceiver; i++)
    {
      string is = std::to_string (i);
      InstallUdpReceiver (this->m_topology->GetNode ("host0_" + is), Seconds (1.0), this->m_endTime);
      for (int j = 0; j < this->m_nSender; j++)
        {
          string js = std::to_string (j);
          InstallUdpSender (this->m_topology->GetNode ("host1_" + js), this->m_topology->GetNode ("host0_" + is),
                            Seconds (2.0), this->m_endTime);
        }
    }
}

void LispExperiment::MobilityHandler (Ptr<LispTopology> topology, ofstream *log, int nReceiver)
{
  for (int i = 0; i < nReceiver; i++)
    {
      string is = std::to_string (i);
      topology->GetCsmaGroup ("host0net_" + is)->connect (topology->GetNode ("xTR0b_" + is));
      topology->GetCsmaGroup ("host0net_" + is)->defaultRoute (topology->GetNode ("xTR0b_" + is));
      topology->GetCsmaGroup ("host0net_" + is)->disconnect (topology->GetNode ("xTR0a_" + is));

      Ptr<LispTopologyNode> xTR0b = topology->GetNode ("xTR0b_" + is);
      xTR0b->AddEntryToMapTables (topology->GetNode ("host0_" + is)->GetAddress ());

      Ptr<Node> xTR0b_node = xTR0b->GetNode ();
      topology->GetLispHelper ()->InstallMapTables (xTR0b_node);
      topology->GetLispHelper ()->SetMapTablesForEtr (xTR0b->GetRloc (), xTR0b->GetIpv4MapTables (), xTR0b->GetIpv6MapTables ());

      for (uint32_t j = 0; j < xTR0b_node->GetNApplications (); j++)
        {
          Ptr<Application> app = xTR0b_node->GetApplication (j);
          auto xtr = DynamicCast<LispEtrItrApplication, Application> (app);
          if (xtr != nullptr)
            {
              xtr->SendMapRegisters ();
            }
        }
    }

  LispExperiment::LogEvent (log, "Mobility event");
}

void LispExperiment::LogEvent (ofstream *output, string event)
{
  *output << Simulator::Now () << " ";
  *output << event << std::endl;
}

void LispExperiment::LogMappingUpdate (Ptr<const Packet> packetIn)
{

  Ptr<Packet> packet = Create<Packet> (*packetIn);

  MappingSocketMsgHeader sockMsgHdr;
  packet->RemoveHeader (sockMsgHdr);
  uint8_t buf[packet->GetSize ()];
  packet->CopyData (buf, packet->GetSize ());
  Ptr<MappingSocketMsg> msg = MappingSocketMsg::Deserialize (buf);
  if (sockMsgHdr.GetMapType () == static_cast<uint16_t> (LispMappingSocket::MAPM_ADD))
    {
      std::ostringstream oss;
      oss << "Mapping update : ";
      Ptr<EndpointId> eid = msg->GetEndPointId ();
      if ((int) sockMsgHdr.GetMapFlags () & (int) LispMappingSocket::MAPF_NEGATIVE)
        {
          oss << Ipv4Address::ConvertFrom (eid->GetEidAddress ()) << " -> None";
        }
      else
        {
          Ptr<Locator> locator = msg->GetLocators ()->GetLocatorByIdx (0);
          oss << Ipv4Address::ConvertFrom (eid->GetEidAddress ())
              << " -> " << Ipv4Address::ConvertFrom (locator->GetRlocAddress ());
        }
      oss << ", Node " << Simulator::GetContext ();

      if (Simulator::Now () > this->m_mobilityTime + Seconds (0.01))
        {
          std::ostringstream addressStream;
          addressStream << Ipv4Address::ConvertFrom (eid->GetEidAddress ());
          string index = addressStream.str ();
          index = index.substr (0, index.size () - 2); // Truncate the .0 at the end of the address
          auto found = this->m_updatedSender.find (index);
          if (found != this->m_updatedSender.end ())
            {
              vector<uint32_t> *senderVector = &(found->second);
              senderVector->push_back (Simulator::GetContext ());
              sort (senderVector->begin (), senderVector->end ());
              auto end = unique (senderVector->begin (), senderVector->end ());
              senderVector->erase (end, senderVector->end ());
              oss << " (valid)";
            }

          map<string, vector<uint32_t> >::iterator iter;
          for (iter = m_updatedSender.begin (); iter != m_updatedSender.end (); iter++)
            {
              vector<uint32_t> *senderVector = &(iter->second);
              if (senderVector->size () != unsigned(m_nSender))
                {
                  break;
                }
            }
          if (iter == m_updatedSender.end ())
            {
              Simulator::Stop ();
            }
        }
      LispExperiment::LogEvent (&m_output, oss.str ());
    }

}

