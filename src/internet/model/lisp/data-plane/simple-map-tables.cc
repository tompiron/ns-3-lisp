/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of Liege
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Lionel Agbodjan <lionel.agbodjan@gmail.com>
 */

#include "simple-map-tables.h"

#include "ns3/ptr.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-interface-address.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "lisp-header.h"
#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimpleMapTables");

NS_OBJECT_ENSURE_REGISTERED (SimpleMapTables);

TypeId
SimpleMapTables::GetTypeId ()
{
  static TypeId tid =
    TypeId ("ns3::SimpleMapTables").SetParent<MapTables> ().SetGroupName (
      "Lisp").AddConstructor<SimpleMapTables> ();
  return tid;
}

SimpleMapTables::SimpleMapTables ()
{
  NS_LOG_FUNCTION (this);
}

SimpleMapTables::~SimpleMapTables ()
{
  NS_LOG_FUNCTION (this);
}

int SimpleMapTables::GetNMapEntries (void)
{
  return m_mappingCache.size () + m_mappingDatabase.size ();
}

Ptr<MapEntry>
SimpleMapTables::DatabaseLookup (const Address &eidAddress)
{
  // TODO Turn eidAddress in prefix
  Ptr<EndpointId> eid = Create<EndpointId> (eidAddress);
  std::map<Ptr<EndpointId>, Ptr<MapEntry>, CompareEndpointId >::iterator it = m_mappingDatabase.find (eid);

  if (it != m_mappingDatabase.end ())
    {
      Ptr<MapEntry> entry = m_mappingDatabase.at (eid);
      MapTables::DbHit ();
      return entry;
    }
  MapTables::DbMiss ();
  return 0;
}

Ptr<MapEntry>
SimpleMapTables::CacheLookup (const Address &eidAddress)
{
  // TODO Turn eidAddress in prefix
  Ptr<EndpointId> eid = Create<EndpointId> (eidAddress);

  if (m_mappingCache.find (eid) != m_mappingCache.end ())
    {
      MapTables::CacheHit ();
      return m_mappingCache.at (eid);
    }
  MapTables::CacheMiss ();
  return 0;
  // TODO investigate lock and unlock
}

// Set an ipv4 eid
void
SimpleMapTables::SetEntry (const Address &eidAddress, const Ipv4Mask &mask, Ptr<MapEntry> mapEntry, MapEntryLocation location)
{
  NS_ASSERT (Ipv4Address::IsMatchingType (eidAddress));
  Ptr<EndpointId> eid = Create<EndpointId> ();

  eid->SetIpv4Mask (mask);
  eid->SetEidAddress (static_cast<Address> (Ipv4Address::ConvertFrom (eidAddress).CombineMask (mask)));

  mapEntry->SetEidPrefix (eid);

  if (location == IN_DATABASE)
    {
      m_mutexDatabase.Lock ();
      std::map<Ptr<EndpointId>, Ptr<MapEntry>, CompareEndpointId>::iterator it = m_mappingDatabase.find (eid);
      if (it != m_mappingDatabase.end ())
        {
          m_mappingDatabase.erase (it);
        }
      m_mappingDatabase.insert (std::pair<Ptr<EndpointId>, Ptr<MapEntry> > (eid, mapEntry));

      m_mutexDatabase.Unlock ();
    }

  else if (location == IN_CACHE)
    {
      m_mutexCache.Lock ();
      std::map<Ptr<EndpointId>, Ptr<MapEntry>, CompareEndpointId>::iterator it = m_mappingCache.find (eid);
      if (it != m_mappingCache.end ())
        {
          m_mappingCache.erase (it);
        }
      m_mappingCache.insert (std::pair<Ptr<EndpointId>, Ptr<MapEntry> > (eid, mapEntry));
      m_mutexCache.Unlock ();
    }
}

// Ipv6
void SimpleMapTables::SetEntry (const Address &eid, const Ipv6Prefix &prefix,
                                Ptr<MapEntry> mapEntry,
                                MapEntryLocation location)
{
  // TODO SetEntry: do the same for ipv6
}

