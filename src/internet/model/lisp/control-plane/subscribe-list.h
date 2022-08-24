/*
 * subscribe-list.h
 *
 *  Created on: 15 nov. 2021
 *  Author: Tom Piron
 */

#ifndef SUBSCRIBE_LIST_H
#define SUBSCRIBE_LIST_H

#include <vector>
#include <map>
#include "ns3/locators.h"
#include "ns3/endpoint-id.h"
#include "ns3/ptr.h"

namespace ns3 {

typedef unsigned __int128 uint128_t;

/*
   * \brief Single Entry of the subscription list.
   *
   * Entry containing the subscriber of a specific EIDs range.
   */
class SubscribeEntry : public SimpleRefCount<SubscribeEntry>
{
public:
  SubscribeEntry ();

  void AddSubscriber (const Locator &subscriber, uint128_t xtrId, uint64_t siteId);
  std::vector<Locator> GetSubscribers (void) const;
  void RemoveSubscriber (uint128_t xtrId, uint64_t siteId);
protected:
  class SubscriberKey : public SimpleRefCount<SubscriberKey>
  {
public:
    uint128_t xtrId;
    uint64_t siteId;
  };

  struct SubscriberKeyCompare
  {
    bool operator() (const SubscriberKey& lhs, const SubscriberKey& rhs) const;
  };

  std::map<SubscriberKey, Locator, SubscriberKeyCompare> m_subscribers;
};

/*
   * \brief Data structure containing subscription information.
   *
   * SubscribeList maps EIDs ranges to a SubscribeEntry.
   * It allows to store and retreive relevant information for
   * the Publish-Subscribe mecanism.
   */
class SubscribeList : public SimpleRefCount<SubscribeList>
{
public:
  SubscribeList ();

  Ptr<SubscribeEntry> CreateEntry (const EndpointId &eidRange);
  Ptr<SubscribeEntry> GetEntry (const EndpointId &eid);
  void RemoveEntry (const EndpointId &eidRange);
protected:
  struct EndpointIdCompare
  {
    bool operator() (const EndpointId& lhs, const EndpointId& rhs) const;
  };

  std::map<EndpointId, Ptr<SubscribeEntry>, EndpointIdCompare> m_list;
};

} /* namespace ns3 */

#endif /* SUSBSCRIBE_LIST_H */
