/*
 * MapTables.h
 *
 *  Created on: 28 janv. 2016
 *      Author: lionel
 */

#ifndef MAPTABLES_H_
#define MAPTABLES_H_

#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "lisp-protocol.h"
#include "lisp-over-ip.h"

namespace ns3 {

class LispProtocol;
class LispOverIp;
class Locators;
class MapEntry;
class Address;
class LispHeader;

class MapTables : public Object
{
public:
  static TypeId GetTypeId (void);
  MapTables ();
  virtual ~MapTables ();

  virtual Ptr<MapEntry> DatabaseLookup (const Address &eid) = 0;

  virtual Ptr<MapEntry> CacheLookup (const Address &eid) = 0;

  enum MapEntryLocation
  {
    IN_DATABASE = 0,
    IN_CACHE = 1
  };

  virtual void SetEntry (const Address &eid, const Ipv4Mask &mask, Ptr<MapEntry> mapEntry, MapEntryLocation location) = 0;
  virtual void SetEntry (const Address &eid, const Ipv6Prefix &prefix, Ptr<MapEntry> mapEntry, MapEntryLocation location) = 0;

  virtual void InsertLocator (const Ipv4Address &eid, const Ipv4Mask &mask, const Ipv4Address &rlocAddress, uint8_t priority, uint8_t weight, MapEntryLocation location, bool reachable) = 0;
  virtual void InsertLocator (const Ipv4Address &eid, const Ipv4Mask &mask, const Ipv6Address &rlocAddress, uint8_t priority, uint8_t weight, MapEntryLocation location, bool reachable) = 0;
  virtual void InsertLocator (const Ipv6Address &eid, const Ipv6Prefix &prefix, const Ipv4Address &rlocAddress, uint8_t priority, uint8_t weight, MapEntryLocation location, bool reachable) = 0;
  virtual void InsertLocator (const Ipv6Address &eid, const Ipv6Prefix &prefix, const Ipv6Address &rlocAddress, uint8_t priority, uint8_t weight, MapEntryLocation location, bool reachable) = 0;

  virtual Ptr<Locator> DestinationRlocSelection (Ptr<const MapEntry> remoteMapEntry) = 0;

  virtual Ptr<Locator> SourceRlocSelection (Address const &srcEid, Ptr<const Locator> destLocator) = 0;

  virtual void MapRequest (void) = 0;

  virtual void MapFree (void) = 0;

  virtual bool IsMapForReceivedPacket (Ptr <const Packet> p, const LispHeader &header, const Address &srcRloc, const Address &destRloc) = 0;

  virtual void GetMapEntryList (MapEntryLocation location, std::list<Ptr<MapEntry> > &entryList) = 0;

  // TODO Add map_notify, map_check_lsbits ?,

  /**
   * Lisp Protocol associated to this MapTables
   * @return
   */
  Ptr<LispOverIp> GetLispOverIp (void);
  void SetLispOverIp (Ptr<LispOverIp> lispProtocol);

  void DbMiss (void);
  void DbHit (void);
  void CacheHit (void);
  void CacheMiss (void);

private:
  Ptr<LispOverIp> m_lispProtocol;

  uint32_t m_dbMiss;    // # failed lookups in db
  uint32_t m_dbHit;     // # successful lookups in db
  uint32_t m_cacheMiss; // # failed lookups in cache
  uint32_t m_cacheHit;  // # successful lookups in cache
};

/**
 *
 */
class Locators : public SimpleRefCount<Locators>
{

public:
  Locators ();
  virtual ~Locators ();

  virtual Ptr<Locator> FindLocator (const Address &address) const = 0;
  virtual void InsertLocator (Ptr<Locator> locator) = 0;
  virtual Ptr<Locator> GetLocatorByIdx (uint8_t locIndex) = 0;
  virtual uint8_t GetNLocators (void) const = 0;
  virtual Ptr<Locator> SelectFirsValidRloc (void) const = 0;
  virtual std::string Print (void) const = 0;

  virtual int Serialize (uint8_t *buf) = 0;

};

/**
 *
 */
class MapEntry : public SimpleRefCount<MapEntry>
{

  static const uint8_t MAX_RLOCS;

public:
  MapEntry ();
  virtual ~MapEntry ();

  virtual Ptr<Locator> FindLocator (const Address &address) const = 0;
  virtual Ptr<Locator> RlocSelection (void) const = 0;
  void InsertLocator (Ptr<Locator> locator);
  Ptr<Locators> GetLocators (void);
  void SetLocators (Ptr<Locators> locators);
  bool IsUsingVersioning (void) const;
  void SetIsUsingVersioning (bool is);
  bool IsUsingLocStatusBits (void) const;
  void SetIsUsingLocStatusBits (bool is);
  bool IsNegative (void) const;
  void setIsNegative (bool isNegative);
  uint16_t GetVersionNumber (void) const;
  uint32_t GetLocsStatusBits (void) const;
  virtual std::string Print (void) const = 0;
  void SetEidPrefix (Ptr<EndpointId> prefix);
  Ptr<EndpointId> GetEidPrefix (void);

protected:
  // TODO add a possible lock to the entry
  Ptr<EndpointId> m_eidPrefix;
  Ptr<Locators> m_locators;

private:
  // Mapping flags
  bool m_inDatabase;
  bool m_isStatic; // Is manually added
  bool m_isUsable;
  bool m_isExpired;
  bool m_isNegative;
  bool m_useVersioning;
  bool m_useLocatorStatusBits;

  uint16_t m_mappingVersionNumber; // Version number of the mapping
  uint32_t m_rlocsStatusBits;
  // TODO investigate last time it has been used (to expunge cache)
};

} /* namespace ns3 */

#endif /* MAPTABLES_H_ */
