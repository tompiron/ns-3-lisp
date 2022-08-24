#ifndef IPV4_TOPOLOGY_GUARD
#define IPV4_TOPOLOGY_GUARD

#include <string>
#include <vector>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include "Topology.h"

class Ipv4CsmaGroup;
class Ipv4TopologyNode;

class Ipv4Topology : public Topology
{
public:
  Ipv4Topology ();
  void SetupIPAddressBase (const char* network, const char* mask, const char* base = "0.0.0.1");
  Ptr<Ipv4TopologyNode> NewNode (std::string name);
  Ptr<Ipv4TopologyNode> GetNode (std::string name);
  Ptr<Ipv4CsmaGroup> NewCsmaGroup (std::string name, std::vector<Ptr<Ipv4TopologyNode> > nodes);
  Ptr<Ipv4CsmaGroup> GetCsmaGroup (std::string name);
  void PopulateRoutingTables (Ptr<Ipv4TopologyNode> router);
  InternetStackHelper* GetInternetHelper ()
  {
    return &internetStackHelper;
  }
  Ipv4AddressHelper* GetIpv4AddressHelper ()
  {
    return &ipv4AddressHelper;
  }
  Ptr<Ipv4TopologyNode> GetGateway ()
  {
    return gateway;
  }

protected:
  InternetStackHelper internetStackHelper;
  Ipv4AddressHelper ipv4AddressHelper;
  Ptr<Ipv4TopologyNode> gateway;
};

class Ipv4CsmaGroup : public CsmaGroup
{
public:
  Ipv4CsmaGroup (std::vector<Ptr<Ipv4TopologyNode> > _nodes, std::string _name, Ipv4Topology * _topology);
  virtual ~Ipv4CsmaGroup ()
  {
  }
  void disconnect (Ptr<Ipv4TopologyNode> node);
  void connect (Ptr<Ipv4TopologyNode> node);
  void defaultRoute (Ptr<Ipv4TopologyNode> node);

protected:
  Ipv4Topology* ipv4Topology;
  std::map<std::string, std::pair<Ptr<Ipv4>, uint32_t> > interfacesByName;
};

class Ipv4TopologyNode : public TopologyNode
{
public:
  Ipv4TopologyNode (std::string _name, Ipv4Topology * _topology);
  virtual ~Ipv4TopologyNode ()
  {
  }
  virtual void ConnectTo (Ptr<TopologyNode> nodeTo, const char* network = "");
  Ipv4Address GetAddress (unsigned int interfaceIndex = 0);
  Ipv4InterfaceContainer GetInterfaces ();

  friend Ipv4CsmaGroup;

protected:
  Ipv4Topology* ipv4Topology;
  Ipv4InterfaceContainer interfaces;
};

#endif
