/*
 * subscribe-list.cc
 *
 *  Created on: 15 nov. 2021
 *  Author: Tom Piron
 */

#include "ns3/subscribe-list.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SubscribeList");

bool SubscribeEntry::SubscriberKeyCompare::operator() (const SubscriberKey& lhs, const SubscriberKey& rhs) const
{
  if (lhs.xtrId != rhs.xtrId)
    {
      return lhs.xtrId < rhs.xtrId;
    }
  else
    {
      return lhs.siteId < rhs.siteId;
    }
}

SubscribeEntry::SubscribeEntry ()
{
}

void SubscribeEntry::AddSubscriber (const Locator &subscriber, uint128_t xtrId, uint64_t siteId)
{
  SubscriberKey key;
  key.xtrId = xtrId;
  key.siteId = siteId;

  auto old_locator = this->m_subscribers.find (key);
  if (old_locator != this->m_subscribers.end ())
    {
      old_locator->second = subscriber;
    }
  else
    {
      this->m_subscribers.insert ({key, subscriber});
    }
}

std::vector<Locator> SubscribeEntry::GetSubscribers (void) const
{
  std::vector<Locator> result;
  for (auto iter = this->m_subscribers.cbegin (); iter != this->m_subscribers.cend (); iter++)
    {
      result.push_back (iter->second);
    }
  return result;
}

void SubscribeEntry::RemoveSubscriber (uint128_t xtrId, uint64_t siteId)
{
  SubscriberKey key;
  key.xtrId = xtrId;
  key.siteId = siteId;

  this->m_subscribers.erase (key);
}

SubscribeList::SubscribeList ()
{
}

Ptr<SubscribeEntry> SubscribeList::CreateEntry (const EndpointId &eidRange)
{
  auto iter = this->m_list.find (eidRange);
  Ptr<SubscribeEntry> entry;
  if (iter != this->m_list.end ())
    {
      entry = iter->second;
    }
  else
    {
      entry = Create<SubscribeEntry>();
      this->m_list.insert ({eidRange, entry});
    }
  return entry;
}

Ptr<SubscribeEntry> SubscribeList::GetEntry (const EndpointId &eid)
{
  Ptr<SubscribeEntry> bestEntry = 0;
  uint16_t bestLength = 0;

  for (auto iter = this->m_list.begin (); iter != this->m_list.end (); iter++)
    {
      EndpointId entryKey = iter->first;
      if (eid.IsIncludedIn (entryKey))
        {
          uint16_t maskLength = entryKey.GetMaskLength ();
          if (maskLength > bestLength)
            {
              bestLength = maskLength;
              bestEntry = iter->second;
            }
        }
    }

  return bestEntry;
}

void SubscribeList::RemoveEntry (const EndpointId &eidRange)
{
  this->m_list.erase (eidRange);
}

bool SubscribeList::EndpointIdCompare::operator() (const EndpointId& lhs, const EndpointId& rhs) const
{
  if (lhs.IsIpv4 () != rhs.IsIpv4 ())
    {
      return lhs.IsIpv4 ();
    }
  else if (lhs.GetEidAddress () != rhs.GetEidAddress ())
    {
      return lhs.GetEidAddress () < rhs.GetEidAddress ();
    }
  else
    {
      return lhs.GetMaskLength () < rhs.GetMaskLength ();
    }
}

} /* namespace ns3 */
