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
#ifndef SRC_INTERNET_MODEL_LISP_CONTROL_PLANE_MAP_REQUEST_MSG_H_
#define SRC_INTERNET_MODEL_LISP_CONTROL_PLANE_MAP_REQUEST_MSG_H_

#include "lisp-control-msg.h"
#include "map-reply-msg.h"

namespace ns3 {

class MapReplyRecord;

class MapRequestRecord : public SimpleRefCount<MapRequestRecord>
{
public:
  MapRequestRecord ();
  MapRequestRecord (Address eidPrefix, uint8_t eidMaskLength);
  virtual
  ~MapRequestRecord ();

  LispControlMsg::AddressFamily GetAfi (void);
  void SetAfi (LispControlMsg::AddressFamily afi);

  void SetMaskLenght (uint8_t maskLenght);
  uint8_t GetMaskLength (void);

  void SetEidPrefix (Address prefix);
  Address GetEidPrefix (void);

  void Serialize (uint8_t *buf) const;
  static Ptr<MapRequestRecord> Deserialize (uint8_t *buf);

  void Print (std::ostream& os);
private:
  LispControlMsg::AddressFamily m_afi;
  uint8_t m_eidMaskLenght;
  Address m_eidPrefix;
};

class MapRequestMsg : public LispControlMsg
{
public:
  static const LispControlMsgType msgType;

  MapRequestMsg ();
  virtual
  ~MapRequestMsg ();

  void SetA (uint8_t a);
  uint8_t GetA (void);

  void SetM (uint8_t m);
  uint8_t GetM (void);

  void SetP (uint8_t p);
  uint8_t GetP (void);

  void SetS (uint8_t s);
  uint8_t GetS (void);

  void SetP2 (uint8_t p);
  uint8_t GetP2 (void);

  void SetS2 (uint8_t s);
  uint8_t GetS2 (void);

  void SetIrc (uint8_t irc);
  uint8_t GetIrc (void);

  void SetRecordCount (uint8_t recCount);
  uint8_t GetRecordCount (void);

  void SetNonce (uint64_t nonce);
  uint64_t GetNonce (void);

  void SetSourceEidAfi (AddressFamily afi);
  AddressFamily GetSourceEidAfi (void);

  void SetSourceEidAddr (Address sourceEid);
  Address GetSourceEidAddr (void);

  void SetItrRlocAddrIp (Address itrRlocAddr);
  Address GetItrRlocAddrIp (void);

  void SetItrRlocAddrIpv6 (Address itrRlocAddr);
  Address GetItrRlocAddrIpv6 (void);

  void SetMapRequestRecord (Ptr<MapRequestRecord> record);
  Ptr<MapRequestRecord> GetMapRequestRecord (void);

  void Serialize (uint8_t *buf) const;
  static Ptr<MapRequestMsg> Deserialize (uint8_t *buf);

  void Print (std::ostream& os) const;
  static LispControlMsg::LispControlMsgType GetMsgType (void);

private:
  uint8_t m_A : 1; //!< authoritative bit, 0 for udp-based map-request sent by itr
                   //!< if 1 the destination site return the map reply rather than the mapping db system
  uint8_t m_M : 1; //!< if set, map reply record segment is included
  uint8_t m_P : 1; //!< Ptobe bit
  uint8_t m_S : 1; //!< Sollicit map request bit (smr bit)
  uint8_t m_p : 1; //!< Pitr bit, set when a pitr send a map request
  uint8_t m_s : 1; //!< smr invoked bit.
  uint8_t m_reserved : 2;
  uint8_t m_irc; //!< ITR-RLOC Count from 0 to 31. If 0 there is 1 ITR-RLOC address
                 //!< if 1 there are 2 and so on up to 31 -> 32 ITR-RLOC addresses encoded
  uint8_t m_recordCount; //!< Number of records. In this work it will be 1 all the time
  uint64_t m_nonce; //!< Randomly generated nonce (must be copied in map reply)
  AddressFamily m_sourceEidAfi; //!< source eid address family
  Address m_sourceEidAddress; //!< source eid Address
  // one for each address family. We only consider 2 addresses in this work.
  // The ITR put its own rloc
  Address m_itrRlocAddrIp;
  Address m_itrRlocAddrIpv6;
  /*
   * In this work, we just consider the case where
   * we have only one map request record (i.e., record count is 1)
   * see rfc 6830: "a sender MUST only send Map-Requests containing one record"
   */
  Ptr<MapRequestRecord> m_mapReqRec;

  /*
   * we don't use this in this work
   * see rfc6830 sec 6.1.2 Map-Request Message Format
   * placeholder: if we need it someday we can use it
   */
  Ptr<MapReplyRecord> mapRepRec;
};

} /* namespace ns3 */

#endif /* SRC_INTERNET_MODEL_LISP_CONTROL_PLANE_MAP_REQUEST_MSG_H_ */
