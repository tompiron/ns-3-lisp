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
#include "map-server-ddt.h"
#include "ns3/simple-map-tables.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "map-register-msg.h"
#include "map-resolver.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("MapServerDdt");

NS_OBJECT_ENSURE_REGISTERED (MapServerDdt);

TypeId
MapServerDdt::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MapServerDdt")
    .SetParent<MapServer> ()
    .SetGroupName ("Lisp")
    .AddConstructor<MapServerDdt> ()
  ;
  return tid;
}

MapServerDdt::MapServerDdt ()
{
  m_mapTablesv4 = Create<SimpleMapTables> ();
  m_mapTablesv6 = Create<SimpleMapTables> ();
}

MapServerDdt::~MapServerDdt ()
{
  m_socket = 0;
  m_msClientSocket = 0;
  m_msClientSocket6 = 0;
}

void MapServerDdt::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("STARTING DDT MAP SERVER");
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      //ConnectToPeerAddress (m_peerAddress, m_peerPort);
    }
  m_socket->SetRecvCallback (MakeCallback (&MapServerDdt::HandleRead, this));
  if (m_msClientSocket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_msClientSocket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), LispProtocol::LISP_SIG_PORT);
      m_msClientSocket->Bind (local);
    }

  if (m_msClientSocket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_msClientSocket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), LispProtocol::LISP_SIG_PORT);
      m_msClientSocket6->Bind (local6);
    }
  m_msClientSocket6->SetRecvCallback (MakeCallback (&MapServerDdt::HandleReadFromClient, this));
  m_msClientSocket->SetRecvCallback (MakeCallback (&MapServerDdt::HandleReadFromClient, this));
}

void MapServerDdt::StopApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  if (m_msClientSocket)
    {
      m_msClientSocket->Close ();
      m_msClientSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_msClientSocket = 0;
    }

  if (m_msClientSocket6)
    {
      m_msClientSocket6->Close ();
      m_msClientSocket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_msClientSocket6 = 0;
    }
  Simulator::Cancel (m_event);
}

void MapServerDdt::SendMapReply (void)
{

}

void MapServerDdt::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_event.IsExpired ());
  m_socket->Send (p);
}

void MapServerDdt::HandleRead (Ptr<Socket> socket)
{

}

void MapServerDdt::HandleReadFromClient (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {
      uint8_t buf[packet->GetSize ()];
      packet->CopyData (buf, packet->GetSize ());
      std::stringstream ss;

      if (buf[0] == static_cast<uint8_t> (MapRegisterMsg::GetMsgType ()))
        {
          NS_LOG_DEBUG ("Get Map Register!");
          Ptr<MapRegisterMsg> msg = MapRegisterMsg::Deserialize (buf);
          Ptr<MapReplyRecord> record = msg->GetRecord ();
          Ptr<EndpointId> eid;
          Ptr<Locators> locators = record->GetLocators ();
          if (record->GetEidAfi () == LispControlMsg::IP)
            {
              ss << "/" << unsigned (record->GetEidMaskLength ());

              Ipv4Mask mask = Ipv4Mask (ss.str ().c_str ());
              eid = Create<EndpointId> (record->GetEidPrefix (), mask);
              Ptr<MapEntryImpl> mapEntry = Create<MapEntryImpl> ();
              mapEntry->SetLocators (locators);
              mapEntry->SetEidPrefix (eid);
              m_mapTablesv4->SetEntry (record->GetEidPrefix (), mask, mapEntry, MapTables::IN_DATABASE);

            }
          else if (record->GetEidAfi () == LispControlMsg::IPV6)
            {
              ss << "/" << unsigned (record->GetEidMaskLength ());

              eid = Create<EndpointId> (record->GetEidPrefix (), Ipv6Prefix (ss.str ().c_str ()));
              Ipv6Prefix prefix = Ipv6Prefix (ss.str ().c_str ());
              eid = Create<EndpointId> (record->GetEidPrefix (), prefix);
              Ptr<MapEntryImpl> mapEntry = Create<MapEntryImpl> ();
              mapEntry->SetLocators (locators);
              mapEntry->SetEidPrefix (eid);
              m_mapTablesv6->SetEntry (record->GetEidPrefix (), prefix, mapEntry, MapTables::IN_DATABASE);
            }
        }
      else if (buf[0] == static_cast<uint8_t> (MapRequestMsg::GetMsgType ()))
        {
          NS_LOG_DEBUG ("Receive Map request on Map-server!");
          Ptr<MapRequestMsg> requestMsg = MapRequestMsg::Deserialize (buf);
          Ptr<MapRequestRecord> record = requestMsg->GetMapRequestRecord ();
          Ptr<MapEntry> entry;
          if (record->GetAfi () == LispControlMsg::IP)
            {
              entry = m_mapTablesv4->DatabaseLookup (record->GetEidPrefix ());
            }
          else if (record->GetAfi () == LispControlMsg::IPV6)
            {
              entry = m_mapTablesv6->DatabaseLookup (record->GetEidPrefix ());
            }
          if (entry == 0)
            {
              NS_LOG_DEBUG ("Send Negative Map-Reply");
            }
          else
            {
              NS_LOG_DEBUG ("Forward Map-Request to ETR");
              Ptr<Locator> locator = entry->GetLocators ()->SelectFirsValidRloc ();
              NS_LOG_DEBUG ("Forward Map-Request to ETR " << Ipv4Address::ConvertFrom (locator->GetRlocAddress ()));
              MapResolver::ConnectToPeerAddress (locator->GetRlocAddress (), m_peerPort, m_socket);
              Ptr<Packet> reqPacket = Create<Packet> (buf, packet->GetSize ());
              Send (reqPacket);
            }

        }
    }
}

} /* namespace ns3 */
