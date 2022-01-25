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
#include "map-register-msg.h"


namespace ns3 {

MapRegisterMsg::MapRegisterMsg ()
{
  m_P = 0;
  m_M = 0;
  m_nonce = 0;
  m_recordCount = 0;
  m_record = 0;
}

MapRegisterMsg::~MapRegisterMsg ()
{
  m_record = 0;
}

uint8_t MapRegisterMsg::GetP (void)
{
  return m_P;
}

void MapRegisterMsg::SetP (uint8_t p)
{
  m_P = p;
}

uint8_t MapRegisterMsg::GetM (void)
{
  return m_M;
}

void MapRegisterMsg::SetM (uint8_t m)
{
  m_M = m;
}

uint8_t MapRegisterMsg::GetRecordCount (void)
{
  return m_recordCount;
}

void MapRegisterMsg::SetRecordCount (uint8_t count)
{
  m_recordCount = count;
}

void MapRegisterMsg::SetRecord (Ptr<MapReplyRecord> record)
{
  m_record = record;
}

Ptr<MapReplyRecord> MapRegisterMsg::GetRecord (void)
{
  return m_record;
}

void MapRegisterMsg::Serialize (uint8_t *buf)
{
  buf[0] = static_cast<uint8_t> (LispControlMsg::MAP_REGISTER);
  uint8_t PMrsvd = (m_P << 7) | (m_M << 6) | reserved;
  buf[1] = PMrsvd;
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

Ptr<MapRegisterMsg> MapRegisterMsg::Deserialize (uint8_t *buf)
{
  Ptr<MapRegisterMsg> msg = Create<MapRegisterMsg> ();

  msg->SetP (buf[1] >> 7);
  msg->SetM (buf[1] >> 6);

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

LispControlMsg::LispControlMsgType MapRegisterMsg::GetMsgType (void)
{
  return LispControlMsg::MAP_REGISTER;
}

void MapRegisterMsg::SetNonce (uint64_t nonce)
{
  m_nonce = nonce;
}
uint64_t MapRegisterMsg::GetNonce (void)
{
  return m_nonce;
}

} /* namespace ns3 */
