/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Sindhuja Venkatesh
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
 * Authors: Sindhuja Venkatesh <intutivestriker88@gmail.com>
 */
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ipv4-netfilter.h"

#include "ip-conntrack-info.h"
#include "ipv4-conntrack-l3-protocol.h"
#include "tcp-conntrack-l4-protocol.h"
#include "udp-conntrack-l4-protocol.h"
#include "icmpv4-conntrack-l4-protocol.h"

#include "tcp-header.h"
#include "udp-header.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"
#include "ipv4-nat.h"
#include "ipv4.h"

#include <iomanip>

NS_LOG_COMPONENT_DEFINE ("Ipv4Nat");

namespace ns3 {

Ipv4NetfilterHook natCallback1;
Ipv4NetfilterHook natCallback2;

NS_OBJECT_ENSURE_REGISTERED (Ipv4Nat);

TypeId
Ipv4Nat::GetTypeId (void)
{
  static TypeId tId = TypeId ("ns3::Ipv4Nat")
    .SetParent<Object> ()
  ;

  return tId;
}

Ipv4Nat::Ipv4Nat ()
  : m_insideInterface (-1),
  m_outsideInterface (-1)
{
  NS_LOG_FUNCTION (this);

  NetfilterHookCallback doNatPreRouting = MakeCallback (&Ipv4Nat::DoNatPreRouting, this);
  NetfilterHookCallback doNatPostRouting = MakeCallback (&Ipv4Nat::DoNatPostRouting, this);

  natCallback1 = Ipv4NetfilterHook (1, NF_INET_POST_ROUTING, NF_IP_PRI_NAT_SRC, doNatPostRouting);
  natCallback2 = Ipv4NetfilterHook (1, NF_INET_PRE_ROUTING, NF_IP_PRI_NAT_DST, doNatPreRouting);

}

/*
 * This method is called by AddAgregate and completes the aggregation
 * by hooking to the Ipv4Netfilter
 */
void
Ipv4Nat::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION (this);
  if (m_ipv4 != 0)
    {
      return;
    }
  Ptr<Node> node = this->GetObject<Node> ();
  if (node != 0)
    {
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      if (ipv4 != 0)
        {
          Ptr<Ipv4Netfilter> netfilter = ipv4->GetNetfilter ();
          if (ipv4 != 0)
            {
              m_ipv4 = ipv4;
              // Set callbacks on netfilter pointer

              netfilter->RegisterHook (natCallback1);
              netfilter->RegisterHook (natCallback2);

            }
        }
    }
  Object::NotifyNewAggregate ();
}

uint32_t
Ipv4Nat::GetNStaticRules (void) const
{
  NS_LOG_FUNCTION (this);
  return m_statictable.size ();
}

Ipv4StaticNatRule
Ipv4Nat::GetStaticRule (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  uint32_t tmp = 0;
  for (StaticNatRules::const_iterator i = m_statictable.begin ();
       i != m_statictable.end ();
       i++)
    {
      if (tmp == index)
        {
          return *i;
        }
      tmp++;
    }
  NS_ASSERT (false);

  return Ipv4StaticNatRule (Ipv4Address (), Ipv4Address ());
}

Ipv4DynamicNatRule
Ipv4Nat::GetDynamicRule (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  uint32_t tmp = 0;
  for (DynamicNatRules::const_iterator i = m_dynamictable.begin ();
       i != m_dynamictable.end ();
       i++)
    {
      if (tmp == index)
        {
          return *i;
        }
      tmp++;
    }
  NS_ASSERT (false);

  return Ipv4DynamicNatRule (Ipv4Address (), Ipv4Mask ());
}

Ipv4DynamicNatTuple
Ipv4Nat::GetDynamicTuple (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  uint32_t tmp = 0;
  for (DynamicNatTuple::const_iterator i = m_dynatuple.begin ();
       i != m_dynatuple.end ();
       i++)
    {
      if (tmp == index)
        {
          return *i;
        }
      tmp++;
    }
  NS_ASSERT (false);

  return Ipv4DynamicNatTuple (Ipv4Address (), Ipv4Address (), uint16_t ('0'), uint16_t ('0'));
}


uint32_t
Ipv4Nat::GetNDynamicRules (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dynamictable.size ();
}

uint32_t
Ipv4Nat::GetNDynamicTuples (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dynatuple.size ();
}

