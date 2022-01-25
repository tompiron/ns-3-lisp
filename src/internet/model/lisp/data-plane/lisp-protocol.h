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
#ifndef LISP_H_
#define LISP_H_

#include "ns3/address.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "lisp-header.h"

namespace ns3 {

class Packet;
class Address;
class MapEntry;
class Locator;
class LispOverIp;
class TypeId;

/**
 * \brief Implementation of the LISP protocol utilities and declaration
 * of LISP constants.
 *
 * The main goal of this class is to implement static check
 * function for the LISP protocol.
 */
class LispProtocol
{

public:
  static const uint8_t PROT_NUMBER; //!< protocol number (0x)
  static const uint16_t LISP_DATA_PORT; //!< LISP data operations port
  static const uint16_t LISP_SIG_PORT; //!< LISP control operations port
  static const uint16_t LISP_MAX_RLOC_PRIO; //!< LISP RLOC maximum priority
  static const uint16_t MAX_VERSION_NUM; //!< LISP maximum version number
  static const uint16_t WRAP_VERSION_NUM;
  /**
   * The following constant cannot be used as a Map-Version number.
   * It indicates that no Map-Version number is assigned to the
   * EID-TO-RLOC mapping
   */
  static const uint8_t NULL_VERSION_NUM;

  // TODO set number of RLOC in the mapping database/cache

  /**
   * This enum indicates what to do when there is no
   * entry in the cache for the source EID of a packet.
   */
  enum EtrState
  {
    LISP_ETR_STANDARD = 1,   //!< We don't do nothing
    /**
     * A notification is sent to the control plane and
     * the packet is forwarded.
     */
    LISP_ETR_NOTIFY = 2,
    /**
     * A notification is sent and the packet is dropped.
     */
    LISP_ETR_SECURE = 3
  };

  /**
   * \brief Get the type ID of this class.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  LispProtocol ();

  /**
   * \brief Destructor
   */
  virtual ~LispProtocol ();


  /**
   * This static method prepends a newly built LISP header to
   * the packet given as an argument.
   *
   * \param packet The packet to which the LISP header must be prepended
   * \param localMapEntry The local Map entry (from the LISP database)
   * \param remoteMapEntry The remote Map entry (from the LISP Cache)
   * \param sourceRloc The source locator (in the outer IP header)
   * \param destRloc The destination locator (in the outer IP header)
   * \return The packet given as an argument with the LISP header
   *            prepended.
   */
  static Ptr<Packet> PrependLispHeader (Ptr<Packet> packet, Ptr<const MapEntry>
                                        localMapEntry, Ptr<const MapEntry> remoteMapEntry, Ptr<Locator> sourceRloc,
                                        Ptr<Locator> destRloc);

  /**
   * This static method checks if the LISP header of a received packet
   * is formated as expected.
   * \param header The LISP header of the received packet
   * \param localMapEntry The Local map entry (on the ETR LISP Database)
   * \param remoteMapEntry The remote map entry (in the ETR LISP Cache)
   * \param srcRloc The source Locator
   * \param destRloc The destination Locator
   * \param lispOverIp A pointer to the LISP protocol implementation
   *
   * \return True if the LISP header is well formated, false otherwise.
   */
  static bool CheckLispHeader (const LispHeader &header, Ptr<const MapEntry>
                               localMapEntry, Ptr<const MapEntry> remoteMapEntry, Ptr<Locator> srcRloc,
                               Ptr<Locator> destRloc, Ptr<LispOverIp> lispOverIp);

  /**
   * The goal of this method is to select the source UDP port for the
   * LISP packet.
   *
   * \param packet The LISP packet
   *
   * \return the selected source UDP port.
   */
  static uint16_t GetLispSrcPort (Ptr<const Packet> packet);

  /**
   * This method determine if the Mapping version number 2 (vnum2)
   *  is greater (newer) than the Mapping version number 1 (vnum1). It
   *  uses the following algorithm:
   *
   * 1. V1 = V2 : The Map-Version numbers are the same.
   * 2. V2 > V1 : if and only if
   * V2 > V1 AND (V2 - V1) <= 2**(N-1)
   * OR
   * V1 > V2 AND (V1 - V2) > 2**(N-1)
   * 3. V1 > V2 : otherwise
   *
   * \param vnum1 The Mapping version number 2
   * \param vnum2 The Mapping version number 1
   *
   * \return True if the mapping version number 2 is newer than the mapping
   * version number 1. False otherwise.
   */
  static bool IsMapVersionNumberNewer (uint16_t vnum2, uint16_t vnum1);
};

/**
 * \brief Implementation of the statistics related to LISP.
 */
class LispStatistics : public Object
{
public:
  // TODO Add attributes
  static TypeId GetTypeId (void);
  LispStatistics ();
  ~LispStatistics ();

