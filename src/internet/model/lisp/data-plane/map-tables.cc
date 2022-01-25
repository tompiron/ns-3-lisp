/*
 * MapTables.cc
 *
 *  Created on: 28 janv. 2016
 *      Author: lionel
 */

#include "map-tables.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MapTables");

NS_OBJECT_ENSURE_REGISTERED (MapTables);

// MapTables
TypeId
MapTables::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::MapTables")
    .SetParent<Object> ()
    .SetGroupName ("Lisp")
  ;
  return tid;
}

MapTables::MapTables (void)
  : m_dbMiss (0),
  m_dbHit (0),
  m_cacheMiss (0),
  m_cacheHit (0)
{
  NS_LOG_FUNCTION (this);
}

MapTables::~MapTables (void)
{
  NS_LOG_FUNCTION (this);
}

Ptr<LispOverIp> MapTables::GetLispOverIp (void)
{
  return m_lispProtocol;
}

void MapTables::SetLispOverIp (Ptr<LispOverIp> lispProtocol)
{
  m_lispProtocol = lispProtocol;
}

void MapTables::DbMiss (void)
{
  m_dbMiss++;
}

void MapTables::DbHit (void)
{
  m_dbHit++;
}

void MapTables::CacheHit (void)
{
  m_cacheHit++;
}
void MapTables::CacheMiss (void)
{
  m_cacheMiss++;
}


// Locators

Locators::Locators ()
{

}
Locators::~Locators ()
{

}

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
Ptr<EndpointId> MapEntry::GetEidPrefix (void)
{
  return m_eidPrefix;
}
} /* namespace ns3 */
