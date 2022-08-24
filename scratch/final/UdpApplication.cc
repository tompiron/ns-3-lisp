#include "UdpApplication.h"

#include "ns3/applications-module.h"

#include "Ipv4Topology.h"

#include <string>

using std::string;

NS_LOG_COMPONENT_DEFINE ("UdpApplication");

void InstallUdpApplication (Ptr<Ipv4TopologyNode> source, Ptr<Ipv4TopologyNode> destination,
                            Time start, Time end, uint16_t port)
{
  InstallUdpReceiver (destination, start, end, port);
  InstallUdpSender (source, destination, start + Seconds (1.0), end, port);
}

void InstallUdpReceiver (Ptr<Ipv4TopologyNode> node, Time start, Time end, uint16_t port)
{
  InetSocketAddress udp_address = InetSocketAddress (node->GetAddress (), port);
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", udp_address);
  ApplicationContainer sinkApp = sinkHelper.Install (node->GetNode ());
  sinkApp.Start (start);
  sinkApp.Stop (end);
}

void InstallUdpSender (Ptr<Ipv4TopologyNode> node, Ptr<Ipv4TopologyNode> destination, Time start, Time end, uint16_t port)
{
  InetSocketAddress udp_dest_address = InetSocketAddress (destination->GetAddress (), port);
  OnOffHelper sender_helper ("ns3::UdpSocketFactory", udp_dest_address);
  sender_helper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  sender_helper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer sender_app = sender_helper.Install (node->GetNode ());
  Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
  var->SetAttribute ("Min", DoubleValue (0));
  var->SetAttribute ("Max", DoubleValue (0.01));
  sender_app.Start (start + Seconds (var->GetValue ()));
  sender_app.Stop (end);
}
