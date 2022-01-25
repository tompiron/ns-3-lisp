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

#include <stdint.h>
#include "lisp-over-ip.h"
#include "ns3/node.h"
#include <ns3/log.h>
#include "ns3/object.h"
#include "ns3/object-vector.h"
#include "lisp-mapping-socket-factory.h"
#include "lisp-mapping-socket.h"
#include "simple-map-tables.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LispOverIp");

NS_OBJECT_ENSURE_REGISTERED (LispOverIp);

TypeId
LispOverIp::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::LispOverIp")
    .SetParent<Object> ()
    .SetGroupName ("Lisp")
    .AddAttribute ("SocketList", "The list of sockets associated to this protocol (lisp).",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&LispOverIp::m_sockets),
                   MakeObjectVectorChecker<LispMappingSocket> ())
  ;
  return tid;
}

LispOverIp::LispOverIp (Ptr<LispStatistics> statisticsForIpv4, Ptr<LispStatistics> statisticsForIpv6)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (statisticsForIpv4 && statisticsForIpv6);
  m_statisticsForIpv4 = statisticsForIpv4;
  m_statisticsForIpv6 = statisticsForIpv6;
}

void LispOverIp::SetLispStatistics (Ptr<LispStatistics> statisticsV4, Ptr<LispStatistics> statisticsV6)
{
  m_statisticsForIpv4 = statisticsV4;
  m_statisticsForIpv6 = statisticsV6;
}

LispOverIp::LispOverIp ()
{
  NS_LOG_FUNCTION (this);
  m_lispSocket = CreateSocket ();
}

LispOverIp::~LispOverIp ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Node> LispOverIp::GetNode ()
{
  return m_node;
}

void LispOverIp::SetNode (Ptr<Node> node)
{
  m_node = node;
}

void
LispOverIp::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION (this);
  if (m_node == 0)
    {
      Ptr<Node>node = this->GetObject<Node>();
      // verify that it's a valid node and that
      // the node has not been set before
      if (node != 0)
        {
          this->SetNode (node);
          if (!node->GetObject<LispMappingSocketFactory> ())
            {
              Ptr<LispMappingSocketFactory> mappingFactory = CreateObject<LispMappingSocketFactory> ();
              mappingFactory->SetLisp (this);
              node->AggregateObject (mappingFactory);
            }
        }
    }
  Object::NotifyNewAggregate ();
}

void LispOverIp::DoDispose (void)
{

}

Ptr<Socket> LispOverIp::CreateSocket (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<LispMappingSocket> socket = CreateObject<LispMappingSocket> ();
  socket->SetNode (m_node);
  socket->SetLisp (this);
  socket->SetSockIndex (m_sockets.size ());
  m_sockets.push_back (socket);
  return socket;
}

Ptr<LispMappingSocket> LispOverIp::GetMappingSocket (uint8_t sockIndex)
{
  return m_sockets.at (sockIndex);
}

void LispOverIp::SetMapTablesIpv4 (Ptr<MapTables> mapTablesIpv4)
{
  NS_ASSERT (mapTablesIpv4 != 0);
  m_mapTablesIpv4 = mapTablesIpv4;
  m_mapTablesIpv4->SetLispOverIp (this);
}
void LispOverIp::SetMapTablesIpv6 (Ptr<MapTables> mapTablesIpv6)
{
  NS_ASSERT (mapTablesIpv6 != 0);
  m_mapTablesIpv6 = mapTablesIpv6;
  m_mapTablesIpv6->SetLispOverIp (this);
}

Ptr<MapEntry> LispOverIp::DatabaseLookup (Address const &eidAddress) const
{
  if (Ipv4Address::IsMatchingType (eidAddress))
    {
      return m_mapTablesIpv4->DatabaseLookup (eidAddress);
    }
  else if (Ipv6Address::IsMatchingType (eidAddress))
    {
      return m_mapTablesIpv6->DatabaseLookup (eidAddress);
    }
  return 0;
}

Ptr<MapEntry> LispOverIp::CacheLookup (Address const &eidAddress) const
{
  if (Ipv4Address::IsMatchingType (eidAddress))
    {
      return m_mapTablesIpv4->CacheLookup (eidAddress);
    }
  else if (Ipv6Address::IsMatchingType (eidAddress))
    {
      return m_mapTablesIpv6->CacheLookup (eidAddress);
    }
  return 0;
}

Ptr<Locator> LispOverIp::SelectDestinationRloc (Ptr<const MapEntry> mapEntry) const
{
  return mapEntry->RlocSelection ();
}

Ptr<Locator> LispOverIp::SelectSourceRloc (Address const &srcEid, Ptr<const Locator> destLocator) const
{
  if (Ipv4Address::IsMatchingType (srcEid))
    {
      return m_mapTablesIpv4->SourceRlocSelection (srcEid, destLocator);
    }
  else if (Ipv6Address::IsMatchingType (srcEid))
    {
      return m_mapTablesIpv6->SourceRlocSelection (srcEid, destLocator);
    }
  return 0;
}

Ptr<LispStatistics> LispOverIp::GetLispStatisticsV4 (void)
{
  return m_statisticsForIpv4;
}

Ptr<LispStatistics> LispOverIp::GetLispStatisticsV6 (void)
{
  return m_statisticsForIpv6;
}

void LispOverIp::OpenLispMappingSocket (void)
{
  NS_LOG_FUNCTION (this);
  // create mapping socket address
  m_lispAddress = static_cast<Address> (MappingSocketAddress (m_node->GetDevice (0)->GetAddress (), 0));

  // bind
  m_lispSocket->Bind (m_lispAddress);
  m_lispSocket->SetRecvCallback (MakeCallback (&LispOverIp::HandleMapSockRead, this));
  NS_LOG_DEBUG ("Bound to " << m_lispAddress);
}