// Insert Locator
void SimpleMapTables::InsertLocator (const Address &eid, const Ipv4Mask &mask,
                                     const Ipv6Prefix &prefix, const Address
                                     &rlocAddress, uint8_t priority, uint8_t
                                     weight, MapEntryLocation location, bool reachable)
{
  Ptr<RlocMetrics> rlocMetrics = Create<RlocMetrics> (priority, weight, reachable);
  Ptr<Locator> locator = Create<Locator> (rlocAddress);
  locator->SetRlocMetrics (rlocMetrics);
  if (location == IN_DATABASE)
    {
      Ptr<MapEntry> dbEntry = DatabaseLookup (eid);
      if (dbEntry)
        {
          dbEntry->InsertLocator (locator);
        }
      else
        {
          Ptr<MapEntry> mapEntry = Create<MapEntryImpl> ();
          mapEntry->InsertLocator (locator);
          if (Ipv4Address::IsMatchingType (eid))
            {
              SetEntry (eid, mask, mapEntry, IN_DATABASE);
            }
          else if (Ipv6Address::IsMatchingType (eid))
            {
              SetEntry (eid, prefix, mapEntry, IN_DATABASE);
            }
        }
    }
  else
    {
      Ptr<MapEntry> dbEntry = CacheLookup (eid);
      if (dbEntry)
        {
          dbEntry->InsertLocator (locator);
        }
      else
        {
          Ptr<MapEntry> mapEntry = Create<MapEntryImpl> ();
          mapEntry->InsertLocator (locator);
          if (Ipv4Address::IsMatchingType (eid))
            {
              SetEntry (eid, mask, mapEntry, IN_CACHE);
            }
          else if (Ipv6Address::IsMatchingType (eid))
            {
              SetEntry (eid, prefix, mapEntry, IN_CACHE);
            }
        }
    }
}

// wrapper for ipv4 eid and ipv4 rlocAddress
void SimpleMapTables::InsertLocator (const Ipv4Address &eid, const Ipv4Mask
                                     &mask, const Ipv4Address &rlocAddress,
                                     uint8_t priority, uint8_t weight,
                                     MapEntryLocation location, bool reachable)
{
  NS_LOG_FUNCTION (this << eid << mask << rlocAddress << uint32_t (priority) << uint32_t (weight) << location << reachable);
  InsertLocator (static_cast<Address> (eid), mask, Ipv6Prefix (), static_cast<Address> (rlocAddress), priority, weight, location, reachable);
}

void SimpleMapTables::InsertLocator (const Ipv4Address &eid, const Ipv4Mask
                                     &mask, const Ipv6Address &rlocAddress,
                                     uint8_t priority, uint8_t weight,
                                     MapEntryLocation location, bool reachable)
{
  NS_LOG_FUNCTION (this << eid << mask << rlocAddress << uint32_t (priority) << uint32_t (weight) << location);
  InsertLocator (static_cast<Address> (eid), mask, Ipv6Prefix (), static_cast<Address> (rlocAddress), priority, weight, location, reachable);
}

void SimpleMapTables::InsertLocator (const Ipv6Address &eid, const Ipv6Prefix
                                     &prefix, const Ipv4Address &rlocAddress,
                                     uint8_t priority, uint8_t weight,
                                     MapEntryLocation location, bool reachable)
{
  // TODO InsertLocator IPv6 eid IPv4 rloc
}

void SimpleMapTables::InsertLocator (const Ipv6Address &eid, const Ipv6Prefix
                                     &prefix, const Ipv6Address &rlocAddress,
                                     uint8_t priority, uint8_t weight,
                                     MapEntryLocation location, bool reachable)
{
  // TODO InsertLocator IPv6 eid IPv6 rloc
}


Ptr<Locator>
SimpleMapTables::DestinationRlocSelection (Ptr<const MapEntry> remoteMapEntry)
{
  return remoteMapEntry->RlocSelection ();
}