void
Ipv4Nat::RemoveStaticRule (uint32_t index)
{

  NS_LOG_FUNCTION (this << index);
  NS_ASSERT (index < m_statictable.size ());
  uint32_t tmp = 0;
  for (StaticNatRules::iterator i = m_statictable.begin ();
       i != m_statictable.end (); i++, tmp++)
    {
      if (tmp == index)
        {
          m_statictable.erase (i);
          return;
        }
    }
  NS_ASSERT_MSG (false, "Rule Not Found");
}

void
Ipv4Nat::RemoveDynamicRule (uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  NS_ASSERT (index < m_dynamictable.size ());
  uint32_t tmp = 0;
  for (DynamicNatRules::iterator i = m_dynamictable.begin ();
       i != m_dynamictable.end (); i++, tmp++)
    {
      if (tmp == index)
        {
          m_dynamictable.erase (i);
          return;
        }
    }
  NS_ASSERT_MSG (false, "Rule Not Found");
}


/**
 * \brief Print the NAT translation table
 *
 * \param stream the ostream the NAT table is printed to
 */

void
Ipv4Nat::PrintTable (Ptr<OutputStreamWrapper> stream) const

{
  NS_LOG_FUNCTION (this);
  std::ostream* os = stream->GetStream ();
  if (GetNStaticRules () > 0)
    {
      *os << "       Static Nat Rules" << std::endl;
      *os << "Local IP     Local Port     Global IP    Global Port " << std::endl;
      for (uint32_t i = 0; i < GetNStaticRules (); i++)
        {
          std::ostringstream locip,gloip,locprt,gloprt;
          Ipv4StaticNatRule rule = GetStaticRule (i);

          if (rule.GetLocalPort ())
            {
              locip << rule.GetLocalIp ();
              *os << std::setiosflags (std::ios::left) << std::setw (16) << locip.str ();

              locprt << rule.GetLocalPort ();
              *os << std::setiosflags (std::ios::left) << std::setw (16) << locprt.str ();
            }

          else
            {
              locip << rule.GetLocalIp ();
              *os << std::setiosflags (std::ios::left) << std::setw (35) << locip.str ();
            }


          gloip << rule.GetGlobalIp ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << gloip.str ();

          if (rule.GetGlobalPort ())
            {
              gloprt << rule.GetGlobalPort ();
              *os << std::setiosflags (std::ios::left) << std::setw (16) << gloprt.str ();
            }

          *os << std::endl;

        }
    }

  if (GetNDynamicRules () > 0)
    {
      *os << std::endl;
      *os << "       Dynamic Nat Rules" << std::endl;
      *os << "Local Network          Local Netmask" << std::endl;
      for (uint32_t i = 0; i < GetNDynamicRules (); i++)
        {
          std::ostringstream locnet,locmask,gloip,glomask,strtprt,endprt;
          Ipv4DynamicNatRule rule = GetDynamicRule (i);

          locnet << rule.GetLocalNet ();
          *os << std::setiosflags (std::ios::left) << std::setw (32) << locnet.str ();

          locmask << rule.GetLocalMask ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << locmask.str ();
        }

      *os << std::endl;

    }

  if (GetNDynamicTuples () > 0)
    {
      *os << std::endl;
      *os << "       Current Dynamic Translations" << std::endl;
      *os << "Local IP             Global IP           Local port            Global Port" << std::endl;
      for (uint32_t i = 0; i < GetNDynamicTuples (); i++)
        {
          std::ostringstream locip,gloip,locprt,gloprt;
          Ipv4DynamicNatTuple tup = GetDynamicTuple (i);

          locip << tup.GetLocalAddress ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << locip.str ();

          gloip << tup.GetGlobalAddress ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << gloip.str ();

          locprt << tup.GetLocalPort ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << locprt.str ();

          gloprt << tup.GetGlobalPort ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << gloprt.str ();

          *os << std::endl;
        }

      *os << std::endl;
    }
}

