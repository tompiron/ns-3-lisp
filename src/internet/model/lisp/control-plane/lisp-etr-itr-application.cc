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

#include "lisp-etr-itr-application.h"
#include "ns3/log.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/map-register-msg.h"
#include "ns3/map-reply-msg.h"
#include "map-resolver.h"
#include "ns3/lisp-mapping-socket.h"
#include "ns3/mapping-socket-msg.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include <climits>
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv6-route.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LispEtrItrApplication");

NS_OBJECT_ENSURE_REGISTERED (LispEtrItrApplication);


TypeId
LispEtrItrApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LispEtrItrApplication")
    .SetParent<Application> ()
    .SetGroupName ("Lisp")
    .AddConstructor<LispEtrItrApplication> ()
    .AddAttribute (
    "PeerPort", "The destination port of the packet",
    UintegerValue (LispProtocol::LISP_SIG_PORT),
    MakeUintegerAccessor (&LispEtrItrApplication::m_peerPort),
    MakeUintegerChecker<uint16_t> ())
    .AddAttribute (
    "Interval", "The time to wait between packets",
    TimeValue (Seconds (3.0)),
    MakeTimeAccessor (&LispEtrItrApplication::m_interval), MakeTimeChecker ())
  ;
  return tid;
}

LispEtrItrApplication::LispEtrItrApplication ()
{
  NS_LOG_FUNCTION (this);
  m_lispMappingSocket = 0;
  m_event = EventId ();
  m_sent = 0;
  m_requestSent = 0;
  m_lispProtoAddress = Address (); // invalid address
}

LispEtrItrApplication::~LispEtrItrApplication ()
{

}


void LispEtrItrApplication::SetMapServerAddresses (std::list<Address> mapServerAddresses)
{
  m_mapServerAddress = mapServerAddresses;
}

void LispEtrItrApplication::SetMapTables (Ptr<MapTables> mapTablesV4, Ptr<MapTables> mapTablesV6)
{
  m_mapTablesV4 = mapTablesV4;
  m_mapTablesV6 = mapTablesV6;
}

void LispEtrItrApplication::AddMapResolverLoc (Ptr<Locator> locator)
{
  m_mapResolverRlocs.push_back (locator);
}

void LispEtrItrApplication::DoDispose (void)
{
  Application::DoDispose ();
}

void LispEtrItrApplication::StartApplication (void)
{
  if (m_lispMappingSocket == 0)
    {
      NS_LOG_DEBUG ("Trying to Connect to lispProto");
      TypeId tid = TypeId::LookupByName ("ns3::LispMappingSocketFactory");
      m_lispMappingSocket = Socket::CreateSocket (GetNode (), tid);
      Ptr<LispOverIp> lisp = m_lispMappingSocket->GetNode ()->GetObject<LispOverIp> ();
      m_lispProtoAddress = lisp->GetLispMapSockAddress ();
      m_lispMappingSocket->Bind ();
      m_lispMappingSocket->Connect (m_lispProtoAddress);
      NS_LOG_DEBUG ("Connected to " << m_lispProtoAddress);
    }
  m_lispMappingSocket->SetRecvCallback (MakeCallback (&LispEtrItrApplication::HandleMapSockRead, this));

  if (m_socket == 0)
    {
      NS_LOG_DEBUG ("Trying to Connect to lispProto");
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType (m_mapServerAddress.front ()))
        {
          m_socket->Bind ();
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom (m_mapServerAddress.front ()), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType (m_mapServerAddress.front ()))
        {
          m_socket->Bind6 ();
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom (m_mapServerAddress.front ()), m_peerPort));
        }
    }
  m_socket->SetRecvCallback (MakeCallback (&LispEtrItrApplication::HandleRead, this));

  if (m_lispCtlMsgRcvSocket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_lispCtlMsgRcvSocket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), LispProtocol::LISP_SIG_PORT);
      m_lispCtlMsgRcvSocket->Bind (local);
    }

  if (m_lispCtlMsgRcvSocket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_lispCtlMsgRcvSocket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), LispProtocol::LISP_SIG_PORT);
      m_lispCtlMsgRcvSocket6->Bind (local6);
    }
  m_lispCtlMsgRcvSocket6->SetRecvCallback (MakeCallback (&LispEtrItrApplication::HandleReadControlMsg, this));
  m_lispCtlMsgRcvSocket->SetRecvCallback (MakeCallback (&LispEtrItrApplication::HandleReadControlMsg, this));
  ScheduleTransmit (Seconds (0.));
  NS_LOG_DEBUG ("Lisp ETR ITR App Stated");
}

