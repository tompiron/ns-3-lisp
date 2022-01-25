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

#include "lisp-protocol.h"
#include <ns3/address.h>
#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/assert.h>
#include "lisp-header.h"
#include "map-tables.h"
#include "lisp-header.h"
#include "string"
#include "lisp-mapping-socket.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LispProtocol");

// LispProtocol

const uint8_t LispProtocol::PROT_NUMBER = 143; // first non assigned (adhoc number)
const uint16_t LispProtocol::LISP_DATA_PORT = 4341;
const uint16_t LispProtocol::LISP_SIG_PORT = 4342;
const uint16_t LispProtocol::LISP_MAX_RLOC_PRIO = 255;
const uint16_t LispProtocol::MAX_VERSION_NUM = 4095;
const uint16_t LispProtocol::WRAP_VERSION_NUM = 2047;
const uint8_t LispProtocol::NULL_VERSION_NUM = 0;


Ptr<Packet>
LispProtocol::PrependLispHeader (Ptr<Packet> packet,
                                 Ptr<const MapEntry> localMapEntry,
                                 Ptr<const MapEntry> remoteMapEntry,
                                 Ptr<Locator> sourceRloc,
                                 Ptr<Locator> destRloc)
{
  NS_ASSERT (packet);
  LispHeader lispHeader = LispHeader ();

  // Check if txNonce present
  if (destRloc->GetRlocMetrics ()->IsTxNoncePresent ())
    {
      // if yes set N bit
      lispHeader.SetNBit (1);
      // copy nonce
      lispHeader.SetNonce (destRloc->GetRlocMetrics ()->GetTxNonce ());
    }

  // check if local and remote mapping use versioning
  if (localMapEntry->IsUsingVersioning () && remoteMapEntry->IsUsingVersioning ())
    {
      lispHeader.SetVBit (1);
      lispHeader.SetDestMapVersion (remoteMapEntry->GetVersionNumber ());
      lispHeader.SetSrcMapVersion (localMapEntry->GetVersionNumber ());
    }
  else if (localMapEntry->IsUsingLocStatusBits ())
    {
      lispHeader.SetLBit (1);
      lispHeader.SetLSBs (localMapEntry->GetLocsStatusBits ());
    }

  packet->AddHeader (lispHeader);
  NS_LOG_DEBUG ("Lisp Header Added: " << lispHeader);
  return packet;
}