uint32_t
Ipv4Nat::DoNatPreRouting (Hooks_t hookNumber, Ptr<Packet> p,
                          Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb)
{
  NS_LOG_FUNCTION (this << p << hookNumber << in << out);

  if (m_ipv4 == 0)
    {
      return NF_DROP;
    }

  Ipv4Header ipHeader;

  NS_LOG_DEBUG ("Input device " << m_ipv4->GetInterfaceForDevice (in) << " inside interface " << m_insideInterface);
  NS_LOG_DEBUG ("Output device " << m_ipv4->GetInterfaceForDevice (out) << " outside interface " << m_outsideInterface);
  p->RemoveHeader (ipHeader);
  if (m_ipv4->GetInterfaceForDevice (in) == m_outsideInterface)
    {
      // outside interface is the input interface, we NAT the destination addr
      // so that the NAT does not try to locally deliver the packet
      NS_LOG_DEBUG ("PREROUTING: evaluating packet with src " << ipHeader.GetSource () << " dst " << ipHeader.GetDestination ());
      Ipv4Address destAddress = ipHeader.GetDestination ();

      /*----------------------------------
        Checking for Static NAT Rules
        ----------------------------------*/
      for (StaticNatRules::const_iterator i = m_statictable.begin ();
           i != m_statictable.end (); i++)
        {
          NS_LOG_DEBUG ("Static NAT rule");
          if (destAddress != (*i).GetGlobalIp ())
            {
              NS_LOG_DEBUG ("Skipping rule with global IP " << (*i).GetGlobalIp ());
              continue;
            }
          if ((*i).GetGlobalPort () == 0)
            {
              NS_LOG_DEBUG ("Rule match with a non-port-specific rule");
              ipHeader.SetDestination ((*i).GetLocalIp ());
              p->AddHeader (ipHeader);
              return NF_ACCEPT;
            }
          else
            {
              NS_LOG_DEBUG ("evaluating rule with local port " << (*i).GetLocalPort () << " global port " << (*i).GetGlobalPort ());

              if (ipHeader.GetProtocol () == MY_IPPROTO_TCP
                  && (((*i).GetProtocol () == MY_IPPROTO_TCP)
                      || (*i).GetProtocol () == 0))
                {
                  TcpHeader tcpHeader;
                  p->RemoveHeader (tcpHeader);
                  if (tcpHeader.GetDestinationPort () == (*i).GetGlobalPort ())
                    {
                      tcpHeader.SetDestinationPort ((*i).GetLocalPort ());
                      p->AddHeader (tcpHeader);
                      ipHeader.SetDestination ((*i).GetLocalIp ());
                      p->AddHeader (ipHeader);
                      return NF_ACCEPT;
                    }
                  p->AddHeader (tcpHeader);
                }
              else if (ipHeader.GetProtocol () == MY_IPPROTO_UDP
                       && (((*i).GetProtocol () == MY_IPPROTO_UDP)
                           || (*i).GetProtocol () == 0))
                {
                  UdpHeader udpHeader;
                  p->RemoveHeader (udpHeader);
                  if (udpHeader.GetDestinationPort () == (*i).GetGlobalPort ())
                    {
                      udpHeader.SetDestinationPort ((*i).GetLocalPort ());
                      p->AddHeader (udpHeader);
                      ipHeader.SetDestination ((*i).GetLocalIp ());
                      p->AddHeader (ipHeader);
                      return NF_ACCEPT;
                    }
                  p->AddHeader (udpHeader);
                }
            }
        }

      /*----------------------------------
        Checking for Dynamix NAT Rules
        ----------------------------------*/
      if (destAddress == GetAddressPoolIp ())
        {
          NS_LOG_DEBUG ("Packet targeted for NATed prefix");
          // Checking if ongoing connection
          for (DynamicNatTuple::const_iterator i = m_dynatuple.begin ();
               i != m_dynatuple.end (); i++)
            {
              NS_LOG_DEBUG ("Global port of rule: " << (*i).GetGlobalPort ());
              if (ipHeader.GetProtocol () == MY_IPPROTO_TCP)
                {
                  TcpHeader tcpHeader;
                  p->RemoveHeader (tcpHeader);
                  if (tcpHeader.GetDestinationPort () == (*i).GetGlobalPort ()) //Rule MATCH
                    {
                      ipHeader.SetDestination ((*i).GetLocalAddress ());
                      tcpHeader.SetDestinationPort ((*i).GetLocalPort ());
                      p->AddHeader (tcpHeader);
                      p->AddHeader (ipHeader);
                      return NF_ACCEPT;
                    }
                  p->AddHeader (tcpHeader);
                }
              if (ipHeader.GetProtocol () == MY_IPPROTO_UDP)
                {
                  UdpHeader udpHeader;
                  p->RemoveHeader (udpHeader);
                  NS_LOG_DEBUG ("Port in udp header: " << udpHeader.GetDestinationPort ());
                  if (udpHeader.GetDestinationPort () == (*i).GetGlobalPort ()) //Rule MATCH
                    {
                      ipHeader.SetDestination ((*i).GetLocalAddress ());
                      udpHeader.SetDestinationPort ((*i).GetLocalPort ());
                      p->AddHeader (udpHeader);
                      p->AddHeader (ipHeader);
                      return NF_ACCEPT;
                    }
                  p->AddHeader (udpHeader);
                }
            }
        }

      /*--------------------------------------
        No match (neither dynamic nor static)
        --------------------------------------*/
      NS_LOG_DEBUG ("NAT drops incoming packet");
      p->AddHeader (ipHeader);
      return NF_DROP;

    }
  p->AddHeader (ipHeader);
  return NF_ACCEPT;
}

