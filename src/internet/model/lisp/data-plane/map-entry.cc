/*
 * map-entry.cc
 *
 *  Created on: Jul 9, 2017
 *      Author: qsong
 */

#include "map-entry.h"
#include "ns3/log.h"
#include "ns3/assert.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MapEntry");
/*MapEntry */

const uint8_t MapEntry::MAX_RLOCS = 32;

MapEntry::MapEntry ()
{
  m_useVersioning = false;
  m_useLocatorStatusBits = false;
  m_mappingVersionNumber = 0;
  m_rlocsStatusBits = 0;
  m_isNegative = 0;
  m_isExpired = 0;
  m_port = 0;
  m_ttl = 0;
}

MapEntry::~MapEntry ()
{

}

Ptr<Locators> MapEntry::GetLocators (void)
{
  return m_locators;
}

void MapEntry::SetLocators (Ptr<Locators> locators)
{
  m_locators = locators;
}

bool MapEntry::IsUsingVersioning (void) const
{
  return m_useVersioning;
}

bool MapEntry::IsUsingLocStatusBits (void) const
{
  return m_useLocatorStatusBits;
}

uint16_t MapEntry::GetVersionNumber (void) const
{
  return m_mappingVersionNumber;
}

void MapEntry::SetVersionNumber (uint16_t versionNb)
{
  m_mappingVersionNumber = versionNb;
}

uint32_t MapEntry::GetLocsStatusBits (void) const
{
  return m_rlocsStatusBits;
}

void MapEntry::InsertLocator (Ptr<Locator> locator)
{
  if (m_locators->GetNLocators () == MAX_RLOCS)
    {
      NS_LOG_ERROR ("[MapEntry] The locators chain is full!");
      return;
    }

  if (!GetLocators ()->FindLocator (locator->GetRlocAddress ()))
    {
      m_locators->InsertLocator (locator);
    }
  // TODO Add error message
  else
    {
      NS_LOG_ERROR ("[MapEntry] The locator already exists!");
      return; // the locator already exists in the table
    }
}

bool MapEntry::IsNegative (void) const
{
  return m_isNegative;
}

void MapEntry::setIsNegative (bool isNegative)
{
  m_isNegative = isNegative;
}

void MapEntry::SetEidPrefix (Ptr<EndpointId> prefix)
{
  m_eidPrefix = prefix;
}
Ptr<EndpointId> MapEntry::GetEidPrefix (void) const
{
  return m_eidPrefix;
}

void
MapEntry::SetTranslatedPort (uint16_t port)
{
  m_port = port;
}
uint16_t
MapEntry::GetTranslatedPort (void) const
{
  return m_port;
}
void
MapEntry::SetRtrRloc (Ptr<Locator> rloc)
{
  m_rtrRloc = rloc;
}
Ptr<Locator>
MapEntry::GetRtrRloc (void) const
{
  return m_rtrRloc;
}
void
MapEntry::SetXtrLloc (Ptr<Locator> rloc)
{
  m_xTRLocalRloc = rloc;
}
Ptr<Locator>
MapEntry::GetXtrLloc (void) const
{
  return m_xTRLocalRloc;
}

bool
MapEntry::IsNatedEntry (void) const
{
  return m_port != 0;
}

uint32_t MapEntry::GetTTL ()
{
  return m_ttl;
}

uint32_t MapEntry::ReduceTTL ()
{
  if (m_ttl != 0)
    {
      return --m_ttl;
    }
  return 0;
}

void MapEntry::SetTTL (uint32_t ttl)
{
  m_ttl = ttl;
}

} /* namespace ns3 */