void LispEtrItrApplication::StopApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_lispMappingSocket != 0)
    {
      m_lispMappingSocket->Close ();
      m_lispMappingSocket->SetRecvCallback (
        MakeNullCallback<void, Ptr<Socket> > ());
      m_lispMappingSocket = 0;
    }

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (
        MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  if (m_lispCtlMsgRcvSocket)
    {
      m_lispCtlMsgRcvSocket->Close ();
      m_lispCtlMsgRcvSocket->SetRecvCallback (
        MakeNullCallback<void, Ptr<Socket> > ());
      m_lispCtlMsgRcvSocket = 0;
    }

  if (m_lispCtlMsgRcvSocket6)
    {
      m_lispCtlMsgRcvSocket6->Close ();
      m_lispCtlMsgRcvSocket6->SetRecvCallback (
        MakeNullCallback<void, Ptr<Socket> > ());
      m_lispCtlMsgRcvSocket6 = 0;
    }

  Simulator::Cancel (m_event);
}


void LispEtrItrApplication::ScheduleTransmit (Time dt)
{
  m_event = Simulator::Schedule (dt, &LispEtrItrApplication::SendMapRegisters, this);
}

void LispEtrItrApplication::SendMapRegisters (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_event.IsExpired ());

  std::list<Ptr<MapEntry> > mapEntries;
  m_mapTablesV4->GetMapEntryList (MapTables::IN_DATABASE, mapEntries);
  m_mapTablesV6->GetMapEntryList (MapTables::IN_DATABASE, mapEntries);

  for (std::list<Ptr<MapEntry> >::const_iterator it = mapEntries.begin (); it != mapEntries.end (); ++it)
    {
      Ptr<MapRegisterMsg> msg = Create<MapRegisterMsg> ();
      Ptr<MapReplyRecord> record = Create<MapReplyRecord> ();

      msg->SetM (0);
      msg->SetP (0);
      msg->SetNonce (0); // Nonce is 0 for map register

      record->SetEidPrefix ((*it)->GetEidPrefix ()->GetEidAddress ());
      if (record->GetEidAfi () == LispControlMsg::IP)
        {
          record->SetEidMaskLength ((*it)->GetEidPrefix ()->GetIpv4Mask ().GetPrefixLength ());
        }
      else if (record->GetEidAfi () == LispControlMsg::IPV6)
        {
          record->SetEidMaskLength ((*it)->GetEidPrefix ()->GetIpv6Prefix ().GetPrefixLength ());
        }
      record->SetLocators ((*it)->GetLocators ());
      record->SetMapVersionNumber ((*it)->GetVersionNumber ());
      msg->SetRecord (record);
      msg->SetRecordCount (1);

      uint8_t buf[72];

      msg->Serialize (buf);
      MapResolver::ConnectToPeerAddress (m_mapServerAddress.front (), LispProtocol::LISP_SIG_PORT, m_socket);
      Ptr<Packet> p = Create<Packet> (buf, 72);
      m_socket->Send (p);
      NS_LOG_DEBUG ("Map-Register sent");
    }

  ++m_sent;

  /*if (m_sent < m_count)
    {
      ScheduleTransmit (m_interval);
    }*/
}

void LispEtrItrApplication::SendToLisp (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  m_lispMappingSocket->Send (packet);

  /*if (m_sent < m_count)
    {
      ScheduleTransmit (m_interval);
    }*/
  NS_LOG_DEBUG ("Hey I sent something!");
}

void LispEtrItrApplication::HandleRead (Ptr<Socket> socket)
{

}

