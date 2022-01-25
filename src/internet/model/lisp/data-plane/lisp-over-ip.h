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
#ifndef LISP_OVER_IP_H_
#define LISP_OVER_IP_H_

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "lisp-protocol.h"
#include "map-tables.h"
#include "ns3/object.h"
#include "mapping-socket-address.h"
#include "mapping-socket-msg.h"
#include <set>
//#include "lisp-mapping-socket.h"

namespace ns3 {

class MapTables;
class MapEntry;
class Node;
class Socket;
class LispMappingSocket;

/**
 * \class LispOverIp
 * \brief An abstract class for LISP data plane.
 *
 * This class must be implemented by the classes
 * that implement the LISP data plane for IPv4 and
 * IPv6.
 *
 */
class LispOverIp : public Object
{
public:
  /**
   * \struct address_compare
   *
   * The only goal of this structure is to
   * hold the method to compare two addresses
   * of the same type.
   */
  struct address_compare
  {
    bool operator() (const Address& lhs, const Address& rhs) const
    {
      if (Ipv4Address::IsMatchingType (lhs) && Ipv4Address::IsMatchingType (rhs))
        {
          return Ipv4Address::ConvertFrom (lhs).Get () < Ipv4Address::ConvertFrom (rhs).Get ();
        }
      else
        {
          return Ipv6Address::ConvertFrom (lhs) < (Ipv6Address::ConvertFrom (rhs));
        }
    }
  };

  /**
   * \brief Get the type ID.
   * \return The object TypeID.
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  LispOverIp ();

  /**
   * \brief Constructor
   *
   * \param statisticsForIpv4 The LISP statistics for IPv4 packet.
   * \param statisticsForIpv6 The LISP statistics for IPv6 packets.
   */
  LispOverIp (Ptr<LispStatistics> statisticsForIpv4, Ptr<LispStatistics> statisticsForIpv6);

  /**
   * \brief Destructor
   */
  virtual
  ~LispOverIp ();

  /**
   * \brief Get a pointer to the current Node
   * \return A pointer to the current Node.
   */
  Ptr<Node> GetNode (void);

  /**
   * \brief Set the current node
   *
   * \param A pointer to the current Node.
   */
  void SetNode (Ptr<Node>);

  /**
   * \brief Create a new LispMappingSocket.
   *
   * \return A pointer to a newly created LispMapping socket.
   */
  Ptr<Socket> CreateSocket (void);

  /**
   * \brief Set the MapTables for this LISP protocol
   *
   * \param mapTablesIpv4 A pointer to a MapTables for IPv4 EID addresses.
   */
  void SetMapTablesIpv4 (Ptr<MapTables> mapTablesIpv4);

  /**
   * \brief Set the MapTables for this LISP protocol.
   *
   * \param mapTablesIpv4 A pointer to a MapTables for IPv6 EID addresses.
   */
  void SetMapTablesIpv6 (Ptr<MapTables> mapTablesIpv6);

  /**
   * \brief Look the EID address given as an argument up in the LISP Database.
   * \param eidAddress The EID address that is looked up in the database.
   * \return The entry matching the EID address.
   */
  Ptr<MapEntry> DatabaseLookup (Address const &eidAddress) const;

  /**
   * \brief Look the EID address given as an argument up in the LISP Cache.
   * \param eidAddress The EID address that is looked up in the cache.
   * \return The entry matching the EID address.
   */
  Ptr<MapEntry> CacheLookup (Address const &eidAddress) const;

  /**
   * \brief Select the best destination RLOC among all the RLOC in the
   * map entry.
   *
   * \param mapEntry Entry that contains the set of RLOCs
   * \return A pointer to the selected RLOC.
   */
  Ptr<Locator> SelectDestinationRloc (Ptr<const MapEntry> mapEntry) const;
  /**
   * \brief Select  the source RLOC according to the destination RLOC.
   *
   * The selected source RLOC must be:
   * - Usable (up)
   * - The address of a local interface
   * - Its priority must be smaller that 255.
   *
   * \param srcEid The source EID address.
   * \param destLocator A pointer to the destination RLOC.
   * \return A pointer to the selected source RLOC.
   */
  Ptr<Locator> SelectSourceRloc (Address const &srcEid, Ptr<const Locator> destLocator) const;