uint32_t
Ipv4Nat::DoNatPostRouting (Hooks_t hookNumber, Ptr<Packet> p,
                           Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb)
{
  NS_LOG_FUNCTION (this << p << hookNumber << in << out);

  if (m_ipv4 == 0)
    {
      return NF_DROP;
    }

  Ipv4Header ipHeader;

  NS_LOG_DEBUG ("Input device " << m_ipv4->GetInterfaceForDevice (in) << " inside interface " << m_insideInterface);
  NS_LOG_DEBUG ("Output device " << m_ipv4->GetInterfaceForDevice (out) << " outside interface " << m_outsideInterface);
  p->RemoveHeader (ipHeader);
  if (m_ipv4->GetInterfaceForDevice (out) == m_outsideInterface)
    {
      // matching output interface, consider whether to NAT the source
      // address and port
      NS_LOG_DEBUG ("POSTROUTING: evaluating packet with src " << ipHeader.GetSource () << " dst " << ipHeader.GetDestination ());
      Ipv4Address srcAddress = ipHeader.GetSource ();


      //Checking for Static NAT Rules
      for (StaticNatRules::const_iterator i = m_statictable.begin ();
           i != m_statictable.end (); i++)
        {
          if (srcAddress != (*i).GetLocalIp ())
            {
              NS_LOG_DEBUG ("Skipping rule with local IP " << (*i).GetLocalIp ());
              continue;
            }
          if ((*i).GetLocalPort () == 0)
            {
              NS_LOG_DEBUG ("Rule match with a non-port-specific rule");
              ipHeader.SetSource ((*i).GetGlobalIp ());
              p->AddHeader (ipHeader);
              return NF_ACCEPT;
            }
          else
            {
              NS_LOG_DEBUG ("Evaluating rule with local port " << (*i).GetLocalPort () << " global port " << (*i).GetGlobalPort ());
              if (ipHeader.GetProtocol () == MY_IPPROTO_TCP
                  && (((*i).GetProtocol () == MY_IPPROTO_TCP)
                      || (*i).GetProtocol () == 0))
                {
                  TcpHeader tcpHeader;
                  p->RemoveHeader (tcpHeader);
                  if (tcpHeader.GetSourcePort () == (*i).GetLocalPort ())
                    {
                      tcpHeader.SetSourcePort ((*i).GetGlobalPort ());
                    }
                  p->AddHeader (tcpHeader);
                }
              else if (ipHeader.GetProtocol () == MY_IPPROTO_UDP
                       && (((*i).GetProtocol () == MY_IPPROTO_UDP)
                           || (*i).GetProtocol () == 0))
                {
                  UdpHeader udpHeader;
                  p->RemoveHeader (udpHeader);
                  if (udpHeader.GetSourcePort () == (*i).GetLocalPort ())
                    {
                      udpHeader.SetSourcePort ((*i).GetGlobalPort ());
                    }
                  p->AddHeader (udpHeader);
                }
              NS_LOG_DEBUG ("Rule match with a port-specific rule");
              ipHeader.SetSource ((*i).GetGlobalIp ());
              p->AddHeader (ipHeader);
              return NF_ACCEPT;
            }
        }

      //Checking for Dynamic NAT Rules

      //Checking for existing connection
      for (DynamicNatTuple::const_iterator i = m_dynatuple.begin ();
           i != m_dynatuple.end (); i++)
        {
          if (srcAddress == (*i).GetLocalAddress ())
            {
              if (ipHeader.GetProtocol () == MY_IPPROTO_TCP)
                {
                  TcpHeader tcpHeader;
                  p->RemoveHeader (tcpHeader);
                  if (tcpHeader.GetSourcePort () == (*i).GetLocalPort ())
                    {
                      ipHeader.SetSource ((*i).GetGlobalAddress ());
                      tcpHeader.SetSourcePort ((*i).GetGlobalPort ());
                      p->AddHeader (tcpHeader);
                      p->AddHeader (ipHeader);
                      return NF_ACCEPT;
                    }
                  p->AddHeader (tcpHeader);
                }
              else
                {
                  UdpHeader udpHeader;
                  p->RemoveHeader (udpHeader);
                  if (udpHeader.GetSourcePort () == (*i).GetLocalPort ())
                    {
                      ipHeader.SetSource ((*i).GetGlobalAddress ());
                      udpHeader.SetSourcePort ((*i).GetGlobalPort ());
                      p->AddHeader (udpHeader);
                      p->AddHeader (ipHeader);
                      return NF_ACCEPT;
                    }
                  p->AddHeader (udpHeader);
                }

            }
        }

      //This is for the new connections
      for (DynamicNatRules::const_iterator i = m_dynamictable.begin ();
           i != m_dynamictable.end (); i++)
        {
          NS_LOG_DEBUG ("New stream through NAT");
          // Source address belong to the network mask that must be translated by NAT
          if ((*i).GetLocalNet ().CombineMask ((*i).GetLocalMask ()) == srcAddress.CombineMask ((*i).GetLocalMask ()))
            {
              ipHeader.SetSource (GetAddressPoolIp ());

              if (ipHeader.GetProtocol () == MY_IPPROTO_TCP)

                {
                  NS_LOG_DEBUG ("Changing TCP port");
                  TcpHeader tcpHeader;
                  p->RemoveHeader (tcpHeader);

                  Ipv4DynamicNatTuple natuple
                    (srcAddress,
                    GetAddressPoolIp (),
                    tcpHeader.GetSourcePort (),
                    GetNewOutsidePort ());

                  tcpHeader.SetSourcePort (GetCurrentPort ());

                  p->AddHeader (tcpHeader);
                  m_dynatuple.push_front (natuple);
                }
              else
                {
                  NS_LOG_DEBUG ("Changing UDP port");
                  UdpHeader udpHeader;
                  p->RemoveHeader (udpHeader);

                  Ipv4DynamicNatTuple natuple
                    (srcAddress,
                    GetAddressPoolIp (),
                    udpHeader.GetSourcePort (),
                    GetNewOutsidePort ());

                  udpHeader.SetSourcePort (GetCurrentPort ());

                  p->AddHeader (udpHeader);
                  m_dynatuple.push_front (natuple);
                }

              p->AddHeader (ipHeader);
              return NF_ACCEPT;
            }

        }
    }

  /* No match with any rule to go out -> Leave packet unmodified and accept it */
  p->AddHeader (ipHeader);
  NS_LOG_DEBUG ("END OF POST_ROUTING >>>> src is " << ipHeader.GetSource () << "and destination is " << ipHeader.GetDestination ());
  return NF_ACCEPT;
}