  /**
   * This method increases the m_noLocMapPresent by one unit.
   */
  void NoLocalMap (void);
  /**
   *
   */
  void BadDestVersionNumber (void);
  /**
   *
   */
  void BadSrcVersionNumber (void);
  /**
   *
   */
  void IncInputPacket (void);
  /**
   *
   */
  void IncBadSizePackets (void);
  /**
   *
   */
  void IncInputDifAfPackets (void);
  /**
   *
   */
  void IncCacheMissPackets (void);
  /**
   *
   */
  void IncNoValidRloc (void);
  /**
   *
   */
  void IncOutputDropPackets (void);
  /**
   *
   */
  void IncNoValidMtuPackets (void);
  /**
   *
   */
  void IncNoEnoughSpace (void);
  /**
   *
   */
  void IncOutputPackets (void);
  /**
   *
   */
  void IncOutputDifAfPackets (void);

  /**
   *
   * \param os
   */
  void Print (std::ostream &os) const;

private:
  // input statistics
  /*
   * total input packets
   */
  uint32_t m_inputPackets;
  /*
   * total input packets with a different
   * address family in the outer header packet
   */
  uint32_t m_inputDifAfPackets;
  /*
   * packets dropped because they are shorter
   * that their header.
   */
  uint32_t m_badSizePackets; // TODO may be remove
  /*
   * packets for which there is no
   * local mapping
   */
  uint32_t m_noLocMapPresent;
  /*
   * packets that are dropped because
   * the data length is larger that the packets
   * themselves
   */
  uint32_t m_badDataLength; // TODO may be remove
  /*
   * Packets with bad source version number
   */
  uint32_t m_badSourceVersionNumber;
  /*
   * packets with a bad destination version number
   */
  uint32_t m_badDestVesionNumber;

  // output statistics
  /*
   * total number of output lisp packets
   */
  uint32_t m_outputPackets;
  /*
   * total number of input packets with a different
   * address family in the inner header
   */
  uint32_t m_outputDifAfPackets;
  /*
   * packets that are dropped because of a cache miss
   */
  uint32_t m_cacheMissPackets;
  /*
   * packets dropped because there is no valid RLOC
   */
  uint32_t m_noValidRlocPackets;
  /*
   * packets dropped because they
   * don't pass the MTU check
   */
  uint32_t m_noValidMtuPackets;
  /*
   * packets dropped because the there is no buffer space
   */
  uint32_t m_noEnoughBufferPacket;
  /*
   * total output packets that are dropped
   */
  uint32_t m_outputDropPackets;
};

/**
 * \brief Implementation of the RLOC metrics.
 *
 * This class exists to holds the metrics associated to an RLOC.
 */
class RlocMetrics : public SimpleRefCount<RlocMetrics>
{
public:
  //static TypeId GetTypeId (void);
  /**
   * \brief Constructor
   */
  RlocMetrics ();
  /**
   * \brief Constructor
   * \param priority The RLOC Priority
   * \param weight The RLOC Weight
   */
  RlocMetrics (uint8_t priority, uint8_t weight);

  /**
   * \brief Constructor
   * \param priority The RLOC Priority
   * \param weight The RLOC Weight
   * \param reachable State of the RLOC (reachable or not)
   */
  RlocMetrics (uint8_t priority, uint8_t weight, bool reachable);
  /**
   * \brief Destructor
   */
  ~RlocMetrics ();

  /**
   * The RLOC flags
   */
  enum RlocMetricsFlags
  {
    RLOCF_UP = 0x01,     //!< Set if RLOC is up
    RLOCF_LIF = 0x02,    //!< Set if the RLOC address is the one of the local interface
    RLOC_TXNONCE = 0x04, //!< Set if the transmission nonce is present
    RLOCF_RXNONCE = 0x08,//!< Set if the reception nonce is up.
  };

  /**
   * This method returns the priority of the RLOC.
   *
   * \return The priority of the RLOC
   */
  uint8_t GetPriority (void) const;

  /**
   * This method set the priority of the RLOC.
   *
   * \param priority The priority of the RLOC
   */
  void SetPriority (uint8_t priority);