bool
LispProtocol::CheckLispHeader (const LispHeader &header,
                               Ptr<const MapEntry> localMapEntry,
                               Ptr<const MapEntry> remoteMapEntry,
                               Ptr<Locator> srcRloc, Ptr<Locator> destRloc,
                               Ptr<LispOverIp> lispOverIp)
{
  /*
   * TODO Make enum for message types (0, 1 = dest number older, 2 = src older,
   * 3 = the received status bits is different from the ones in the cache
   *
   *
   */
  Ptr<LispStatistics> stats;

  if (Ipv4Address::IsMatchingType (destRloc->GetRlocAddress ()))
    {
      stats = lispOverIp->GetLispStatisticsV4 ();
    }
  else
    {
      stats = lispOverIp->GetLispStatisticsV6 ();
    }

  int msgType = 0;
  bool retValue = 1;

  if (header.GetNBit ())
    {
      // the LISP header contains a nonce
      if (remoteMapEntry && destRloc)
        {
          /*
           * we copy the last received nonce in the locator
           */
          destRloc->GetRlocMetrics ()->SetRxNoncePresent (true);
          destRloc->GetRlocMetrics ()->SetRxNonce (header.GetNonce ());
        }
    }
  else if (header.GetVBit ()) // note: N bit and V bit cannot be set in header
    {
      // See RFC 6834 for more info about Map-version number
      if (localMapEntry->IsUsingVersioning () && header.GetDestMapVersion () != localMapEntry->GetVersionNumber ())
        {
          // TODO Question Why Does OpenLisp mask the version number
          if (IsMapVersionNumberNewer (localMapEntry->GetVersionNumber (), header.GetDestMapVersion ()))
            {
              /*
               * The dest version num is newer than the one of
               * the local map.
               * This should not happen (because system is
               * authoritative on the mapping) TODO Why
               */
              stats->BadDestVersionNumber ();
              retValue = false;
            }
          else
            {
              /*
               * The dest version number is older
               * than the one in the local map.
               * TODO Notify the control plane ! Why
               *
               */
              msgType = LispMappingSocket::MAPM_REMOTESTALE;
            }
        }
      else if (remoteMapEntry && remoteMapEntry->IsUsingVersioning () && header.GetSrcMapVersion () != remoteMapEntry->GetVersionNumber ())
        {
          if (IsMapVersionNumberNewer (remoteMapEntry->GetVersionNumber (), header.GetSrcMapVersion ()))
            {
              /*
               * The src version number is newer than the one in the
               * Cache
               * Notify control plane! Why?
               */

              msgType = LispMappingSocket::MAPM_LOCALSTALE;
            }
          else
            {
              /*
               * The src version number is older than the one in the
               * Cache
               * Todo Erro why ?
               */
              stats->BadSrcVersionNumber ();
              retValue = false;
            }
        }
    }
  if (!msgType)
    {
      if (header.GetLBit ())
        {
          /*
           * If the Loc Status Bit is set and
           * the LSB field is different from the one
           * in the Cache, notify the Control plane
           */

          if (remoteMapEntry && destRloc && header.GetLSBs () != remoteMapEntry->GetLocsStatusBits ())
            {
              msgType = LispMappingSocket::MAPM_LSBITS;
            }
        }
      else if (header.GetIBit ())
        {
          /*
           * Ignore Ibit.
           * Do nothing for now
           * TODO Consider Instance ID later
           */
        }
    }

  if (msgType)
    {
      // TODO notify control plane through socket
    }
  return retValue;
}

uint16_t LispProtocol::GetLispSrcPort (Ptr<const Packet> packet)
{
  return LISP_DATA_PORT;
}

bool LispProtocol::IsMapVersionNumberNewer (uint16_t vnum2, uint16_t vnum1)
{
  if ((vnum2 > vnum1 && (vnum2 - vnum1) < LispProtocol::WRAP_VERSION_NUM)
      || (vnum1 > vnum2 && (vnum1 - vnum2) > LispProtocol::WRAP_VERSION_NUM + 1))
    {
      return true;
    }
  else
    {
      return false;
    }
}

// LispStatistics
TypeId LispStatistics::GetTypeId (void)
{
  // TODO Add attributes
  static TypeId tid = TypeId ("ns3::LispStatistics")
    .SetParent<Object> ()
    .SetGroupName ("Lisp")
    .AddConstructor<LispStatistics> ()
  ;
  return tid;
}

LispStatistics::LispStatistics (void)
  : m_inputPackets (0),
  m_inputDifAfPackets (0),
  m_badSizePackets (0),
  m_noLocMapPresent (0),
  m_badDataLength (0),
  m_badSourceVersionNumber (0),
  m_badDestVesionNumber (0),
  m_outputPackets (0),
  m_outputDifAfPackets (0),
  m_cacheMissPackets (0),
  m_noValidRlocPackets (0),
  m_noValidMtuPackets (0),
  m_noEnoughBufferPacket (0),
  m_outputDropPackets (0)
{
  NS_LOG_FUNCTION (this);
}

LispStatistics::~LispStatistics ()
{

}

void LispStatistics::NoLocalMap (void)
{
  m_noLocMapPresent++;
}
void LispStatistics::BadDestVersionNumber (void)
{
  m_badDestVesionNumber++;
}
void LispStatistics::BadSrcVersionNumber (void)
{
  m_badSourceVersionNumber++;
}
void LispStatistics::IncInputPacket (void)
{
  m_inputPackets++;
}
void LispStatistics::IncBadSizePackets (void)
{
  m_badSizePackets++;
}
void LispStatistics::IncInputDifAfPackets (void)
{
  m_inputDifAfPackets++;
}
void LispStatistics::IncCacheMissPackets (void)
{
  m_cacheMissPackets++;
}
void LispStatistics::IncNoValidRloc (void)
{
  m_noValidRlocPackets++;
}
void LispStatistics::IncOutputDropPackets (void)
{
  m_outputDropPackets++;
}
void LispStatistics::IncNoValidMtuPackets (void)
{
  m_noValidMtuPackets++;
}
void LispStatistics::IncNoEnoughSpace (void)
{
  m_noEnoughBufferPacket++;
}

