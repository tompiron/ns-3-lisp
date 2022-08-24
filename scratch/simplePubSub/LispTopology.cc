#include "LispTopology.h"

using std::string;
using std::vector;

NS_LOG_COMPONENT_DEFINE ("LispTopology");

void LispTopologyNode::SetLispCapable ()
{
  this->lispTopology->GetLispHelper ()->Install (this->node);
  this->lispTopology->GetLispHelper ()->InstallMapTables (this->node);
}

void LispTopologyNode::SetMapTables ()
{
  for (auto iter = this->interfaces.Begin (); iter < this->interfaces.End (); iter++)
    {
      Ipv4Address eidAddress = iter->first->GetAddress (iter->second, 0).GetLocal ();
      this->AddEntryToMapTables (eidAddress);
    }

  this->lispTopology->GetLispHelper ()->SetMapTablesForEtr (this->rloc, this->ipv4MapTables, this->ipv6MapTables);
}

void LispTopologyNode::InitializeMapTables ()
{
  if (this->ipv4MapTables == nullptr)
    {
      this->ipv4MapTables = CreateObject<SimpleMapTables> ();
    }
  if (this->ipv6MapTables == nullptr)
    {
      this->ipv6MapTables = CreateObject<SimpleMapTables> ();
    }
}

LispTopologyNode::LispTopologyNode (string _name, LispTopology* _topology) : Ipv4TopologyNode (_name, _topology)
{
  this->lispTopology = _topology;
}

void LispTopologyNode::ConnectTo (Ptr<TopologyNode> nodeTo)
{
  this->ConnectTo (nodeTo, IS_EID);
}

void LispTopologyNode::ConnectTo (Ptr<TopologyNode> nodeTo, bool is_rlocs)
{
  this->Ipv4TopologyNode::ConnectTo (nodeTo);
  Ptr<LispTopologyNode> lispNodeTo = DynamicCast<LispTopologyNode, TopologyNode> (nodeTo);
  if (lispNodeTo == nullptr)
    {
      return;
    }
  if (is_rlocs)
    {
      Ipv4Address thisAddress = this->GetAddress (this->interfaces.GetN () - 1);
      Ipv4Address nodeToAddress = lispNodeTo->GetAddress (lispNodeTo->interfaces.GetN () - 1);
      this->lispTopology->GetLispHelper ()->AddRlocToSet (thisAddress);
      this->rloc = thisAddress;
      this->lispTopology->GetLispHelper ()->AddRlocToSet (nodeToAddress);
      lispNodeTo->rloc = nodeToAddress;
    }
}

void LispTopologyNode::SetMapServer (Time start, Time stop, unsigned int interfaceIndex)
{
  this->SetMapTables ();
  this->SetLispCapable ();

  Address address = this->GetAddress (interfaceIndex);
  this->lispTopology->GetLispXtrHelper ()->AddMapServerAddress (address);
  this->lispTopology->GetMapResolverHelper ()->SetMapServerAddress (address);

  this->application = this->lispTopology->GetMapServerHelper ()->Install (this->node);
  this->application.Start (start);
  this->application.Stop (stop);
}

void LispTopologyNode::SetMapResolver (Time start, Time stop, unsigned int interfaceIndex)
{
  this->SetLispCapable ();

  Address address = this->GetAddress (interfaceIndex);
  Ptr<Locator> rloc = Create<Locator> (address);
  this->lispTopology->GetLispXtrHelper ()->AddMapResolverRlocs (rloc);

  this->application = this->lispTopology->GetMapResolverHelper ()->Install (this->node);
  this->application.Start (start);
  this->application.Stop (stop);
}

void LispTopologyNode::SetXtr (Time start, Time stop)
{
  this->SetMapTables ();
  this->SetLispCapable ();
  this->application = this->lispTopology->GetLispXtrHelper ()->Install (this->node);
  this->application.Start (start);
  this->application.Stop (stop);
}

void LispTopologyNode::AddEntryToMapTables (Ipv4Address eidAddress)
{
  this->InitializeMapTables ();

  Ipv4Mask eidMask = Ipv4Mask ("255.255.255.0");
  auto location = MapTables::IN_DATABASE;

  Ipv4Address rlocAddress = Ipv4Address::ConvertFrom (this->rloc);
  uint8_t priority = 200;
  uint8_t weight = 30;
  bool reachability = true;
  if (Ipv4Address::IsMatchingType (eidAddress))
    {
      ipv4MapTables->InsertLocator (eidAddress, eidMask, rlocAddress, priority, weight,
                                    location, reachability);
    }
  else if (Ipv6Address::IsMatchingType (eidAddress))
    {
      ipv6MapTables->InsertLocator (eidAddress, eidMask, rlocAddress, priority, weight,
                                    location, reachability);
    }
}

Ptr<LispTopologyNode> LispTopology::NewNode (std::string name)
{
  Ptr<LispTopologyNode> node = Create<LispTopologyNode> (name, this);
  this->AddNode (node);
  return node;
}

Ptr<LispTopologyNode> LispTopology::GetNode (std::string name)
{
  Ptr<TopologyNode> node = this->Topology::GetNode (name);
  return DynamicCast<LispTopologyNode, TopologyNode> (node);
}
