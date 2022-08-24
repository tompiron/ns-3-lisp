/*
 * map-entry.h
 *
 *  Created on: Jul 9, 2017
 *      Author: qsong
 */

#ifndef SRC_INTERNET_MODEL_LISP_DATA_PLANE_MAP_ENTRY_H_
#define SRC_INTERNET_MODEL_LISP_DATA_PLANE_MAP_ENTRY_H_

#include "locator.h"
#include "locators.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "ns3/endpoint-id.h"

namespace ns3 {
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
  void SetVersionNumber (uint16_t versionNb);
  uint32_t GetLocsStatusBits (void) const;
  virtual std::string Print (void) const = 0;
  void SetEidPrefix (Ptr<EndpointId> prefix);
  Ptr<EndpointId> GetEidPrefix (void) const;

  void SetTranslatedPort (uint16_t port);
  uint16_t GetTranslatedPort (void) const;
  void SetRtrRloc (Ptr<Locator> rloc);
  Ptr<Locator> GetRtrRloc (void) const;
  void SetXtrLloc (Ptr<Locator> rloc);
  Ptr<Locator> GetXtrLloc (void) const;

  bool IsNatedEntry (void) const;
  /*
   * \returns True, if the entry was registered with the proxy bit. False, otherwise.
   *
   * If this entry is on a map server, return true if the map server should reply directly
   * to a request for this entry. Return false, if it should forward it to the ETR.
   * Return an undefined value when not on a map server.
   */
  bool IsProxyMode (void) const;
  /*
   * \param True to enable proxy mode, False to disable it.
   *
   * If this entry is on a map server, should be set to true if the Register message
   * had the proxy (P) bit set.
   */
  void SetProxyMode (bool value);

  uint32_t GetTTL ();
  uint32_t ReduceTTL ();
  void SetTTL (uint32_t ttl);

protected:
  // TODO add a possible lock to the entry
  Ptr<EndpointId> m_eidPrefix;
  Ptr<Locators> m_locators;

  /* The following members are used only by RTRs to record details about
   * NATed devices, so as to be able to forward packets to them through
   * NAT.
   * If EID is NATed, m_locators will actually be the translated global address used by NAT (in cache)
   */
  uint16_t m_port; // Translated global port used by NAT
  Ptr<Locator> m_rtrRloc; //The RTR own address, that was used in the outer header of the ECMed MapRegister
  Ptr<Locator> m_xTRLocalRloc; // The local xTR RLOC, which is NATed

private:
  // Mapping flags
  bool m_inDatabase;
  bool m_isStatic; // Is manually added
  bool m_isUsable;
  bool m_isExpired;
  bool m_isNegative;
  bool m_useVersioning;
  bool m_useLocatorStatusBits;
  bool m_proxyMode;
  uint32_t m_ttl;

  uint16_t m_mappingVersionNumber; // Version number of the mapping
  uint32_t m_rlocsStatusBits;
  // TODO investigate last time it has been used (to expunge cache)
};

} /* namespace ns3 */

#endif /* SRC_INTERNET_MODEL_LISP_DATA_PLANE_MAP_ENTRY_H_ */