void LispEtrItrApplication::HandleReadControlMsg (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {

        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {

        }

      uint8_t buf[packet->GetSize ()];
      packet->CopyData (buf, packet->GetSize ());

      if (buf[0] == MapRequestMsg::GetMsgType ())
        {
          Ptr<MapRequestMsg> requestMsg = MapRequestMsg::Deserialize (buf);
          NS_LOG_DEBUG ("Receive Map-Request on ETR from " << Ipv4Address::ConvertFrom (requestMsg->GetItrRlocAddrIp ()));
          Ptr<MapRequestRecord> record = requestMsg->GetMapRequestRecord ();
          Ptr<MapEntry> entry;
          if (record->GetAfi () == LispControlMsg::IP)
            {
              // TODO May be use mapping socket instead
              entry = m_mapTablesV4->DatabaseLookup (record->GetEidPrefix ());
            }
          else if (record->GetAfi () == LispControlMsg::IPV6)
            {
              entry = m_mapTablesV6->DatabaseLookup (record->GetEidPrefix ());
            }
          if (entry == 0)
            {
              NS_LOG_DEBUG ("Send Negative Map-Reply");
            }
          else
            {
              NS_LOG_DEBUG ("Send Map-Reply to ITR");
              Ptr<MapReplyMsg> mapReply = Create<MapReplyMsg> ();
              Ptr<MapReplyRecord> replyRecord = Create<MapReplyRecord> ();

              mapReply->SetNonce (requestMsg->GetNonce ());
              mapReply->SetRecordCount (1);

              replyRecord->SetAct (MapReplyRecord::NoAction);
              replyRecord->SetA (1);
              replyRecord->SetMapVersionNumber (entry->GetVersionNumber ());
              replyRecord->SetRecordTtl (MapReplyRecord::m_defaultRecordTtl);
              replyRecord->SetEidPrefix (entry->GetEidPrefix ()->GetEidAddress ());

              if (entry->GetEidPrefix ()->IsIpv4 ())
                {
                  replyRecord->SetEidMaskLength (entry->GetEidPrefix ()->GetIpv4Mask ().GetPrefixLength ());
                }
              else
                {
                  replyRecord->SetEidMaskLength (entry->GetEidPrefix ()->GetIpv6Prefix ().GetPrefixLength ());
                }

              replyRecord->SetLocators (entry->GetLocators ());

              mapReply->SetRecord (replyRecord);
              if (requestMsg->GetItrRlocAddrIp () != static_cast<Address> (Ipv4Address ()))
                {
                  MapResolver::ConnectToPeerAddress (requestMsg->GetItrRlocAddrIp (), m_peerPort, m_socket);
                }
              else if ((requestMsg->GetItrRlocAddrIpv6 () != static_cast<Address> (Ipv6Address ())))
                {
                  MapResolver::ConnectToPeerAddress (requestMsg->GetItrRlocAddrIpv6 (), m_peerPort, m_socket);
                }
              else
                {
                  NS_LOG_ERROR ("NO VALID ITR address");
                }
              NS_LOG_DEBUG ("MAP REPLY READY");
              mapReply->Print (std::cout);
              uint8_t buf[256];
              mapReply->Serialize (buf);
              Ptr<Packet> replyPacket = Create<Packet> (buf, 256);
              Send (replyPacket);
              NS_LOG_DEBUG ("Map Reply Sent to " << Ipv4Address::ConvertFrom (requestMsg->GetItrRlocAddrIp ()));
            }
        }
      else if (buf[0] == static_cast<uint8_t> (MapReplyMsg::GetMsgType ()))
        {
          NS_LOG_DEBUG ("GET MAP REPLY ? " << unsigned (buf[0]));
          // Get Map Reply
          Ptr<MapReplyMsg> replyMsg = MapReplyMsg::Deserialize (buf);
          Ptr<MapReplyRecord> replyRecord = replyMsg->GetRecord ();

          // prepare mapping socket msg
          Ptr<MappingSocketMsg> mapSockMsg = Create<MappingSocketMsg> ();
          MappingSocketMsgHeader mapSockHeader;
          mapSockHeader.SetMapType (LispMappingSocket::MAPM_ADD);
          mapSockHeader.SetMapVersioning (replyRecord->GetMapVersionNumber ());
          if (replyRecord)
            {
              mapSockHeader.SetMapRlocCount (replyRecord->GetLocatorCount ());
              if (!replyRecord->GetLocatorCount ())
                {
                  // Negative map reply
                  mapSockHeader.SetMapFlags ((int) mapSockHeader.GetMapFlags () | static_cast<int> (LispMappingSocket::MAPF_NEGATIVE));
                  mapSockHeader.SetMapAddresses ((int) mapSockHeader.GetMapAddresses () | static_cast<int> (LispMappingSocket::MAPA_EIDMASK));
                }
              mapSockHeader.SetMapAddresses ((int) mapSockHeader.GetMapAddresses () | static_cast<int> (LispMappingSocket::MAPA_RLOC));

              mapSockMsg->SetLocators (replyRecord->GetLocators ());
            }
          else
            {
              NS_LOG_ERROR ("There must always be a Record in the Reply msg!");
            }

          std::stringstream ss;
          ss << "/" << (int) replyRecord->GetEidMaskLength ();
          Ptr<EndpointId> eid;
          if (replyRecord->GetEidAfi () == LispControlMsg::IP)
            {
              eid = Create<EndpointId> (replyRecord->GetEidPrefix (), Ipv4Mask (ss.str ().c_str ()));
              mapSockMsg->SetEndPoint (eid);
            }
          else
            {
              eid = Create<EndpointId> (replyRecord->GetEidPrefix (), Ipv6Prefix (ss.str ().c_str ()));
              mapSockMsg->SetEndPoint (eid);
            }

          uint8_t buf[256];
          mapSockMsg->Serialize (buf);
          Ptr<Packet> packet = Create<Packet> (buf, 256);
          packet->AddHeader (mapSockHeader);
          SendToLisp (packet);
          DeleteFromMapReqList (eid);
        }
      else if (buf[0] == static_cast<uint8_t> (LispControlMsg::MAP_NOTIFY))
        {

        }
      else
        {
          NS_LOG_ERROR ("Problem with packet!");
        }
    }
}