void
Ipv4Nat::AddAddressPool (Ipv4Address globalip, Ipv4Mask globalmask)
{
  NS_LOG_FUNCTION (this << globalip << globalmask);
  m_globalip = globalip;
  m_globalmask = globalmask;
}

Ipv4Address
Ipv4Nat::GetAddressPoolIp () const
{
  return m_globalip;
}

Ipv4Mask
Ipv4Nat::GetAddressPoolMask () const
{
  return m_globalmask;
}

void
Ipv4Nat::AddPortPool (uint16_t strtprt, uint16_t endprt)         //port range
{
  NS_LOG_FUNCTION (this << strtprt << endprt);
  m_startport = strtprt;
  m_endport = endprt;
  m_currentPort = strtprt - 1;
}

uint16_t
Ipv4Nat::GetStartPort () const
{
  return m_startport;
}

uint16_t
Ipv4Nat::GetEndPort () const
{
  return m_endport;
}

uint16_t
Ipv4Nat::GetCurrentPort () const
{
  return m_currentPort;
}

uint16_t
Ipv4Nat::GetNewOutsidePort ()
{
  for (int i = m_startport - 1; i <= m_endport; i++)
    {
      if ( m_currentPort == i)
        {
          m_currentPort++;
          return m_currentPort;

        }
    }
  return 0;
}

