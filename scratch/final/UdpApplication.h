#ifndef UDP_APP_GUARD
#define UDP_APP_GUARD

#include "Ipv4Topology.h"

void InstallUdpApplication (Ptr<Ipv4TopologyNode> source, Ptr<Ipv4TopologyNode> destination,
                            Time start, Time end, uint16_t port = 5000);
void InstallUdpReceiver (Ptr<Ipv4TopologyNode> node, Time start, Time end, uint16_t port = 5000);
void InstallUdpSender (Ptr<Ipv4TopologyNode> node, Ptr<Ipv4TopologyNode> destination, Time start, Time end, uint16_t port = 5000);

#endif