void LispStatistics::IncOutputPackets (void)
{
  m_outputPackets++;
}
void LispStatistics::IncOutputDifAfPackets (void)
{
  m_outputDifAfPackets++;
}

// RlocMetrics

//NS_OBJECT_ENSURE_REGISTERED (RlocMetrics);
/*TypeId RlocMetrics::GetTypeId (void)
{
  // TODO Add attributes
  static TypeId tid = TypeId ("ns3::RlocMetrics")
    .SetParent<Object> ()
    .SetGroupName ("Lisp")
    .AddConstructor<RlocMetrics> ()
  ;
  return tid;
}*/

RlocMetrics::RlocMetrics ()
  : m_priority (0),
  m_weight (0),
  m_rlocIsUp (true),
  m_rlocIsLocalInterface (false),
  m_txNoncePresent (false),
  m_rxNoncePresent (false),
  m_txNonce (0),
  m_rxNonce (0),
  m_mtu (0)
{

}

RlocMetrics::RlocMetrics (uint8_t priority, uint8_t weight)
{
  m_priority = priority;
  m_weight = weight;
  // by default locator is up
  m_rlocIsUp = true;
  // mtu not set yet
  m_mtu = 0;
}

RlocMetrics::RlocMetrics (uint8_t priority, uint8_t weight, bool reachable)
{
  m_priority = priority;
  m_weight = weight;
  // by default locator is up
  m_rlocIsUp = reachable;
  // default 0 : mtu not set yet
  m_mtu = 0;
}

RlocMetrics::~RlocMetrics ()
{
}

uint8_t RlocMetrics::GetPriority (void) const
{
  return m_priority;
}

void RlocMetrics::SetPriority (uint8_t priority)
{
  NS_ASSERT (priority >= 0 && priority <= LispProtocol::LISP_MAX_RLOC_PRIO);
  m_priority = priority;
}

uint8_t RlocMetrics::GetWeight (void) const
{
  return m_weight;
}

void RlocMetrics::SetWeight (uint8_t weight)
{
  m_weight = weight;
}

uint32_t RlocMetrics::GetMtu (void) const
{
  return m_mtu;
}

void RlocMetrics::SetMtu (uint32_t mtu)
{
  m_mtu = mtu;
}

bool RlocMetrics::IsUp (void)
{
  return m_rlocIsUp;
}

void RlocMetrics::SetUp (bool status)
{
  m_rlocIsUp = true;
}

bool RlocMetrics::IsLocalInterface (void)
{
  return m_rlocIsLocalInterface;
}

bool RlocMetrics::IsRxNoncePresent (void)
{
  return m_rxNoncePresent;
}

void RlocMetrics::SetRxNoncePresent (bool rxNoncePresent)
{
  m_rxNoncePresent = rxNoncePresent;
}

void RlocMetrics::SetIsLocalIf (bool isLocal)
{
  m_rlocIsLocalInterface = isLocal;
}

bool RlocMetrics::IsTxNoncePresent (void)
{
  return m_txNoncePresent;
}

uint32_t RlocMetrics::GetTxNonce (void)
{
  return m_txNonce;
}

void RlocMetrics::SetTxNoncePresent (bool txNoncePresent)
{
  m_txNoncePresent = txNoncePresent;
}

uint32_t RlocMetrics::GetRxNonce (void)
{
  return m_rxNonce;
}
void RlocMetrics::SetRxNonce (uint32_t rxNonce)
{
  m_rxNonce = rxNonce;
}