void LispEtrItrApplication::HandleMapSockRead (Ptr<Socket> lispMappingSocket)
{
  NS_LOG_FUNCTION (this);

  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  SeedManager::SetSeed (++m_seed % ULONG_MAX);

  Ptr<Packet> packet;
  Address from;

  while ((packet = lispMappingSocket->RecvFrom (from)))
    {
      NS_LOG_DEBUG ("Hello I got something from lisp :)");
      MappingSocketMsgHeader sockMsgHdr;
      packet->RemoveHeader (sockMsgHdr);
      uint8_t buf[packet->GetSize ()];
      packet->CopyData (buf, packet->GetSize ());
      Ptr<MappingSocketMsg> msg = MappingSocketMsg::Deserialize (buf);
      NS_LOG_DEBUG ("MSG HEADER " << sockMsgHdr);
      if (sockMsgHdr.GetMapType () == static_cast<uint16_t> (LispMappingSocket::MAPM_MISS))
        {
          if (IsInRequestList (msg->GetEndPointId ()))
            {
              NS_LOG_DEBUG ("EID is in REQUEST LIST!!!");
              return;
            }
          // Build map request msg
          Ptr<MapRequestMsg> mapReqMsg = Create<MapRequestMsg> ();
          Address itrAddress = GetLocalAddress (m_mapResolverRlocs.front ()->GetRlocAddress ());
          if (Ipv4Address::IsMatchingType (itrAddress))
            {
              NS_LOG_DEBUG ("Cannot find " << msg->GetEndPointId ()->Print () << " on " << Ipv4Address::ConvertFrom (itrAddress));
              mapReqMsg->SetItrRlocAddrIp (itrAddress);
            }
          else
            {
              mapReqMsg->SetItrRlocAddrIpv6 (itrAddress);
            }

          mapReqMsg->SetIrc (1);
          mapReqMsg->SetNonce (uv->GetInteger (0, UINT_MAX));
          Address eidAddress = msg->GetEndPointId ()->GetEidAddress ();

          mapReqMsg->SetSourceEidAddr (static_cast<Address> (Ipv4Address ()));
          mapReqMsg->SetSourceEidAfi (LispControlMsg::IP);
          uint8_t maskLength = 0;
          if (Ipv4Address::IsMatchingType (eidAddress))
            {
              maskLength = 32;
            }
          else if (Ipv6Address::IsMatchingType (eidAddress))
            {
              maskLength = 128;
            }

          mapReqMsg->SetMapRequestRecord (Create<MapRequestRecord> (eidAddress, maskLength));

          uint8_t bufMapReq[64];
          mapReqMsg->Serialize (bufMapReq);

          Ptr<Packet> packet2 = Create<Packet> (bufMapReq, 64);
          AddInMapReqList (msg->GetEndPointId (), mapReqMsg);
          MapResolver::ConnectToPeerAddress (m_mapResolverRlocs.front ()->GetRlocAddress (), LispProtocol::LISP_SIG_PORT, m_socket);
          Send (packet2);
        }
    }
}