  /**
   * \brief Performs the encapsulation by adding the udp header to the packet
   *
   * \param packet The packet to which the header will be added
   * \param udpLength The value of the length field of the udp header.
   * \param udpSrcPort The value of the source port field of the udp header
   * \return The packet with the udp header prepended to it.
   */
  virtual Ptr<Packet> LispEncapsulate (Ptr<Packet> packet, uint16_t udpLength, uint16_t udpSrcPort) = 0;

  /**
   * \brief Get the LISP statistics for IPv4 packets.
   *
   * \return A pointer to the LispStatistics object for IPv4 packets
   */
  Ptr<LispStatistics> GetLispStatisticsV4 (void);

  /**
   * \brief Get the LISP statistics for IPv6 packets.
   *
   * \return A pointer to the LispStatistics object for IPv6 packets
   */
  Ptr<LispStatistics> GetLispStatisticsV6 (void);

  /**
   * \brief Set the LISP statistics.
   * \param statisticsV4 A pointer to the LispStatistics object for IPv4 packets
   * \param statisticsV6 A pointer to the LispStatistics object for IPv6 packets
   */
  void SetLispStatistics (Ptr<LispStatistics> statisticsV4, Ptr<LispStatistics> statisticsV6);

  /**
   * \brief Get the Lisp Mapping socket identified by the index given as
   * an argument
   *
   * \param sockIndex A unique index identifying a LISP Mapping Socket
   * \return A pointer to the LispMappingSocket corresponding to the index.
   */
  Ptr<LispMappingSocket> GetMappingSocket (uint8_t sockIndex);

  /**
   * \brief Get the address of the LISP data plane (the LispMappingSocketAddress)
   * \return The address of the LISP data plane (the LispMappingSocketAddress).
   */
  Address GetLispMapSockAddress (void);

  /**
   * \brief Open the LISP mapping socket so that control plane can connect to it.
   */
  void OpenLispMappingSocket (void);

  /**
   * \brief Handle a new packet received on the LispMappingSocket given
   * as an argument (callback method).
   *
   * \param socket The socket on which the packet is received.
   */
  void HandleMapSockRead (Ptr<Socket> socket);
  void SendNotifyMessage (uint8_t messageType, Ptr<Packet> packet, MappingSocketMsgHeader mapSockMsgHeader, int flags);

  /**
   * \brief Set the List of the RLOC addresses of the system.
   * \param rlocsList A set containing all the RLOC addresses of the system.
   */
  void SetRlocsList (std::set<Address> rlocsList);

  /**
   * \brief Get the list of RLOC addresses of the system.
   * \return A set containing the list of RLOC addresses of the system.
   */
  std::set<Address> GetRlocsList (void) const;

  /**
   * \brief Check if the address given as an argument is an RLOC address
   * of the system.
   *
   * \param address The address to check.
   * \return True if the address is an RLOC, false otherwise.
   */
  bool IsLocatorInList (Address address) const;

  /**
   * \brief Get the MapTables for IPv4 packets associated to this node.
   * \return The MapTables for IPv4 addresses.
   */
  Ptr<MapTables> GetMapTablesV4 (void) const;

  /**
   * \brief Get the MapTables for IPv6 packets associated to this node.
   * \return The MapTables for IPv6 addresses.
   */
  Ptr<MapTables> GetMapTablesV6 (void) const;

protected:
  // Note: Each entry of the table can contain Ipv6 or Ipv4 RLOC addresses
  Ptr<MapTables> m_mapTablesIpv4;       //!< Map table for Ipv4 EID prefixes
  Ptr<MapTables> m_mapTablesIpv6;       //!< Map table for Ipv6 EID prefixes
  Ptr<LispStatistics> m_statisticsForIpv4;
  Ptr<LispStatistics> m_statisticsForIpv6;
  std::set<Address> m_rlocsList;        //!< The list of all RLOCs addresses of the system (needed to prevent encapsulation)


private:
  /**
   * This function will notify other components connected to the node that a new stack member is now connected
   * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
   */
  virtual void NotifyNewAggregate ();
  virtual void DoDispose (void);
  std::vector<Ptr<LispMappingSocket> > m_sockets;       //!< list of mapping sockets
  Ptr<Socket> m_lispSocket; //!< the socket owned by the data plane.
  Address m_lispAddress; //!< the "address" of the data plane (to connect to the socket)
  Ptr<Node> m_node; //!< the node

};

} /* namespace ns3 */

#endif /* LISP_OVER_IP_H_ */
