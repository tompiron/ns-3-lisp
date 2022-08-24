#ifndef LISP_TOPOLOGY_GUARD
#define LISP_TOPOLOGY_GUARD

#include <string>
#include <vector>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include "Ipv4Topology.h"

#define IS_RLOC true
#define IS_EID false

class LispTopologyNode;

class LispTopology : public Ipv4Topology
{
public:
  LispTopology () : Ipv4Topology ()
  {
  }

  virtual ~LispTopology ()
  {
  }

  Ptr<LispTopologyNode> NewNode (std::string name);
  Ptr<LispTopologyNode> GetNode (std::string name);

  LispHelper* GetLispHelper ()
  {
    return &lispHelper;
  }

  LispEtrItrAppHelper* GetLispXtrHelper ()
  {
    return &lispXtrHelper;
  }

  MapServerDdtHelper* GetMapServerHelper ()
  {
    return &mapServerHelper;
  }

  MapResolverDdtHelper* GetMapResolverHelper ()
  {
    return &mapResolverHelper;
  }

protected:
  LispHelper lispHelper;
  LispEtrItrAppHelper lispXtrHelper;
  MapServerDdtHelper mapServerHelper;
  MapResolverDdtHelper mapResolverHelper;
};

class LispTopologyNode : public Ipv4TopologyNode
{
protected:
  void SetLispCapable ();
  void SetMapTables ();
  void InitializeMapTables ();

public:
  LispTopologyNode (std::string _name, LispTopology* _topology);
  virtual ~LispTopologyNode ()
  {
  }
  virtual void ConnectTo (Ptr<TopologyNode> nodeTo);
  void ConnectTo (Ptr<TopologyNode> nodeTo, bool is_rlocs);
  void SetMapServer (Time start, Time stop, unsigned int interfaceIndex = 0);
  void SetMapResolver (Time start, Time stop, unsigned int interfaceIndex = 0);
  void SetXtr (Time start, Time stop);
  void AddEntryToMapTables (Ipv4Address eidAddress);

  Address GetRloc ()
  {
    return rloc;
  }

  Ptr<SimpleMapTables> GetIpv4MapTables ()
  {
    return ipv4MapTables;
  }

  Ptr<SimpleMapTables> GetIpv6MapTables ()
  {
    return ipv6MapTables;
  }

protected:
  LispTopology* lispTopology;
  ApplicationContainer application;
  Address rloc;
  Ptr<SimpleMapTables> ipv4MapTables;
  Ptr<SimpleMapTables> ipv6MapTables;
};

#endif
