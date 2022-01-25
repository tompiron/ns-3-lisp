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
#include "map-request-msg.h"

namespace ns3 {

const LispControlMsg::LispControlMsgType msgType = LispControlMsg::MAP_REQUEST;

MapRequestMsg::MapRequestMsg () :
  m_A (0),
  m_M (0),
  m_P (0),
  m_S (0),
  m_p (0),
  m_s (0),
  m_reserved (0),
  m_irc (0),
  m_recordCount (1),
  m_nonce (0)
{
  m_sourceEidAfi = LispControlMsg::IP;
  m_sourceEidAddress = static_cast<Address> (Ipv4Address ());
  m_itrRlocAddrIp = static_cast<Address> (Ipv4Address ());
  m_itrRlocAddrIpv6 = static_cast<Address> (Ipv6Address ());
  m_mapReqRec = 0;
}

MapRequestMsg::~MapRequestMsg ()
{
  // TODO Auto-generated destructor stub
}

void MapRequestMsg::SetA (uint8_t a)
{
  m_A = a;
}

uint8_t MapRequestMsg::GetA (void)
{
  return m_A;
}

void MapRequestMsg::SetM (uint8_t m)
{
  m_M = m;
}
uint8_t MapRequestMsg::GetM (void)
{
  return m_M;
}

void MapRequestMsg::SetP (uint8_t p)
{
  m_P = p;
}
uint8_t MapRequestMsg::GetP (void)
{
  return m_P;
}

void MapRequestMsg::SetS (uint8_t s)
{
  m_S = s;
}
uint8_t MapRequestMsg::GetS (void)
{
  return m_S;
}

void MapRequestMsg::SetP2 (uint8_t p)
{
  m_p = p;
}
uint8_t MapRequestMsg::GetP2 (void)
{
  return m_p;
}

void MapRequestMsg::SetS2 (uint8_t s)
{
  m_s = s;
}
uint8_t MapRequestMsg::GetS2 (void)
{
  return m_s;
}

void MapRequestMsg::SetIrc (uint8_t irc)
{
  m_irc = irc;
}
uint8_t MapRequestMsg::GetIrc (void)
{
  return m_irc;
}

void MapRequestMsg::SetRecordCount (uint8_t recCount)
{
  m_recordCount = recCount;
}
uint8_t MapRequestMsg::GetRecordCount (void)
{
  return m_recordCount;
}

void MapRequestMsg::SetNonce (uint64_t nonce)
{
  m_nonce = nonce;
}
uint64_t MapRequestMsg::GetNonce (void)
{
  return m_nonce;
}

void MapRequestMsg::SetSourceEidAfi (AddressFamily afi)
{
  m_sourceEidAfi = afi;
}

LispControlMsg::AddressFamily MapRequestMsg::GetSourceEidAfi (void)
{
  return m_sourceEidAfi;
}

void MapRequestMsg::SetSourceEidAddr (Address sourceEid)
{
  m_sourceEidAddress = sourceEid;
}
Address MapRequestMsg::GetSourceEidAddr (void)
{
  return m_sourceEidAddress;
}

void MapRequestMsg::SetItrRlocAddrIp (Address itrRlocAddr)
{
  m_itrRlocAddrIp = itrRlocAddr;
}
Address MapRequestMsg::GetItrRlocAddrIp (void)
{
  return m_itrRlocAddrIp;
}

void MapRequestMsg::SetItrRlocAddrIpv6 (Address itrRlocAddr)
{
  m_itrRlocAddrIpv6 = itrRlocAddr;
}
Address MapRequestMsg::GetItrRlocAddrIpv6 (void)
{
  return m_itrRlocAddrIpv6;
}

void MapRequestMsg::SetMapRequestRecord (Ptr<MapRequestRecord> record)
{
  m_mapReqRec = record;
}

Ptr<MapRequestRecord> MapRequestMsg::GetMapRequestRecord (void)
{
  return m_mapReqRec;
}

void MapRequestMsg::Serialize (uint8_t *buf) const
{
  uint8_t AMPSpsres = 0;

  AMPSpsres = (m_A << 7) | (m_M << 6) | (m_P << 5) | (m_S << 4) | (m_p << 3) | (m_s << 2) | (m_reserved);

  buf[0] = static_cast<uint8_t> (LispControlMsg::MAP_REQUEST);
  buf[1] = AMPSpsres;
  buf[2] = m_irc;
  buf[3] = m_recordCount;

  buf[4] = (m_nonce >> 56) & 0xffff;
  buf[5] = (m_nonce >> 48) & 0xffff;
  buf[6] = (m_nonce >> 40) & 0xffff;
  buf[7] = (m_nonce >> 32) & 0xffff;
  buf[8] = (m_nonce >> 24) & 0xffff;
  buf[9] = (m_nonce >> 16) & 0xffff;
  buf[10] = (m_nonce >> 8) & 0xffff;
  buf[11] = (m_nonce >> 0) & 0xffff;

  buf[12] = static_cast<uint8_t> (m_sourceEidAfi);

  uint8_t size = 13;
  if (m_sourceEidAfi == LispControlMsg::IP)
    {
      Ipv4Address::ConvertFrom (m_sourceEidAddress).Serialize (buf + size);
      size += 4;
    }
  else if (m_sourceEidAfi == LispControlMsg::IPV6)
    {
      Ipv6Address::ConvertFrom (m_sourceEidAddress).Serialize (buf + size);
      size += 16;
    }

  if (!Ipv4Address ().IsEqual (Ipv4Address::ConvertFrom (m_itrRlocAddrIp)))
    {
      buf[size] = 1;
      size++;
      Ipv4Address::ConvertFrom (m_itrRlocAddrIp).Serialize (buf + size);
      size += 4;
    }
  else
    {
      buf[size] = 0;
      size++;
    }


  if (!Ipv6Address::ConvertFrom (m_itrRlocAddrIpv6).IsEqual (Ipv6Address ()))
    {
      buf[size] = 1;
      size++;
      Ipv6Address::ConvertFrom (m_itrRlocAddrIpv6).Serialize (buf + size);
      size += 16;
    }
  else
    {
      buf[size] = 0;
      size++;
    }

  m_mapReqRec->Serialize (buf + size);
}

Ptr<MapRequestMsg> MapRequestMsg::Deserialize (uint8_t *buf)
{
  Ptr<MapRequestMsg> msg = Create<MapRequestMsg> ();

  uint8_t AMPSpsres = buf[1];
  msg->SetA (AMPSpsres >> 7);
  msg->SetM (AMPSpsres >> 6);
  msg->SetP (AMPSpsres >> 5);
  msg->SetS (AMPSpsres >> 4);
  msg->SetP2 (AMPSpsres >> 3);
  msg->SetS2 (AMPSpsres >> 2);

  msg->SetIrc (buf[2]);
  msg->SetRecordCount (buf[3]);

  uint64_t nonce = 0;
  nonce |= buf[4];
  nonce <<= 8;
  nonce |= buf[5];
  nonce <<= 8;
  nonce |= buf[6];
  nonce <<= 8;
  nonce |= buf[7];
  nonce <<= 8;
  nonce |= buf[8];
  nonce <<= 8;
  nonce |= buf[9];
  nonce <<= 8;
  nonce |= buf[10];
  nonce <<= 8;
  nonce |= buf[11];

  msg->SetNonce (nonce);

  msg->SetSourceEidAfi (static_cast<LispControlMsg::AddressFamily> (buf[12]));

  uint8_t size = 13;

  if (msg->GetSourceEidAfi () == LispControlMsg::IP)
    {
      msg->SetSourceEidAddr (static_cast<Address> (Ipv4Address::Deserialize (buf + size)));
      size += 4;
    }
  else
    {
      msg->SetSourceEidAddr (static_cast<Address> (Ipv6Address::Deserialize (buf + size)));
      size += 16;
    }

  if (buf[size] == 1)
    {
      size++;
      msg->SetItrRlocAddrIp (static_cast<Address> (Ipv4Address::Deserialize (buf + size)));
      size += 4;
    }
  else
    {
      msg->SetItrRlocAddrIp (static_cast<Address> (Ipv4Address ()));
      size++;
    }

  if (buf[size] == 1)
    {
      size++;
      msg->SetItrRlocAddrIpv6 (static_cast<Address> (Ipv6Address::Deserialize (buf + size)));
      size += 16;
    }
  else
    {
      msg->SetItrRlocAddrIpv6 (static_cast<Address> (Ipv6Address ()));
      size++;
    }

  msg->SetMapRequestRecord (MapRequestRecord::Deserialize (buf + size));
  return msg;
}

void MapRequestMsg::Print (std::ostream& os) const
{
  os << "A " << unsigned(m_A) << " "
     << "M " << unsigned(m_M) << " "
     << "P " << unsigned(m_P) << " "
     << "S " << unsigned(m_S) << " "
     << "p " << unsigned(m_p) << " "
     << "s " << unsigned(m_s) << " "
     << "IRC " << unsigned(m_irc) << " "
     << "Record Count " << unsigned(m_recordCount) << " "
     << "Nonce " << unsigned(m_nonce) << " "
     << "EID AFI " << unsigned(static_cast<int> (m_sourceEidAfi)) << " ";

  if (m_sourceEidAfi == LispControlMsg::IP)
    {
      os << "Source EID " << Ipv4Address::ConvertFrom (m_sourceEidAddress) << " ";
    }
  else if (m_sourceEidAfi == LispControlMsg::IPV6)
    {
      os << "Source EID " << Ipv6Address::ConvertFrom (m_sourceEidAddress) << " ";
    }

  if (!Ipv4Address::ConvertFrom (m_itrRlocAddrIp).IsEqual (Ipv4Address ()))
    {
      os << "ITR RLOC v4 " << Ipv4Address::ConvertFrom (m_itrRlocAddrIp) << " ";
    }
  if (!Ipv6Address::ConvertFrom (m_itrRlocAddrIpv6).IsEqual (Ipv6Address ()))
    {
      os << "ITR RLOC v6 " << Ipv6Address::ConvertFrom (m_itrRlocAddrIpv6) << " ";
    }

  os << "\nRequest Record ";
  m_mapReqRec->Print (os);
}

LispControlMsg::LispControlMsgType MapRequestMsg::GetMsgType (void)
{
  return LispControlMsg::MAP_REQUEST;
}


MapRequestRecord::MapRequestRecord ()
{
  m_afi = LispControlMsg::IP;
  m_eidMaskLenght = 0;
  m_eidPrefix = static_cast<Address> (Ipv4Address ());
}

MapRequestRecord::MapRequestRecord (Address eidPrefix, uint8_t eidMaskLength)
{
  m_afi = LispControlMsg::IP;
  m_eidPrefix = eidPrefix;
  m_eidMaskLenght = eidMaskLength;
}

MapRequestRecord::~MapRequestRecord ()
{

}

LispControlMsg::AddressFamily MapRequestRecord::GetAfi (void)
{
  return m_afi;
}
void MapRequestRecord::SetAfi (LispControlMsg::AddressFamily afi)
{
  m_afi = afi;
}

void MapRequestRecord::SetMaskLenght (uint8_t maskLenght)
{
  m_eidMaskLenght = maskLenght;
}
uint8_t MapRequestRecord::GetMaskLength (void)
{
  return m_eidMaskLenght;
}

void MapRequestRecord::SetEidPrefix (Address prefix)
{
  m_eidPrefix = prefix;
}
Address MapRequestRecord::GetEidPrefix (void)
{
  return m_eidPrefix;
}

void MapRequestRecord::Serialize (uint8_t *buf) const
{
  buf[0] = static_cast<uint8_t> (m_afi);
  buf[1] = m_eidMaskLenght;
  if (m_afi == LispControlMsg::IP)
    {
      Ipv4Address::ConvertFrom (m_eidPrefix).Serialize (buf + 2);
    }
  else
    {
      Ipv6Address::ConvertFrom (m_eidPrefix).Serialize (buf + 2);
    }
}

Ptr<MapRequestRecord> MapRequestRecord::Deserialize (uint8_t *buf)
{
  Ptr<MapRequestRecord> record = Create<MapRequestRecord> ();

  record->SetAfi (static_cast<LispControlMsg::AddressFamily> (buf[0]));
  record->SetMaskLenght (buf[1]);

  if (record->GetAfi () == LispControlMsg::IP)
    {
      record->SetEidPrefix (static_cast<Address> (Ipv4Address::Deserialize (buf + 2)));
    }
  else
    {
      record->SetEidPrefix (static_cast<Address> (Ipv6Address::Deserialize (buf + 2)));
    }
  return record;
}

void MapRequestRecord::Print (std::ostream& os)
{

  os << "EID PREFIX AFI " << unsigned(static_cast<int> (m_afi)) << " "
     << "Mask Length " << unsigned(m_eidMaskLenght);
  if (m_afi == LispControlMsg::IP)
    {
      os << "EID prefix" << Ipv4Address::ConvertFrom (m_eidPrefix) << " ";
    }
  else if (m_afi == LispControlMsg::IPV6)
    {
      os << "EID prefix" << Ipv6Address::ConvertFrom (m_eidPrefix) << " ";
    }
}

} /* namespace ns3 */