void
Ipv4Nat::SetInside (int32_t interfaceIndex)
{
  NS_LOG_FUNCTION (this << interfaceIndex);
  m_insideInterface = interfaceIndex;

}

void
Ipv4Nat::SetOutside (int32_t interfaceIndex)
{

  NS_LOG_FUNCTION (this << interfaceIndex);
  m_outsideInterface = interfaceIndex;
}


void
Ipv4Nat::AddDynamicRule (const Ipv4DynamicNatRule& rule)
{
  NS_LOG_FUNCTION (this);
  m_dynamictable.push_front (rule);
}


void
Ipv4Nat::AddStaticRule (const Ipv4StaticNatRule& rule)
{
  NS_LOG_FUNCTION (this);
  m_statictable.push_front (rule);
  NS_LOG_DEBUG ("list has " << m_statictable.size () << " elements after pushing");
  NS_ASSERT_MSG (m_ipv4, "Forgot to aggregate Ipv4Nat to Node");
  if (m_ipv4->GetInterfaceForAddress (rule.GetGlobalIp ()) != -1)
    {
      NS_LOG_WARN ("Adding node's own IP address as the global NAT address");
      return;
    }
  NS_ASSERT_MSG (m_outsideInterface > -1, "Forgot to assign outside interface");
  // Add address to outside interface so that node will proxy ARP for it
  Ipv4Mask outsideMask = m_ipv4->GetAddress (m_outsideInterface, 0).GetMask ();
  Ipv4InterfaceAddress natAddress (rule.GetGlobalIp (), outsideMask);
  m_ipv4->AddAddress (m_outsideInterface, natAddress);
}

Ipv4StaticNatRule::Ipv4StaticNatRule (Ipv4Address localip, uint16_t locprt, Ipv4Address globalip,uint16_t gloprt, uint16_t protocol)
{
  NS_LOG_FUNCTION (this << localip << locprt << globalip << gloprt << protocol);
  m_localaddr = localip;
  m_globaladdr = globalip;
  m_localport = locprt;
  m_globalport = gloprt;
  NS_ASSERT (protocol == 0 || protocol == MY_IPPROTO_TCP || protocol == MY_IPPROTO_UDP);
  m_protocol = protocol;

}

// This version is used for no port restrictions
Ipv4StaticNatRule::Ipv4StaticNatRule (Ipv4Address localip, Ipv4Address globalip)
{
  NS_LOG_FUNCTION (this << localip << globalip);
  m_localaddr = localip;
  m_globaladdr = globalip;
  m_localport = 0;
  m_globalport = 0;
  m_protocol = 0;
}

Ipv4Address
Ipv4StaticNatRule::GetLocalIp () const
{
  return m_localaddr;
}

Ipv4Address
Ipv4StaticNatRule::GetGlobalIp () const
{
  return m_globaladdr;
}

uint16_t
Ipv4StaticNatRule::GetLocalPort () const
{
  return m_localport;
}

uint16_t
Ipv4StaticNatRule::GetGlobalPort () const
{
  return m_globalport;
}

uint16_t
Ipv4StaticNatRule::GetProtocol () const
{
  return m_protocol;
}

Ipv4DynamicNatRule::Ipv4DynamicNatRule (Ipv4Address localnet, Ipv4Mask localmask)
{
  NS_LOG_FUNCTION (this << localnet << localmask);
  m_localnetwork = localnet;
  m_localmask = localmask;

}

Ipv4Address
Ipv4DynamicNatRule::GetLocalNet () const
{
  return m_localnetwork;
}

Ipv4Mask
Ipv4DynamicNatRule::GetLocalMask () const
{
  return m_localmask;
}

Ipv4DynamicNatTuple::Ipv4DynamicNatTuple (Ipv4Address local, Ipv4Address global, uint16_t localport, uint16_t globalport)
{
  NS_LOG_FUNCTION (this << local << global << localport << globalport);
  m_localip = local;
  m_globalip = global;
  m_localport = localport;
  m_globalport = globalport;
}

Ipv4Address
Ipv4DynamicNatTuple::GetLocalAddress () const
{
  return m_localip;
}

Ipv4Address
Ipv4DynamicNatTuple::GetGlobalAddress () const
{
  return m_globalip;
}

uint16_t
Ipv4DynamicNatTuple::GetGlobalPort () const
{
  return m_globalport;
}

uint16_t
Ipv4DynamicNatTuple::GetLocalPort () const
{
  return m_localport;
}

}
