/*
 * map-request-record.cc
 *
 *  Created on: Apr 4, 2017
 *      Author: qsong
 */

#include "map-request-record.h"

namespace ns3 {

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

uint8_t MapRequestRecord::GetN (void) const
{
  return m_N;
}

void MapRequestRecord::SetN (uint8_t n)
{
  m_N = n;
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

uint8_t MapRequestRecord::GetSizeInBytes (void) const
{
  uint8_t size = 4; // Flags + Mask length + AFI
  if (m_afi == LispControlMsg::IP)
    {
      size += 4;
    }
  else
    {
      size += 16;
    }
  return size;
}

void MapRequestRecord::Serialize (uint8_t *buf) const
{
  int position = 0;
  buf[position] = 0 | (m_N << 7);
  position += 1;

  // EID mask len
  buf[position] = m_eidMaskLenght;
  position += 1;

  // 3,4th byte for EID-Prefix-AFI
  buf[position] = 0x00;
  position += 1;
  buf[position] = static_cast<uint8_t> (m_afi);
  position += 1;
  if (m_afi == LispControlMsg::IP)
    {
      Ipv4Address::ConvertFrom (m_eidPrefix).Serialize (buf + position);
    }
  else
    {
      Ipv6Address::ConvertFrom (m_eidPrefix).Serialize (buf + position);
    }
}

Ptr<MapRequestRecord> MapRequestRecord::Deserialize (uint8_t *buf)
{
  Ptr<MapRequestRecord> record = Create<MapRequestRecord>();
  int position = 0;
  record->SetN ((buf[0] >> 7) & 0x01);
  record->SetAfi (static_cast<LispControlMsg::AddressFamily> (buf[3]));
  record->SetMaskLenght (buf[1]);
  position = 4;
  if (record->GetAfi () == LispControlMsg::IP)
    {
      record->SetEidPrefix (
        static_cast<Address> (Ipv4Address::Deserialize (buf + position)));
    }
  else
    {
      record->SetEidPrefix (
        static_cast<Address> (Ipv6Address::Deserialize (buf + position)));
    }
  return record;
}

void MapRequestRecord::Print (std::ostream& os)
{

  os << "N " << unsigned(m_N) << " "
     << "EID PREFIX AFI " << unsigned(static_cast<int> (m_afi)) << " "
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