void RlocMetrics::SetTxNonce (uint32_t txNonce)
{
  m_txNonce = txNonce;
}

std::string RlocMetrics::Print ()
{
  std::stringstream str;

  str << "Priority: " << unsigned(m_priority) << "\t" << "Weight: " << unsigned(m_weight) << "\tRxNonce: " <<
    m_rxNonce << "\tTxNonce: " << m_txNonce << "\tMTU: " << m_mtu << "\tUP: " << unsigned(m_rlocIsUp)
      << "\tLocal If: " << unsigned(m_rlocIsLocalInterface) << "\n";
  return str.str ();
}

uint8_t RlocMetrics::Serialize (uint8_t *buf)
{
  buf[0] = m_priority;
  buf[1] = m_weight;
  uint8_t flags = 0;

  if (IsRxNoncePresent ())
    {
      flags |= RLOCF_RXNONCE;
    }
  if (IsTxNoncePresent ())
    {
      flags |= RLOC_TXNONCE;
    }
  if (IsUp ())
    {
      flags |= RLOCF_UP;
    }
  if (IsLocalInterface ())
    {
      flags |= RLOCF_LIF;
    }

  buf[2] = flags;

  // rx nonce
  buf[3] = (m_rxNonce >> 24) & 0xff;
  buf[4] = (m_rxNonce >> 16) & 0xff;
  buf[5] = (m_rxNonce >> 8) & 0xff;
  buf[6] = (m_rxNonce >> 0) & 0xff;

  // tx nonce
  buf[7] = (m_txNonce >> 24) & 0xff;
  buf[8] = (m_txNonce >> 16) & 0xff;
  buf[9] = (m_txNonce >> 8) & 0xff;
  buf[10] = (m_txNonce >> 0) & 0xff;

  // mtu
  buf[11] = (m_mtu >> 24) & 0xff;
  buf[12] = (m_mtu >> 16) & 0xff;
  buf[13] = (m_mtu >> 8) & 0xff;
  buf[14] = (m_mtu >> 0) & 0xff;

  return 15;
}

Ptr<RlocMetrics> RlocMetrics::Deserialized (const uint8_t *buf)
{
  Ptr<RlocMetrics> rlocMetrics = Create<RlocMetrics> ();
  rlocMetrics->SetPriority (buf[0]);
  rlocMetrics->SetWeight (buf[1]);

  uint8_t flags = buf[2];
  if (flags & RLOCF_RXNONCE)
    {
      rlocMetrics->SetRxNoncePresent (true);
    }
  if (flags & RLOC_TXNONCE)
    {
      rlocMetrics->SetTxNoncePresent (true);
    }
  if (flags & RLOCF_LIF)
    {
      rlocMetrics->SetIsLocalIf (true);
    }
  if (flags & RLOCF_UP)
    {
      rlocMetrics->SetUp (true);
    }
  else
    {
      rlocMetrics->SetUp (false);
    }

  uint32_t rxNonce = 0;
  rxNonce |= buf[3];
  rxNonce <<= 8;
  rxNonce |= buf[4];
  rxNonce <<= 8;
  rxNonce |= buf[5];
  rxNonce <<= 8;
  rxNonce |= buf[6];

  rlocMetrics->SetRxNonce (rxNonce);

  uint32_t txNonce = 0;
  txNonce |= buf[7];
  txNonce <<= 8;
  txNonce |= buf[8];
  txNonce <<= 8;
  txNonce |= buf[9];
  txNonce <<= 8;
  txNonce |= buf[10];

  rlocMetrics->SetTxNonce (txNonce);

  uint32_t mtu = 0;
  mtu |= buf[11];
  mtu <<= 8;
  mtu |= buf[12];
  mtu <<= 8;
  mtu |= buf[13];
  mtu <<= 8;
  mtu |= buf[14];

  rlocMetrics->SetMtu (mtu);

  return rlocMetrics;
}