void LispOverIp::HandleMapSockRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {
      MappingSocketMsgHeader sockMsgHdr;
      packet->RemoveHeader (sockMsgHdr);
      uint8_t buf[packet->GetSize ()];
      packet->CopyData (buf, packet->GetSize ());
      Ptr<MappingSocketMsg> msg = MappingSocketMsg::Deserialize (buf);
      NS_LOG_DEBUG ("MSG HEADER " << sockMsgHdr);
      if (sockMsgHdr.GetMapType () == static_cast<uint16_t> (LispMappingSocket::MAPM_ADD))
        {
          NS_LOG_DEBUG ("ADD Message received on lisp" << msg->GetEndPointId ()->Print () << " " << msg->GetLocators ()->Print ());
          Ptr<EndpointId> eid = msg->GetEndPointId ();
          Ptr<MapEntry> mapEntry = Create<MapEntryImpl> ();
          Ptr<Locators> locators;
          mapEntry->SetEidPrefix (eid);
          if ((int) sockMsgHdr.GetMapFlags () & (int) LispMappingSocket::MAPF_NEGATIVE)
            {
              NS_LOG_DEBUG ("MAP ENTRY is negative!");
              mapEntry->setIsNegative (1);
            }
          else
            {
              NS_LOG_DEBUG ("Setting Is Local If to 0!");
              locators = msg->GetLocators ();
              for (int i = 0; i < locators->GetNLocators (); ++i)
                {
                  Ptr<Locator> locator = locators->GetLocatorByIdx (i);
                  locator->GetRlocMetrics ()->SetIsLocalIf (0);
                }
              mapEntry->setIsNegative (0);
              mapEntry->SetLocators (locators);
            }
          if (eid->IsIpv4 ())
            {
              m_mapTablesIpv4->SetEntry (eid->GetEidAddress (), eid->GetIpv4Mask (), mapEntry, MapTables::IN_CACHE);
              NS_LOG_DEBUG ("MAP ENTRY IPV4 SET WE CAN GO!");
            }
          else
            {
              m_mapTablesIpv6->SetEntry (eid->GetEidAddress (), eid->GetIpv6Prefix (), mapEntry, MapTables::IN_CACHE);
              NS_LOG_DEBUG ("MAP ENTRY IPV6 SET WE CAN GO!");
            }
        }
      else if (sockMsgHdr.GetMapType () == static_cast<uint16_t> (LispMappingSocket::MAPM_DELETE))
        {

        }
      else if (sockMsgHdr.GetMapType () == static_cast<uint16_t> (LispMappingSocket::MAPM_GET))
        {

        }
    }
}

Address LispOverIp::GetLispMapSockAddress (void)
{
  return m_lispAddress;
}

void LispOverIp::SendNotifyMessage (uint8_t messageType, Ptr<Packet> packet, MappingSocketMsgHeader mapSockMsgHeader, int flags)
{
  // Notify message is sent to every application
  NS_LOG_FUNCTION (messageType);

  switch (messageType)
    {
    case (static_cast<uint8_t> (LispMappingSocket::MAPM_MISS)):
      switch (LispMappingSocket::m_lispMissMsgType)
        {
        case (LispMappingSocket::LISP_MISSMSG_EID):
          break;
        default:
          break;
        }
      break;
    case (static_cast<uint8_t> (LispMappingSocket::MAPM_DELETE)):
      break;
    default:
      break;
    }
  mapSockMsgHeader.SetMapFlags (flags | LispMappingSocket::MAPF_DONE);
  packet->AddHeader (mapSockMsgHeader);
  NS_LOG_DEBUG ("Send Messages " << m_sockets.size ());
  for (uint32_t i = 1; i < m_sockets.size (); ++i)
    {
      Address ad = static_cast<Address> (MappingSocketAddress ());
      m_sockets.at (i)->GetSockName (ad);
      m_lispSocket->Connect (ad);
      m_lispSocket->Send (packet);
      NS_LOG_DEBUG ("Send Notif to " << MappingSocketAddress::ConvertFrom (ad) << "from " << MappingSocketAddress::ConvertFrom (m_lispAddress));
    }
}

void LispOverIp::SetRlocsList (const std::set<Address> rlocsList)
{
  m_rlocsList = rlocsList;
}

std::set<Address> LispOverIp::GetRlocsList (void) const
{
  return m_rlocsList;
}

bool LispOverIp::IsLocatorInList (Address address) const
{

  for (std::set<Address>::const_iterator it = m_rlocsList.begin (); it != m_rlocsList.end (); ++it)
    {
      if (Ipv4Address::IsMatchingType (address) && Ipv4Address::IsMatchingType (*it)
          && Ipv4Address::ConvertFrom (address).IsEqual (Ipv4Address::ConvertFrom (*it)))
        {
          return true;
        }
      else if (Ipv6Address::IsMatchingType (address) && Ipv6Address::IsMatchingType (*it)
               && Ipv6Address::ConvertFrom (address).IsEqual (Ipv6Address::ConvertFrom (*it)))
        {
          return true;
        }
      else
        {
          continue;
        }
    }
  return false;
}

Ptr<MapTables> LispOverIp::GetMapTablesV4 (void) const
{
  return m_mapTablesIpv4;
}

Ptr<MapTables> LispOverIp::GetMapTablesV6 (void) const
{
  return m_mapTablesIpv6;
}

} /* namespace ns3 */
