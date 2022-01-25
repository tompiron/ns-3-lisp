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
#include "map-reply-msg.h"
#include "ns3/simple-map-tables.h"
namespace ns3 {

MapReplyMsg::MapReplyMsg ()
{
  m_P = 0;
  m_E = 0;
  m_S = 0;
}

MapReplyMsg::~MapReplyMsg ()
{
}


void MapReplyMsg::SetE (uint8_t e)
{
  m_E = e;
}
uint8_t MapReplyMsg::GetE (void)
{
  return m_E;
}

void MapReplyMsg::SetP (uint8_t p)
{
  m_P = p;
}
uint8_t MapReplyMsg::GetP (void)
{
  return m_P;
}

void MapReplyMsg::SetS (uint8_t s)
{
  m_S = s;
}
uint8_t MapReplyMsg::GetS (void)
{
  return m_S;
}

void MapReplyMsg::SetRecordCount (uint8_t recCount)
{
  m_recordCount = recCount;
}
uint8_t MapReplyMsg::GetRecordCount (void)
{
  return m_recordCount;
}

void MapReplyMsg::SetNonce (uint64_t nonce)
{
  m_nonce = nonce;
}
uint64_t MapReplyMsg::GetNonce (void)
{
  return m_nonce;
}

void MapReplyMsg::Serialize (uint8_t *buf) const
{
  uint8_t EPSres = 0;

  EPSres = (m_E << 7) | (m_P << 6) | (m_S << 5) | (m_reserved);

  buf[0] = static_cast<uint8_t> (GetMsgType ());
  buf[1] = EPSres;
  buf[2] = m_recordCount;

  buf[3] = (m_nonce >> 56) & 0xffff;
  buf[4] = (m_nonce >> 48) & 0xffff;
  buf[5] = (m_nonce >> 40) & 0xffff;
  buf[6] = (m_nonce >> 32) & 0xffff;
  buf[7] = (m_nonce >> 24) & 0xffff;
  buf[8] = (m_nonce >> 16) & 0xffff;
  buf[9] = (m_nonce >> 8) & 0xffff;
  buf[10] = (m_nonce >> 0) & 0xffff;

  uint8_t size = 11;
  m_record->Serialize (buf + size);
}
Ptr<MapReplyMsg> MapReplyMsg::Deserialize (uint8_t *buf)
{
  Ptr<MapReplyMsg> msg = Create<MapReplyMsg> ();

  uint8_t EPSres = buf[1];
  msg->SetE (EPSres >> 7);
  msg->SetP (EPSres >> 6);
  msg->SetS (EPSres >> 5);

  msg->SetRecordCount (buf[2]);

  uint64_t nonce = 0;
  nonce |= buf[3];
  nonce <<= 8;
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

  msg->SetNonce (nonce);

  uint8_t size = 11;
  msg->SetRecord (MapReplyRecord::Deserialize (buf + size));

  return msg;
}

void MapReplyMsg::SetRecord (Ptr<MapReplyRecord> record)
{
  m_record = record;
}
Ptr<MapReplyRecord> MapReplyMsg::GetRecord (void)
{
  return m_record;
}

void MapReplyMsg::Print (std::ostream& os) const
{
  os << "E: " << unsigned (m_E) << " P: " << unsigned (m_P) << " S: " << unsigned (m_S) <<
    " Nonce: " << unsigned (m_nonce) << " Record Count: " << unsigned (m_recordCount) << "Reply-Record: ";
  m_record->Print (os);
  os << std::endl;
}
LispControlMsg::LispControlMsgType MapReplyMsg::GetMsgType (void)
{
  return LispControlMsg::MAP_REPLY;
}

const uint32_t MapReplyRecord::m_defaultRecordTtl = 0xffffff;

MapReplyRecord::MapReplyRecord () :
  m_recordTtl (0),
  m_locatorCount (0),
  m_eidMaskLength (0),
  m_A (0),
  m_reserved (0),
  m_mapVersionNumber (0)
{
  m_act = Drop;
  m_eidPrefixAfi = LispControlMsg::IP;
  m_eidPrefix = static_cast<Address> (Ipv4Address ());
}

MapReplyRecord::~MapReplyRecord ()
{

}

void MapReplyRecord::SetRecordTtl (uint32_t recordTtl)
{
  m_recordTtl = recordTtl;
}

uint32_t MapReplyRecord::GetRecordTtl (void)
{
  return m_recordTtl;
}

void MapReplyRecord::SetLocatorCount (uint8_t locCount)
{
  m_locatorCount = locCount;
}

uint8_t MapReplyRecord::GetLocatorCount (void)
{
  return m_locatorCount;
}

void MapReplyRecord::SetEidMaskLength (uint8_t eidMaskLength)
{
  m_eidMaskLength = eidMaskLength;
}

uint8_t MapReplyRecord::GetEidMaskLength (void)
{
  return m_eidMaskLength;
}


void MapReplyRecord::SetAct (ACT act)
{
  m_act = act;
}

MapReplyRecord::ACT MapReplyRecord::GetAct (void)
{
  return m_act;
}

void MapReplyRecord::SetA (uint8_t a)
{
  m_A = a;
}

uint8_t MapReplyRecord::GetA (void)
{
  return m_A;
}

void MapReplyRecord::SetMapVersionNumber (uint16_t versionNumber)
{
  m_mapVersionNumber = versionNumber;
}

uint16_t MapReplyRecord::GetMapVersionNumber (void)
{
  return m_mapVersionNumber;
}

LispControlMsg::AddressFamily MapReplyRecord::GetEidAfi (void)
{
  return m_eidPrefixAfi;
}

void MapReplyRecord::SetEidAfi (LispControlMsg::AddressFamily afi)
{
  m_eidPrefixAfi = afi;
}

void MapReplyRecord::SetLocators (Ptr<Locators> locators)
{
  m_locators = locators;
  if (locators)
    {
      m_locatorCount = locators->GetNLocators ();
    }
  else
    {
      m_locatorCount = 0;
    }
}

Ptr<Locators> MapReplyRecord::GetLocators (void)
{
  return m_locators;
}

void MapReplyRecord::Serialize (uint8_t *buf)
{
  uint8_t aRes = (m_A << 7) | (m_reserved);
  buf[0] = aRes;
  buf[1] = static_cast<uint8_t> (m_act);
  buf[2] = m_locatorCount;
  buf[3] = m_eidMaskLength;

  buf[4] = (m_mapVersionNumber >> 8) & 0xff;
  buf[5] = (m_mapVersionNumber >> 0) & 0xff;

  buf[6] = (m_recordTtl >> 24) & 0xff;
  buf[7] = (m_recordTtl >> 16) & 0xff;
  buf[8] = (m_recordTtl >> 8) & 0xff;
  buf[9] = (m_recordTtl >> 0) & 0xff;
  uint8_t size = 10;

  if (Ipv4Address::IsMatchingType (m_eidPrefix))
    {
      buf[10] = static_cast<uint8_t> (LispControlMsg::IP);
      size++;
      Ipv4Address::ConvertFrom (m_eidPrefix).Serialize (buf + size);
      size += 4;
    }
  else if (Ipv6Address::IsMatchingType (m_eidPrefix))
    {
      buf[10] = static_cast<uint8_t> (LispControlMsg::IPV6);
      size++;
      Ipv6Address::ConvertFrom (m_eidPrefix).Serialize (buf + size);
      size += 16;
    }

  if (m_locators)
    {
      buf[size] = 1;
      size++;
      m_locators->Serialize (buf + size);
    }
  else
    {
      buf[size] = 0;
    }

}

Ptr<MapReplyRecord> MapReplyRecord::Deserialize (uint8_t *buf)
{
  Ptr<MapReplyRecord> record = Create<MapReplyRecord> ();

  record->SetA (buf[0] >> 7);

  record->SetAct (static_cast<ACT> (buf[1]));
  record->SetLocatorCount (buf[2]);
  record->SetEidMaskLength (buf[3]);

  uint16_t mapVersionNumber = 0;
  mapVersionNumber |= buf[4];
  mapVersionNumber <<= 8;
  mapVersionNumber |= buf[5];

  record->SetMapVersionNumber (mapVersionNumber);

  uint32_t recordTtl = 0;

  recordTtl |= buf[6];
  recordTtl <<= 8;
  recordTtl |= buf[7];
  recordTtl <<= 8;
  recordTtl |= buf[8];
  recordTtl <<= 8;
  recordTtl |= buf[9];

  record->SetRecordTtl (recordTtl);

  uint8_t size = 10;

  if (buf[size] == static_cast<uint8_t> (LispControlMsg::IP))
    {
      size++;
      record->SetEidAfi (LispControlMsg::IP);
      record->SetEidPrefix (static_cast<Address> (Ipv4Address::Deserialize (buf + size)));
      size += 4;
    }
  else if (buf[size] == static_cast<uint8_t> (LispControlMsg::IPV6))
    {
      size++;
      record->SetEidAfi (LispControlMsg::IPV6);
      record->SetEidPrefix (static_cast<Address> (Ipv4Address::Deserialize (buf + size)));
      size += 16;
    }

  if (buf[size])
    {
      size++;
      record->SetLocators (LocatorsImpl::Deserialize (buf + size));
    }
  return record;
}

void MapReplyRecord::SetEidPrefix (Address eidPrefix)
{
  m_eidPrefix = eidPrefix;
  if (Ipv4Address::IsMatchingType (eidPrefix))
    {
      m_eidPrefixAfi = LispControlMsg::IP;
    }
  else
    {
      m_eidPrefixAfi = LispControlMsg::IPV6;
    }
}

Address MapReplyRecord::GetEidPrefix (void)
{
  return m_eidPrefix;
}

void MapReplyRecord::Print (std::ostream& os)
{

  os.flush ();
  os << "\nEid prefix afi " << unsigned(static_cast<int> (m_eidPrefixAfi)) << " "
     << "Mask Length " << unsigned(m_eidMaskLength);
  if (m_eidPrefixAfi == LispControlMsg::IP)
    {
      os << "EID prefix " << Ipv4Address::ConvertFrom (m_eidPrefix) << " ";
    }
  else if (m_eidPrefixAfi == LispControlMsg::IPV6)
    {
      os << "EID prefix " << Ipv6Address::ConvertFrom (m_eidPrefix) << " ";
    }
  if (m_locators)
    {
      os << "Locators: " << m_locators->Print () << std::endl;
    }
}

} /* namespace ns3 */