// srcEid est important pour recuperer la liste des locators (sources)
// associe a cet eid
Ptr<Locator>
SimpleMapTables::SourceRlocSelection (Address const &srcEid, Ptr<const Locator> destLocator)
{
  NS_LOG_DEBUG ("Selecting Source Locator for the source eid: " << Ipv4Address::ConvertFrom (srcEid) << " to dest locator " << Ipv4Address::ConvertFrom (destLocator->GetRlocAddress ()));
  NS_ASSERT (destLocator);

  bool isDestIpv4 = false;
  Ptr<Node> node = (MapTables::GetLispOverIp ())->GetNode ();
  Socket::SocketErrno errno_;
  Address srcAddress;
  Ptr<Locator> srcLocator;
  // if the destination is Ipv4
  if (Ipv4Address::IsMatchingType (destLocator->GetRlocAddress ()))
    {
      isDestIpv4 = true;
      Ptr<Ipv4Route> route = 0;
      Ptr<Ipv4RoutingProtocol> routingProtocol =
        node->GetObject<Ipv4> ()->GetRoutingProtocol ();

      if (routingProtocol != 0)
        {
          Ipv4Header header = Ipv4Header ();
          header.SetDestination (Ipv4Address::ConvertFrom (destLocator->GetRlocAddress ()));
          route = routingProtocol->RouteOutput (
            0,
            header,
            0, errno_);
        }
      else
        {
          NS_LOG_ERROR (
            "SimpleMapTables::SelectSrcRloc: routingProtocol == 0");
        }

      if (route)
        {
          // we know the route and we get the output interface
          int32_t interface = node->GetObject<Ipv4> ()->GetInterfaceForDevice (
            route->GetOutputDevice ());
          // we get the interface address
          Ipv4Address ipv4SrcAddress = (node->GetObject<Ipv4> ()->GetAddress (
                                          interface, 0)).GetLocal ();
          srcAddress = static_cast<Address> (ipv4SrcAddress);
        }
      else
        {
          NS_LOG_WARN ("No route to destination locator. Return.");
          return 0;
        }
    }
  else if (Ipv6Address::IsMatchingType (destLocator->GetRlocAddress ()))
    {
      Ptr<Ipv6Route> route = 0;
      Ptr<Ipv6RoutingProtocol> routingProtocol =
        node->GetObject<Ipv6> ()->GetRoutingProtocol ();
      if (routingProtocol)
        {
          Ipv6Header header = Ipv6Header ();
          header.SetDestinationAddress (Ipv6Address::ConvertFrom (destLocator->GetRlocAddress ()));
          route = routingProtocol->RouteOutput (0,
                                                header,
                                                0, errno_);
        }
      else
        {
          NS_LOG_ERROR (
            "SimpleMapTables::SelectSrcRloc: routingProtocol == 0");
        }

      if (route)
        {
          // we know the route and we get the output interface
          int32_t interface = node->GetObject<Ipv6> ()->GetInterfaceForDevice (
            route->GetOutputDevice ());
          // we get the inteface address
          Ipv6Address ipv6SrcAddress = (node->GetObject<Ipv6> ()->GetAddress (
                                          interface, 0)).GetAddress ();
          srcAddress = static_cast<Address> (ipv6SrcAddress);
        }
      else
        {
          NS_LOG_WARN ("No route to destination locator. Return.");
          return 0;
        }
    }
  else
    {
      return 0; // should not happen

    }
  // Now we take the mapping entry and look for a Locator address that
  // corresponds to the outgoing interface address

  // Get the mapping entry corresponding to the EID in the db
  Ptr<MapEntry> srcEidMapEntry = DatabaseLookup (srcEid);

  if (srcEidMapEntry != 0)
    {
      Ptr<Locators> locs = srcEidMapEntry->GetLocators ();
      // we look for an RLOC address that corresponds to the output ifAddress
      srcLocator = locs->FindLocator (srcAddress);

      if (!srcLocator)
        {
          NS_LOG_ERROR ("No locator address for outgoing interface " << Ipv4Address::ConvertFrom (srcAddress));
          return 0;
        }
    }
  if (isDestIpv4)
    {
      // should never be true
      if (!Ipv4Address::IsMatchingType (srcLocator->GetRlocAddress ()))
        {
          return 0;
        }
    }
  else
    {
      if (!Ipv6Address::IsMatchingType (srcLocator->GetRlocAddress ()))
        {
          return 0;
        }
    }
  // Doit on verifier si c'est une interface locale alors qu'on sait que l'adresse est celle de l'interface de sortie ?
  // TODO add it back in condition && srcLocator->GetRlocMetrics ()->IsLocalInterface ()
  if (srcLocator->GetRlocMetrics ()->IsUp () && srcLocator->GetRlocMetrics ()->IsLocalInterface () && srcLocator->GetRlocMetrics ()->GetPriority () < LispProtocol::LISP_MAX_RLOC_PRIO)
    {
      return srcLocator;
    }

  NS_LOG_LOGIC ("No source locator found for source Address");
  return 0;
}

