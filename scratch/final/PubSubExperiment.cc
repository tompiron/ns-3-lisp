#include "PubSubExperiment.h"

NS_LOG_COMPONENT_DEFINE ("PubSubExperiment");

PubSubExperiment::PubSubExperiment (int run, int nSender, int nReceiver) : LispExperiment (run, nSender, nReceiver)
{
  this->m_prefix = "-pubsub";
  this->m_mobilityTime = Seconds (35);
}

void PubSubExperiment::SetConfig ()
{
  this->LispExperiment::SetConfig ();

  Config::SetDefault ("ns3::LispEtrItrApplication::EnableProxyMode", BooleanValue (true));
  Config::SetDefault ("ns3::LispEtrItrApplication::EnableSubscribe", BooleanValue (true));
}
