#include "Ipv4Topology.h"

using std::string;
using std::vector;

NS_LOG_COMPONENT_DEFINE ("Ipv4Topology");

Ipv4TopologyNode::Ipv4TopologyNode (string _name, Ipv4Topology * _topology) : TopologyNode (_name, _topology)
{
  this->ipv4Topology = _topology;
  _topology->GetInternetHelper ()->Install (this->node);
}

Ipv4InterfaceContainer assignIPs (Ipv4AddressHelper* helper, NetDeviceContainer devices, bool incrementNetwork = true)
{
  Ipv4InterfaceContainer interfaces = helper->Assign (devices);
  if (incrementNetwork)
    {
      helper->NewNetwork ();
    }
  return interfaces;
}

void Ipv4TopologyNode::ConnectTo (Ptr<TopologyNode> nodeTo, const char* network)
{
  this->TopologyNode::ConnectTo (nodeTo);
  Ptr<Ipv4TopologyNode> ipv4NodeTo = DynamicCast<Ipv4TopologyNode, TopologyNode> (nodeTo);
  if (ipv4NodeTo == nullptr)
    {
      return;
    }

  NetDeviceContainer toInstall;
  toInstall.Add (this->netDevices.Get (this->netDevices.GetN () - 1));
  toInstall.Add (ipv4NodeTo->netDevices.Get (ipv4NodeTo->netDevices.GetN () - 1));

  Ipv4InterfaceContainer newInterfaces;
  if (string (network) == string (""))
    {
      newInterfaces = assignIPs (this->ipv4Topology->GetIpv4AddressHelper (), toInstall);
    }
  else
    {
      Ipv4AddressHelper ipv4AddressHelper;
      ipv4AddressHelper.SetBase (network, "255.255.255.0", "0.0.0.1");
      newInterfaces = assignIPs (&ipv4AddressHelper, toInstall);
    }

  this->interfaces.Add (newInterfaces.Get (0));
  ipv4NodeTo->interfaces.Add (newInterfaces.Get (1));
}

Ipv4Address Ipv4TopologyNode::GetAddress (unsigned int interfaceIndex)
{
  return this->interfaces.GetAddress (interfaceIndex);
}

Ipv4InterfaceContainer Ipv4TopologyNode::GetInterfaces ()
{
  Ipv4InterfaceContainer result = Ipv4InterfaceContainer (this->interfaces);
  return result;
}

vector<Ptr<TopologyNode> > castVector (vector<Ptr<Ipv4TopologyNode> > vec)
{
  vector<Ptr<TopologyNode> > result;
  for (auto iter = vec.cbegin (); iter != vec.cend (); iter++)
    {
      result.push_back (DynamicCast<Ipv4TopologyNode, TopologyNode> (*iter));
    }
  return result;
}

Ipv4CsmaGroup::Ipv4CsmaGroup (vector<Ptr<Ipv4TopologyNode> > _nodes, string _name, Ipv4Topology * _topology) : CsmaGroup (castVector (_nodes), _name, _topology)
{
  this->ipv4Topology = _topology;

  Ipv4StaticRoutingHelper routingHelper;
  Ipv4InterfaceContainer interfaces = assignIPs (this->ipv4Topology->GetIpv4AddressHelper (), this->netDevices);
  for (uint32_t i = 0; i < _nodes.size (); i++)
    {
      auto interfacePair = interfaces.Get (i);
      Ptr<Ipv4TopologyNode> node = _nodes[i];
      node->interfaces.Add (interfacePair);
      this->interfacesByName.insert ({node->GetName (), interfacePair});

      Ptr<Ipv4> ipv4 = interfacePair.first;
      uint32_t interface = interfacePair.second;
      Ipv4Address address = ipv4->GetAddress (interface, 0).GetLocal ();
      Ptr<Ipv4StaticRouting> routing = routingHelper.GetStaticRouting (ipv4);
      routing->AddNetworkRouteTo (address, Ipv4Mask ("255.255.255.0"), interface);
    }
}

