#include "Topology.h"

using std::string;
using std::vector;
using std::logic_error;

NS_LOG_COMPONENT_DEFINE ("Topology");

Topology::Topology ()
{
  this->linkHelper.SetDeviceAttribute ("DataRate", StringValue ("5Gbps"));
  this->linkHelper.SetChannelAttribute ("Delay", StringValue ("0ms"));
  this->csmaHelper.SetChannelAttribute ("DataRate", StringValue ("5Gbps"));
  this->csmaHelper.SetChannelAttribute ("Delay", StringValue ("0ms"));
}

Ptr<TopologyNode> Topology::NewNode (string name)
{
  Ptr<TopologyNode> node = Create<TopologyNode> (name, this);
  this->AddNode (node);
  return node;
}

Ptr<TopologyNode> Topology::GetNode (string name)
{
  auto iterator = this->nodesByName.find (name);
  if (iterator == this->nodesByName.cend ())
    {
      throw logic_error ("Unknown node.");
    }
  return (*iterator).second;
}

Ptr<CsmaGroup> Topology::NewCsmaGroup (string name, vector<Ptr<TopologyNode> > nodes)
{
  Ptr<CsmaGroup> group = Create<CsmaGroup> (nodes, name, this);
  this->AddCsmaGroup (group);
  return group;
}

Ptr<CsmaGroup> Topology::GetCsmaGroup (string name)
{
  auto iterator = this->csmaGroupByName.find (name);
  if (iterator == this->csmaGroupByName.cend ())
    {
      throw logic_error ("Unknown node.");
    }
  return (*iterator).second;
}

void Topology::EnablePcapFiles (string pcapFilesPrefix)
{
  this->linkHelper.EnablePcapAll (pcapFilesPrefix);
  this->csmaHelper.EnablePcapAll (pcapFilesPrefix + "-csma");
}

void Topology::AddNode (Ptr<TopologyNode> node)
{
  this->nodesByName.insert ({node->GetName (), node});
  return;
}

void Topology::AddCsmaGroup (Ptr<CsmaGroup> group)
{
  this->csmaGroupByName.insert ({group->GetName (), group});
  return;
}

CsmaGroup::CsmaGroup (vector<Ptr<TopologyNode> > _nodes, string _name, Topology* _topology)
{
  this->topology = _topology;
  this->name = _name;

  NodeContainer container;
  for (auto iter = _nodes.cbegin (); iter != _nodes.cend (); iter++)
    {
      container.Add ((*iter)->GetNode ());
    }

  NetDeviceContainer devices = this->topology->GetCsmaHelper ()->Install (container);
  for (uint32_t i = 0; i < _nodes.size (); i++)
    {
      Ptr<NetDevice> device = devices.Get (i);
      _nodes[i]->netDevices.Add (device);
      this->netDevices.Add (device);
    }
}

void CsmaGroup::disconnect (Ptr<TopologyNode> node)
{
}

void CsmaGroup::connect (Ptr<TopologyNode> node)
{
}

void CsmaGroup::defaultRoute (Ptr<TopologyNode> node)
{
}

TopologyNode::TopologyNode (string _name, Topology* _topology)
{
  this->node = CreateObject<Node>();
  this->topology = _topology;
  this->name = _name;
}

void TopologyNode::ConnectTo (Ptr<TopologyNode> nodeTo)
{
  NodeContainer linkNodes = NodeContainer (this->node, nodeTo->node);
  NetDeviceContainer networkDevices = topology->GetLinkHelper ()->Install (linkNodes);
  this->netDevices.Add (networkDevices.Get (0));
  nodeTo->netDevices.Add (networkDevices.Get (1));
}
