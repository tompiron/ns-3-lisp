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
#ifndef MAP_SOCKET_MSG_HEADER_H_
#define MAP_SOCKET_MSG_HEADER_H_

#include "ns3/header.h"
#include "ns3/object.h"

namespace ns3 {

class Address;
class Ipv6Prefix;
class Ipv4Mask;
class Locators;
class EndpointId;

class MappingSocketMsgHeader : public Header
{
public:
  /**
   * \brief Get the type identifier.
   * \return type identifier
   */
  static TypeId GetTypeId (void);

  MappingSocketMsgHeader ();
  virtual
  ~MappingSocketMsgHeader ();

  /**
   * \brief Print some informations about the packet.
   * \param os output stream
   * \return info about this packet
   */
  virtual void Print (std::ostream& os) const;

  /**
   * \brief Get the serialized size of the packet.
   * \return size
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * \brief Serialize the packet.
   * \param start Buffer iterator
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start Buffer iterator
   * \return size of the packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * \brief Return the instance type identifier.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId (void) const;

  void SetMapVersion (uint8_t mapVersion);
  uint8_t GetMapVersion (void);

  void SetMapType (uint16_t mapType);
  uint16_t GetMapType (void);

  void SetMapFlags (uint32_t mapFlags);
  uint32_t GetMapFlags (void);

  void SetMapAddresses (uint16_t mapAddresses);
  uint16_t GetMapAddresses (void);

  void SetMapVersioning (uint16_t mapVersioning);
  uint16_t GetMapVersioning (void);

  void SetMapRlocCount (uint32_t mapRlocCount);
  uint32_t GetMapRlocCount (void);
private:
  uint8_t m_mapVersion;   /* ? future binary compatibility */
  uint16_t m_mapType;   /* message type */
  uint32_t m_mapFlags;   /* flags */
  uint16_t m_mapAddresses;
  uint16_t m_mapVersioning;   /* Map Version number */
  uint32_t m_mapRlocCount;   /* Number of RLOCs appended*/

};

class MappingSocketMsg :  public SimpleRefCount<MappingSocketMsg>
{
public:
  static TypeId GetTypeId (void);

  MappingSocketMsg ();
  MappingSocketMsg (Ptr<EndpointId> endPoint, Ptr<Locators> locatorsList);
  virtual ~MappingSocketMsg ();

  void Print (std::ostream& os);

  void SetEndPoint (Ptr<EndpointId> endpoint);
  Ptr<EndpointId> GetEndPointId (void);

  void SetLocators (Ptr<Locators> locatorsList);
  Ptr<Locators> GetLocators (void);

  void Serialize (uint8_t *buf);
  static Ptr<MappingSocketMsg> Deserialize (uint8_t *buf);

private:
  Ptr<EndpointId> m_endPoint; //!< prefix + netmask
  Ptr<Locators> m_locatorsList; //!< locators associated to the endpoint

};



} /* namespace ns3 */

#endif /* MAP_SOCKET_MSG_H_ */