void Ipv4CsmaGroup::disconnect (Ptr<Ipv4TopologyNode> node)
{
  auto iter = this->interfacesByName.find (node->GetName ());
  if (iter != this->interfacesByName.cend ())
    {
      auto interfacePair = iter->second;
      interfacePair.first->SetDown (interfacePair.second);
    }
}

void Ipv4CsmaGroup::connect (Ptr<Ipv4TopologyNode> node)
{
  auto iter = this->interfacesByName.find (node->GetName ());
  if (iter != this->interfacesByName.cend ())
    {
      auto interfacePair = iter->second;
      interfacePair.first->SetUp (interfacePair.second);
    }
}

void Ipv4CsmaGroup::defaultRoute (Ptr<Ipv4TopologyNode> node)
{
  auto search = this->interfacesByName.find (node->GetName ());
  if (search == this->interfacesByName.cend ())
    {
      NS_LOG_WARN ("Tried to set default route to non-connected node on csma group '" << this->name << "'.");
      return;
    }
  Ipv4Address address = search->second.first->GetAddress (search->second.second, 0).GetLocal ();

  Ipv4StaticRoutingHelper routingHelper;
  for (auto iter = this->interfacesByName.cbegin (); iter != this->interfacesByName.cend (); iter++)
    {
      Ptr<Ipv4> ipv4 = iter->second.first;
      uint32_t interface = iter->second.second;
      Ptr<Ipv4StaticRouting> routing = routingHelper.GetStaticRouting (ipv4);
      if (iter->first != node->GetName ())
        {
          routing->SetDefaultRoute (address, interface);
        }
      else
        {
          routing->SetDefaultRoute (this->ipv4Topology->GetGateway ()->GetAddress (), 1);
        }
    }
}

Ipv4Topology::Ipv4Topology () : Topology ()
{
  this->SetupIPAddressBase ("10.0.0.0", "255.255.255.0");
}

void Ipv4Topology::SetupIPAddressBase (const char* network, const char* mask, const char* base)
{
  this->ipv4AddressHelper.SetBase (network, mask, base);
}

Ptr<Ipv4TopologyNode> Ipv4Topology::NewNode (string name)
{
  Ptr<Ipv4TopologyNode> node = Create<Ipv4TopologyNode> (name, this);
  this->AddNode (node);
  return node;
}

Ptr<Ipv4TopologyNode> Ipv4Topology::GetNode (string name)
{
  Ptr<TopologyNode> node = this->Topology::GetNode (name);
  return DynamicCast<Ipv4TopologyNode, TopologyNode> (node);
}

Ptr<Ipv4CsmaGroup> Ipv4Topology::NewCsmaGroup (string name, vector<Ptr<Ipv4TopologyNode> > nodes)
{
  Ptr<Ipv4CsmaGroup> group = Create<Ipv4CsmaGroup> (nodes, name, this);
  this->AddCsmaGroup (group);
  return group;
}

Ptr<Ipv4CsmaGroup> Ipv4Topology::GetCsmaGroup (string name)
{
  Ptr<CsmaGroup> group = this->Topology::GetCsmaGroup (name);
  return DynamicCast<Ipv4CsmaGroup, CsmaGroup> (group);
}

void Ipv4Topology::PopulateRoutingTables (Ptr<Ipv4TopologyNode> router)
{
  this->gateway = router;
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ipv4StaticRoutingHelper routingHelper;
  for (auto iter = this->nodesByName.begin (); iter != this->nodesByName.end (); iter++)
    {
      Ptr<Ipv4TopologyNode> node = DynamicCast<Ipv4TopologyNode, TopologyNode> (iter->second);
      if (node != nullptr && node->GetName () != this->gateway->GetName ())
        {
          Ipv4InterfaceContainer interfaces = node->GetInterfaces ();
          if (interfaces.GetN () > 0)
            {
              auto interfacePair = interfaces.Get (0);
              Ptr<Ipv4> ipv4 = interfacePair.first;
              uint32_t interface = interfacePair.second;
              Ptr<Ipv4StaticRouting> routing = routingHelper.GetStaticRouting (ipv4);
              routing->SetDefaultRoute (this->gateway->GetAddress (), interface);
            }
        }
    }
}