  /**
   * Get the weight of the RLOC.
   *
   * \return the weight of the RLOC.
   */
  uint8_t GetWeight (void) const;
  /**
   * Set the weight of the RLOC.
   * \param weight the new weight of the RLOC.
   */
  void SetWeight (uint8_t weight);

  /**
   * Get the MTU of the RLOC.
   * \return the MTU.
   */
  uint32_t GetMtu (void) const;

  /**
   * Set the MTU of the RLOC.
   * \param mtu The MTU of the RLOC.
   */
  void SetMtu (uint32_t mtu);

  /**
   * Get the state of the RLOC (usable or not)
   * \return true if the RLOC is usable, false otherwise
   */
  bool IsUp (void);
  /**
   * Set the state of the RLOC.
   * \param status The new state of the RLOC.
   */
  void SetUp (bool status);

  /**
   * Get if the RLOC address is the one of a local interface or not.
   *
   * \return True if the RLOC address is the one of a local interface. False
   * otherwise.
   */
  bool IsLocalInterface (void);

  /**
   * Set if the RLOC is the one of a local interface or not.
   * \param isLocal The state of the RLOC address (local or not).
   */
  void SetIsLocalIf (bool isLocal);

  /**
   * Get if a transmission nonce is present or not.
   * \return True if a transmission nonce is present.
   */
  bool IsTxNoncePresent (void);
  /**
   * Set if the transmission nonce is present or not.
   * \param txNoncePresent The state of the transmission nonce (present or not)
   */
  void SetTxNoncePresent (bool txNoncePresent);

  /**
   * Get if the reception nonce is present or not.
   * \return True if the reception nonce is present.
   */
  bool IsRxNoncePresent (void);

  /**
   * Set the transmission nonce.
   * \param rxNoncePresent The state of the transmission nonce (present or not)
   */
  void SetRxNoncePresent (bool rxNoncePresent);

  /**
   * Get the transmission nonce.
   * \return The transmission nonce.
   */
  uint32_t GetTxNonce (void);
  /**
   * Set the transmission nonce.
   * \param txNonce The transmission nonce.
   */
  void SetTxNonce (uint32_t txNonce);

  /**
   * Get the reception nonce.
   * \return The reception nonce.
   */
  uint32_t GetRxNonce (void);

  /**
   * Set the reception nonce
   * \param rxNonce The reception nonce
   */
  void SetRxNonce (uint32_t rxNonce);

  /**
   * Print the RlocMetrics object.
   * \return The string representation of the RlocMetrics object.
   */
  std::string Print ();

  /**
   * Serialize the RlocMetrics object in the buffer given as
   * an argument.
   * \param buf The buffer in which the RlocMetrics object will be serialized.
   * \return The size of the buffer given as an argument after serialization.
   */
  uint8_t Serialize (uint8_t *buf);

  /**
   * Deserialize the RlocMetrics object from the buffer given as an argument
   * \param buf The buffer containing the serialized RlocMetrics object.
   * \return A pointer to the deserialized RlocMetrics object.
   */
  static Ptr<RlocMetrics> Deserialized (const uint8_t *buf);

private:
  uint8_t m_priority; // Rloc priority
  /*
   * locator weight: used when several Rlocs have the same
   * priority. The sum of the weight fields of the Rlocs having the
   * same priority must always be 100 or 0.
   */
  uint8_t m_weight;
  bool m_rlocIsUp; // Rloc Status Bit
  bool m_rlocIsLocalInterface; // Rloc is a local interface
  bool m_txNoncePresent; // Rloc Tx Nonce is present
  bool m_rxNoncePresent; // Rloc Rx Nonce is present
  uint32_t m_txNonce; // used when sending a LISP encapsulated packet
  uint32_t m_rxNonce; // used when receiving LISP encapsulated packet
  /*
   * This is useful for local mapping for which flag 'i' is
   * set.
   * Initialized at creation by copying the mtu of the corresponding
   * interface here. This value is not updated if changed. (TODO OK ?)
   */
  uint32_t m_mtu;
};

/**
 * \brief Implementation of a Routing LOCator.
 *
 * The goal of this class is to hold the information related to an
 * RLOC (the RLOC address and its metrics).
 *
 */
class Locator : public SimpleRefCount<Locator>
{
public:
  /**
   * \brief Constructor
   * \param rlocAddress The RLOC address
   */
  Locator (Address rlocAddress);
  /**
   * \brief Constructor
   */
  Locator ();

  /**
   * \brief Destructor
   */
  ~Locator ();

  /**
   * \brief Get the address of the RLOC.
   * \return The RLOC address.
   */
  Address GetRlocAddress (void) const;

