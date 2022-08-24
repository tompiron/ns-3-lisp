#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "LispTopology.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("SimplePubSub");

/* Topology:  map_resolver    map_server
 *                      \      /
 *                       \    /
 *           xTR0 <----> router <-----> xTR1
 *            /        (non-LISP)           \
 *           /                               \
 *         host0                            host1
 *      (non-LISP)                        (non-LISP)
 */

struct Main
{
  int main (int argumentCount, char const *argv[])
  {
    LogComponentEnable ("SimplePubSub", LOG_LEVEL_INFO);
    PacketMetadata::Enable ();
    Config::SetDefault ("ns3::LispEtrItrApplication::EnableProxyMode", BooleanValue (true));

    Ptr<LispTopology> topology = Create<LispTopology>();
    topology->NewNode ("router");
    topology->NewNode ("xTR0a")->ConnectTo (topology->GetNode ("router"), IS_RLOC);
    topology->NewNode ("xTR0b")->ConnectTo (topology->GetNode ("router"), IS_RLOC);
    topology->GetNode ("router")->ConnectTo (topology->NewNode ("xTR1"), IS_RLOC);

    topology->GetNode ("router")->ConnectTo (topology->NewNode ("map_resolver"), IS_RLOC);
    topology->GetNode ("router")->ConnectTo (topology->NewNode ("map_server"), IS_RLOC);

    topology->PopulateRoutingTables (topology->GetNode ("router"));
    topology->GetNode ("xTR0a")->AddEntryToMapTables (Ipv4Address ("192.168.0.1"));
    topology->GetNode ("xTR0b")->AddEntryToMapTables (Ipv4Address ("192.168.0.1"));

    topology->GetNode ("map_server")->SetMapServer (Seconds (0.0), Seconds (20.0));
    topology->GetNode ("map_resolver")->SetMapResolver (Seconds (0.0), Seconds (20.0));
    topology->GetNode ("xTR0a")->SetXtr (Seconds (1.0), Seconds (10.0));
    topology->GetNode ("xTR0b")->SetXtr (Seconds (10.0), Seconds (20.0));
    topology->GetNode ("xTR1")->SetXtr (Seconds (1.0), Seconds (20.0));

    topology->EnablePcapFiles ("simple_pubsub");

    Simulator::Run ();
    Simulator::Destroy ();

    topology = 0;
    return 0;
  }
};

int main (int argc, char const *argv[])
{
  Main main;
  main.main (argc, argv);
}