void SimpleMapTables::MapRequest (void)
{

}

void SimpleMapTables::MapFree (void)
{

}

void SimpleMapTables::GetMapEntryList (MapTables::MapEntryLocation location, std::list<Ptr<MapEntry> > &entryList)
{

  if (location == MapTables::IN_DATABASE)
    {
      for (std::map<Ptr<EndpointId>, Ptr<MapEntry> >::iterator it = m_mappingDatabase.begin (); it != m_mappingDatabase.end (); ++it)
        {
          it->second->SetEidPrefix (it->first);
          entryList.push_back (it->second);
        }
    }
  else if (location == MapTables::IN_CACHE)
    {
      for (std::map<Ptr<EndpointId>, Ptr<MapEntry> >::iterator it =
             m_mappingCache.begin (); it != m_mappingCache.end (); ++it)
        {
          it->second->SetEidPrefix (it->first);
          entryList.push_back (it->second);
        }
    }
}

bool SimpleMapTables::IsMapForReceivedPacket (Ptr <const Packet> p, const LispHeader &header, const Address &srcRloc, const Address &destRloc)
{
  NS_LOG_FUNCTION (this);
  Ipv4Header innerHeader;
  Ptr<Locator> srcLocator;
  Ptr<Locator> destLocator;
  Ptr<LispOverIp> lispOverIp = MapTables::GetLispOverIp ();
  p->PeekHeader (innerHeader);

  Address srcEidAddress = static_cast<Address> (innerHeader.GetSource ());
  Address destEidAddress = static_cast<Address> (innerHeader.GetDestination ());

  // The destination address should be in the Db
  /* TAKE CARE OF THE DESTRLOC */
  Ptr<MapEntry> localMapEntry = DatabaseLookup (destEidAddress);


  if (!localMapEntry)
    {
      /* we received a LISP packet addressed to the wrong ETR
       * we assume that the local mapping is alway present
       * TODO Error no mapping in DB DROP
       */
      lispOverIp->GetLispStatisticsV4 ()->NoLocalMap ();
      return false;
    }

  destLocator = localMapEntry->FindLocator (destRloc);
  if (!destLocator)
    {
      /*
       * the mapping exists in the database but the RLOC address
       *  doesn't.
       *  this should not happen
       *  TODO DROP packet
       */
      lispOverIp->GetLispStatisticsV4 ()->NoLocalMap ();
      return false;
    }
  NS_LOG_DEBUG ("Local Map Entry exists for destination eid " << Ipv4Address::ConvertFrom (destEidAddress) << ": " << localMapEntry->RlocSelection ());
  /* TAKE CARE OF THE SRCRLOC */
  Ptr<MapEntry> remoteMapEntry = CacheLookup (srcEidAddress);

  if (remoteMapEntry)
    {
      srcLocator = remoteMapEntry->FindLocator (srcRloc);
      if (!srcLocator)
        {
          /*
           * The entry exists in the cache but the
           * RLOC address of the remote ETR doesn't
           * We should notify through socket or not
           * TODO Drop Packet;
           */
          lispOverIp->GetLispStatisticsV4 ()->NoLocalMap ();
          return false;
        }
    }
  else
    {
      /*
       * There is no entry in the cache...
       * This should happens the first time we look for
       * an eid address
       * NB: there are 3 behavior possible,
       * here we choose the default one --- Do Nothing.
       * TODO Notify through socket
       */
      /*Do nothing*/
    }

  // check the LISP header
  if (!LispProtocol::CheckLispHeader (header, localMapEntry, remoteMapEntry, srcLocator, destLocator, lispOverIp))
    {
      //lispOverIp->GetLispStatisticsV4 ()->;
      /*
       * There is an error with the lisp header
       * TODO DROP packet
       */

      return false;
    }

  return true;
}

////////////////
// LocatorsImpl
LocatorsImpl::LocatorsImpl ()
{

}

LocatorsImpl::~LocatorsImpl ()
{

}

Ptr<Locator> LocatorsImpl::GetLocatorByIdx (uint8_t locIndex)
{
  NS_ASSERT (locIndex < m_locatorsChain.size () && locIndex >= 0);

  int i = 0;
  for (std::list<Ptr<Locator> >::iterator it = m_locatorsChain.begin ();
       it != m_locatorsChain.end (); ++it, i++)
    {
      if (i == locIndex)
        {
          return *it;
        }
    }
  return 0;
}