void LispEtrItrApplication::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_event.IsExpired ());

  m_socket->Send (packet);
  ++m_sent;
}

Address LispEtrItrApplication::GetLocalAddress (Address address)
{

  /*Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4> ();
  Ptr<NetDevice> device;;
  if (ipv4)
    {
      device = ipv4->GetNetDevice (1);
      Ipv4InterfaceAddress ifAddress = ipv4->GetAddress (device->GetIfIndex(), 0);
      NS_LOG_DEBUG ("LOCAL ADDRESS --- " << ifAddress.GetLocal ());
      return static_cast<Address> (ifAddress.GetLocal ());
    }
  else
    {
      Ptr<Ipv6> ipv6 = m_node->GetObject<Ipv6> ();
      device = ipv6->GetNetDevice (1);
      Ipv6InterfaceAddress ifAddress = ipv6->GetAddress (device->GetIfIndex(), 0);
      return static_cast<Address> (ifAddress.GetAddress ());
    }*/
  Socket::SocketErrno errno_;
  Address srcAddress;
  Ptr<Locator> srcLocator;
  // if the destination is Ipv4
  if (Ipv4Address::IsMatchingType (address))
    {
      Ptr<Ipv4Route> route = 0;
      Ptr<Ipv4RoutingProtocol> routingProtocol =
        m_node->GetObject<Ipv4> ()->GetRoutingProtocol ();

      if (routingProtocol != 0)
        {
          Ipv4Header header = Ipv4Header ();
          header.SetDestination (Ipv4Address::ConvertFrom (address));
          route = routingProtocol->RouteOutput (
            0,
            header,
            0, errno_);
        }
      else
        {
          NS_LOG_ERROR (
            "Map resolver unreachable: routingProtocol == 0");
        }

      if (route)
        {
          // we know the route and we get the output interface
          int32_t interface = m_node->GetObject<Ipv4> ()->GetInterfaceForDevice (
            route->GetOutputDevice ());
          // we get the interface address
          Ipv4Address ipv4SrcAddress = (m_node->GetObject<Ipv4> ()->GetAddress (
                                          interface, 0)).GetLocal ();
          srcAddress = static_cast<Address> (ipv4SrcAddress);
        }
      else
        {
          NS_LOG_WARN ("No route to Map-Resolver. Return.");
          return static_cast<Address> (Ipv4Address ());
        }
    }
  else if (Ipv6Address::IsMatchingType (address))
    {
      Ptr<Ipv6Route> route = 0;
      Ptr<Ipv6RoutingProtocol> routingProtocol =
        m_node->GetObject<Ipv6> ()->GetRoutingProtocol ();
      if (routingProtocol)
        {
          Ipv6Header header = Ipv6Header ();
          header.SetDestinationAddress (Ipv6Address::ConvertFrom (address));
          route = routingProtocol->RouteOutput (0,
                                                header,
                                                0, errno_);
        }
      else
        {
          NS_LOG_ERROR (
            "Map resolver unreachable: routingProtocol == 0");
        }

      if (route)
        {
          // we know the route and we get the output interface
          int32_t interface = m_node->GetObject<Ipv6> ()->GetInterfaceForDevice (
            route->GetOutputDevice ());
          // we get the inteface address
          Ipv6Address ipv6SrcAddress = (m_node->GetObject<Ipv6> ()->GetAddress (
                                          interface, 0)).GetAddress ();
          srcAddress = static_cast<Address> (ipv6SrcAddress);
        }
      else
        {
          NS_LOG_WARN ("No route to Map-Resolver. Return.");
          return static_cast<Address> (Ipv6Address ());
        }
    }

  return srcAddress;   // should not happen
}

bool LispEtrItrApplication::IsInRequestList (Ptr<EndpointId> eid) const
{
  if (m_requestList.find (eid) != m_requestList.end ())
    {
      return true;
    }
  return false;
}

void LispEtrItrApplication::AddInMapReqList (Ptr<EndpointId> eid, Ptr<MapRequestMsg> reqMsg)
{
  m_requestList.insert (std::pair<Ptr<EndpointId>, Ptr<MapRequestMsg> > (eid, reqMsg));
}

void LispEtrItrApplication::DeleteFromMapReqList (Ptr<EndpointId> eid)
{
  m_requestList.erase (eid);
  NS_LOG_DEBUG ("Remove eid from PENDING LIST");
}

} /* namespace ns3 */
