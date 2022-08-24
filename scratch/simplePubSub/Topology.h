#ifndef TOPOLOGY_GUARD
#define TOPOLOGY_GUARD

#include <string>
#include <vector>
#include <map>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

class TopologyNode;
class CsmaGroup;

class Topology : public SimpleRefCount<Topology>
{
public:
  Topology ();
  virtual ~Topology ()
  {
  }
  Ptr<TopologyNode> NewNode (std::string name);
  Ptr<TopologyNode> GetNode (std::string name);
  Ptr<CsmaGroup> NewCsmaGroup (std::string name, std::vector<Ptr<TopologyNode> > nodes);
  Ptr<CsmaGroup> GetCsmaGroup (std::string name);
  void EnablePcapFiles (std::string pcapFilesPrefix);
  PointToPointHelper* GetLinkHelper ()
  {
    return &linkHelper;
  }
  CsmaHelper* GetCsmaHelper ()
  {
    return &csmaHelper;
  }

protected:
  void AddNode (Ptr<TopologyNode> node);
  void AddCsmaGroup (Ptr<CsmaGroup> group);

protected:
  std::map<std::string, Ptr<TopologyNode> > nodesByName;
  std::map<std::string, Ptr<CsmaGroup> > csmaGroupByName;
  PointToPointHelper linkHelper;
  CsmaHelper csmaHelper;
};

class CsmaGroup : public SimpleRefCount<CsmaGroup>
{
public:
  CsmaGroup (std::vector<Ptr<TopologyNode> > _nodes, std::string _name, Topology* _topology);

  virtual ~CsmaGroup ()
  {
  }

  virtual void disconnect (Ptr<TopologyNode> node);
  virtual void connect (Ptr<TopologyNode> node);
  virtual void defaultRoute (Ptr<TopologyNode> node);
  std::string GetName ()
  {
    return name;
  }

protected:
  Topology* topology;
  NetDeviceContainer netDevices;
  std::string name;
};

class TopologyNode : public SimpleRefCount<TopologyNode>
{
public:
  TopologyNode (std::string _name, Topology* _topology);

  virtual ~TopologyNode ()
  {
  }

  virtual void ConnectTo (Ptr<TopologyNode> nodeTo);

  Ptr<Node> GetNode ()
  {
    return node;
  }

  std::string GetName ()
  {
    return name;
  }

  NetDeviceContainer GetNetDevices ()
  {
    return NetDeviceContainer (this->netDevices);
  }

  friend CsmaGroup;

protected:
  Topology* topology;
  Ptr<Node> node;
  std::string name;
  NetDeviceContainer netDevices;
};

#endif