// Locator
Locator::Locator ()
{
  NS_LOG_FUNCTION (this);
  m_rlocAddress = static_cast<Address> (Ipv4Address ());
}
Locator::Locator (Address rlocAddress)
{
  NS_ASSERT (Ipv4Address::IsMatchingType (rlocAddress)
             || Ipv6Address::IsMatchingType (rlocAddress));

  m_rlocAddress = rlocAddress;
  m_metrics = Create<RlocMetrics> (0, 0, 1);
}

Locator::~Locator ()
{
}

Address Locator::GetRlocAddress (void) const
{
  return m_rlocAddress;
}

void Locator::SetRlocAddress (const Address &rlocAddress)
{
  NS_ASSERT (Ipv4Address::IsMatchingType (rlocAddress)
             || Ipv6Address::IsMatchingType (rlocAddress));
  m_rlocAddress = rlocAddress;
}

Ptr<RlocMetrics> Locator::GetRlocMetrics (void) const
{
  return m_metrics;
}

void Locator::SetRlocMetrics (Ptr<RlocMetrics> metrics)
{
  m_metrics = metrics;
}

std::string Locator::Print (void)
{
  std::string locator = std::string ();
  std::stringstream str;
  if (Ipv4Address::IsMatchingType (m_rlocAddress))
    {
      Ipv4Address::ConvertFrom (m_rlocAddress).Print (str);
      locator += "\tRLOC address: " + str.str () + "\n";
    }
  else if (Ipv6Address::IsMatchingType (m_rlocAddress))
    {
      // TODO do same for Ipv6
      Ipv6Address::ConvertFrom (m_rlocAddress).Print (str);
      locator += "\tRLOC address: " + str.str () + "\n";
    }

  if (m_metrics != 0)
    {
      locator += "\t" + m_metrics->Print ();
    }

  str.str (std::string ());
  return locator;
}

uint8_t Locator::Serialize (uint8_t *buf)
{
  uint8_t size = 0;
  if (Ipv4Address::IsMatchingType (m_rlocAddress))
    {
      buf[0] = 1;
      Ipv4Address::ConvertFrom (m_rlocAddress).Serialize (buf + 1);
      size += 5;
      if (m_metrics)
        {
          buf[5] = 1;
          size +=  m_metrics->Serialize (buf + 6);
        }
      else
        {
          buf[5] = 0;
        }
    }
  else
    {
      buf[0] = 0;
      Ipv6Address::ConvertFrom (m_rlocAddress).Serialize (buf + 1);
      size += 17;
      if (m_metrics)
        {
          buf[17] = 1;
          size += m_metrics->Serialize (buf + 18);
        }
      else
        {
          buf[17] = 0;
        }
    }
  size++; // metrix present indicator
  return size;
}
Ptr<Locator> Locator::Deserialized (const uint8_t *buf)
{
  NS_ASSERT (buf);

  Ptr<Locator> locator = Create<Locator> ();
  if (buf[0])
    {
      locator->SetRlocAddress (static_cast<Address> (Ipv4Address::Deserialize (buf + 1)));
      if (buf[5])
        {
          locator->SetRlocMetrics (RlocMetrics::Deserialized (buf + 6));
        }
    }
  else
    {
      locator->SetRlocAddress (static_cast<Address> (Ipv4Address::Deserialize (buf + 1)));
      if (buf[17])
        {
          locator->SetRlocMetrics (RlocMetrics::Deserialized (buf + 18));
        }
    }
  return locator;
}


// EID
EndpointId::EndpointId ()
{
  m_eidAddress = static_cast<Address> (Ipv4Address ()); // ipv4 by default
  m_mask = Ipv4Mask ();
  m_prefix = Ipv6Prefix ();
}

EndpointId::EndpointId (const Address &eidAddress)
{

  m_eidAddress = eidAddress;
  m_mask = Ipv4Mask ();
  m_prefix = Ipv6Prefix ();
}