Ptr<Locator>
LocatorsImpl::FindLocator (const Address &address) const
{
  for (std::list<Ptr<Locator> >::const_iterator it = m_locatorsChain.begin ();
       it != m_locatorsChain.end (); ++it)
    {
      if ((*it)->GetRlocAddress () == address)
        {
          return *it;
        }
    }
  return 0;
}

uint8_t
LocatorsImpl::GetNLocators (void) const
{
  return m_locatorsChain.size ();
}

Ptr<Locator>
LocatorsImpl::SelectFirsValidRloc (void) const
{
  /*
   * the first valid rloc of the linked list
   * is the one which is up and whose priority is < than 255
   */
  for (std::list<Ptr<Locator> >::const_iterator it = m_locatorsChain.begin ();
       it != m_locatorsChain.end (); ++it)
    {
      if ((*it)->GetRlocMetrics ()->IsUp () && (*it)->GetRlocMetrics ()->GetPriority () < LispProtocol::LISP_MAX_RLOC_PRIO)
        {
          return *it;
        }
    }
  return 0;
}

void
LocatorsImpl::InsertLocator (Ptr<Locator> locator)
{
  m_locatorsChain.push_back (locator);
  m_locatorsChain.sort (compare_rloc);
}


std::string LocatorsImpl::Print (void) const
{
  std::string locators = std::string ();

  int i = 1;
  std::stringstream str;
  for (std::list<Ptr<Locator> >::const_iterator it = m_locatorsChain.begin ();
       it != m_locatorsChain.end (); ++it)
    {
      str << i;
      locators += "RLOC" + str.str () + "\n";
      locators += (*it)->Print ();
      str.str (std::string ());
      i++;
    }

  return locators;
}

int LocatorsImpl::Serialize (uint8_t *buf)
{
  buf[0] = GetNLocators ();
  int position = 1;
  uint8_t size = 0;

  for (std::list<Ptr<Locator> >::const_iterator it = m_locatorsChain.begin (); it != m_locatorsChain.end (); ++it)
    {
      position++;
      size = (*it)->Serialize (buf + position);
      buf[position - 1] = size;
      position += size;
    }
  return position;
}

Ptr<LocatorsImpl> LocatorsImpl::Deserialize (const uint8_t *buf)
{
  uint8_t locCount = buf[0];
  int position = 1;
  uint8_t size = 0;

  Ptr<LocatorsImpl> locators = Create<LocatorsImpl> ();

  for (int i = 0; i < locCount; i++)
    {
      size = buf[position];
      locators->InsertLocator (Locator::Deserialized (buf + position + 1));
      position += size + 1;
    }
  return locators;
}

bool LocatorsImpl::compare_rloc (Ptr<const Locator> a, Ptr<const Locator> b)
{
  if (a->GetRlocMetrics ()->IsUp () && a->GetRlocMetrics ()->GetPriority () < b->GetRlocMetrics ()->GetPriority ())
    {
      return true;
    }
  else if (b->GetRlocMetrics ()->IsUp () && b->GetRlocMetrics ()->GetPriority () < a->GetRlocMetrics ()->GetPriority ())
    {
      return false;
    }
  else
    {
      return a->GetRlocMetrics ()->GetWeight () < b->GetRlocMetrics ()->GetWeight ();
    }
}

//////////////////
// MapEntryImpl

MapEntryImpl::MapEntryImpl ()
{
  m_locators = Create<LocatorsImpl> ();
}

MapEntryImpl::MapEntryImpl (Ptr<Locator> locator)
{
  m_locators = Create<LocatorsImpl> ();
  m_locators->InsertLocator (locator);
}




MapEntryImpl::~MapEntryImpl ()
{

}

Ptr<Locator>
MapEntryImpl::FindLocator (const Address &address) const
{
  return m_locators->FindLocator (address);
}

Ptr<Locator>
MapEntryImpl::RlocSelection (void) const
{
  return m_locators->SelectFirsValidRloc ();
}

std::string MapEntryImpl::Print (void) const
{
  //std::string mapEntry = "EID prefix:";

  return m_locators->Print ();
}

} /* namespace ns3 */