  /**
   * \brief Set the RLOC address.
   * \param rlocAddress The RLOC address
   */
  void SetRlocAddress (const Address &rlocAddress);

  /**
   * \brief Get the metrics associated to the RLOC.
   * \return A pointer to the metrics associated to the RLOC.
   */
  Ptr<RlocMetrics> GetRlocMetrics (void) const;

  /**
   * \brief Set the metrics associated to this RLOC.
   * \param rlocMetrics The metrics associated to the RLOC.
   */
  void SetRlocMetrics (Ptr<RlocMetrics> rlocMetrics);

  /**
   * \brief Print some information about the RLOC.
   * \return The information about the RLOC.
   */
  std::string Print (void);

  /**
   * \brief Serialize the RLOC.
   * \param buf Buffer in which the RLOC will be serialized.
   * \return The size of the buffer after serialization.
   */
  uint8_t Serialize (uint8_t *buf);

  /**
   * \brief Deserialize the RLOC.
   * \param buf Buffer that contains the serialized RLOC.
   * \return
   */
  static Ptr<Locator> Deserialized (const uint8_t *buf);

private:
  Address m_rlocAddress;
  Ptr<RlocMetrics> m_metrics;
};

/**
 * \brief Implementation of an Endpoint ID (EID).
 * This class represents an EID prefix. It is characterized
 * by an address (IPv4 or v6) and by a IPv4 mask or IPv6 prefix.
 */
class EndpointId : public SimpleRefCount<EndpointId>
{
public:
  /**
   * \brief Constructor
   */
  EndpointId ();
  /**
   * \brief Constructor
   */
  EndpointId (const Address &eidAddress);

  /**
   * \brief Constructor
   *
   * \param eidAddress an address (type IPv4)
   * \param mask The IPv4 mask associated to the prefix.
   */
  EndpointId (const Address &eidAddress, const Ipv4Mask &mask);

  /**
   * \brief Constructor
   *
   * \param eidAddress An address (type IPv6)
   * \param prefix The IPv6 prefix associated to this address.
   */
  EndpointId (const Address &eidAddress, const Ipv6Prefix &prefix);

  /**
   * \brief Destructor
   */
  ~EndpointId ();

  /**
   * \brief Get the EID address.
   * \return The EID address
   */
  Address GetEidAddress (void) const;
  /**
   * \brief Set the EID address.
   * \param eidAddress The EID address
   */
  void SetEidAddress (const Address &eidAddress);


  /**
   * \brief Get the IPv4 mask.
   * \return The IPv4 mask.
   */
  Ipv4Mask GetIpv4Mask () const;

  /**
   * \brief Set the IPv4 mask.
   * \param mask The IPv4 mask.
   */
  void SetIpv4Mask (const Ipv4Mask &mask);

  /**
   * \brief Set the IPv6 prefix
   * \param prefix The IPv6 prefix.
   */
  void SetIpv6Prefix (const Ipv6Prefix &prefix);

  /**
   * \brief Get the IPv6 prefix.
   * \return The IPv6 prefix of this EID prefix.
   */
  Ipv6Prefix GetIpv6Prefix (void) const;


  /**
   * This methods checks if the EID prefix is an IPv4 one.
   *
   * \returns True if the prefix is an IPv4 prefix, false otherwise (if
   * it is IPv6 prefix).
   */
  bool IsIpv4 (void) const;

  /**
   * \brief This method returns a character string representation of this EID prefix.
   *
   * \return A character string representing the EID
   */
  std::string Print (void) const;

  /**
   * This method serializes the EID prefix.
   *
   * \param buf The buffer that must contain the serialized EID prefix.
   *
   * \return The size of the buffer after the serialization of the EID prefix.
   */
  uint8_t Serialize (uint8_t buf[33]) const;

  /**
   * This method return the size of the buffer in which
   * the EID prefix is serialized to.
   *
   * \return The size of the buffer in which the EID prefix is serialized to.
   */
  uint8_t GetSerializedSize (void) const;

  /**
   * This method deserializes the EID from the buffer given as an
   * argument.
   *
   * \param buf The uint8_t array that contains the serialized representation
   * of the EID prefix.
   * \return A pointer to an EndpointId object. It points to the
   *  deserialized object
   */
  static Ptr<EndpointId> Deserialize (const uint8_t *buf);
private:
  Address m_eidAddress;
  Ipv4Mask m_mask;
  Ipv6Prefix m_prefix;
};

} /* namespace ns3 */

#endif /* LISP_H_ */