EndpointId::EndpointId (const Address &eidAddress, const Ipv4Mask &mask)
{
  NS_ASSERT (Ipv4Address::IsMatchingType (eidAddress));
  m_eidAddress = Ipv4Address::ConvertFrom (eidAddress);
  m_mask = mask;
  m_prefix = Ipv6Prefix ();
}
EndpointId::EndpointId (const Address &eidAddress, const Ipv6Prefix &prefix)
{
  NS_ASSERT (Ipv6Address::IsMatchingType (eidAddress));
  m_eidAddress = eidAddress;
  m_mask = Ipv4Mask ();
  m_prefix = prefix;
}

EndpointId::~EndpointId ()
{

}

void EndpointId::SetEidAddress (const Address &eidAddress)
{
  NS_ASSERT (Ipv4Address::IsMatchingType (eidAddress) || Ipv6Address::IsMatchingType (eidAddress));
  m_eidAddress = eidAddress;
}

Address
EndpointId::GetEidAddress (void) const
{
  return m_eidAddress;
}

void EndpointId::SetIpv4Mask (const Ipv4Mask &mask)
{
  m_mask = mask;
}

Ipv4Mask
EndpointId::GetIpv4Mask (void) const
{
  return m_mask;
}

Ipv6Prefix
EndpointId::GetIpv6Prefix (void) const
{
  return m_prefix;
}

void
EndpointId::SetIpv6Prefix (const Ipv6Prefix &prefix)
{
  m_prefix = prefix;
}

bool EndpointId::IsIpv4 (void) const
{
  return Ipv4Address::IsMatchingType (m_eidAddress);
}

std::string EndpointId::Print (void) const
{
  std::string eid = "EID prefix: ";
  if (IsIpv4 ())
    {
      std::stringstream str;
      Ipv4Address::ConvertFrom (m_eidAddress).Print (str);
      eid += str.str () + "\n";
      str.str (std::string ());
      m_mask.Print (str);
      eid += "ipv4 mask: " + str.str () + "\n\n";
    }
  else
    {
      std::stringstream str;
      Ipv6Address::ConvertFrom (m_eidAddress).Print (str);
      eid += str.str () + "\n";
      str.str (std::string ());
      m_prefix.Print (str);
      eid += "ipv6 prefix: " + str.str () + "\n\n";
    }

  return eid;
}

uint8_t EndpointId::Serialize (uint8_t buf[33]) const
{
  uint8_t size = 0;
  if (IsIpv4 ())
    {
      buf[0] = 1;
      Ipv4Address::ConvertFrom (m_eidAddress).Serialize (buf + 1);
      buf[5] = (m_mask.Get () >> 24) & 0xff;
      buf[6] = (m_mask.Get () >> 16) & 0xff;
      buf[7] = (m_mask.Get () >> 8) & 0xff;
      buf[8] = (m_mask.Get () >> 0) & 0xff;
      size = 9;
    }
  else
    {
      buf[0] = 0;
      Ipv6Address::ConvertFrom (m_eidAddress).Serialize (buf + 1);
      m_prefix.GetBytes (buf + 17);
      size = 33;
    }

  return size;
}

uint8_t EndpointId::GetSerializedSize (void) const
{
  if (IsIpv4 ())
    {
      return 9;
    }
  else
    {
      return 33;
    }
}

Ptr<EndpointId> EndpointId::Deserialize (const uint8_t *buf)
{
  NS_ASSERT (buf);

  Ptr<EndpointId> endpointId = 0;
  if (buf[0])
    {
      Address eidAddress = static_cast<Address> (Ipv4Address::Deserialize (buf + 1));
      uint32_t mask = 0;
      mask |= buf[5];
      mask <<= 8;
      mask |= buf[6];
      mask <<= 8;
      mask |= buf[7];
      mask <<= 8;
      mask |= buf[8];

      Ipv4Mask ipv4Mask = Ipv4Mask (mask);
      endpointId = Create<EndpointId> (eidAddress, ipv4Mask);
    }
  else
    {
      Address eidAddress = static_cast<Address> (Ipv6Address::Deserialize (buf + 1));
      Ipv6Prefix prefix = Ipv6Prefix ((uint8_t *) buf + 17);
      endpointId = Create<EndpointId> (eidAddress, prefix);
    }
  return endpointId;
}

} /* namespace ns3 */
