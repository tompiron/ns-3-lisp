#include "SmrProxyExperiment.h"

NS_LOG_COMPONENT_DEFINE ("SmrProxyExperiment");

SmrProxyExperiment::SmrProxyExperiment (int run, int nSender, int nReceiver) : SmrExperiment (run, nSender, nReceiver)
{
  this->m_prefix = "-smrproxy";
}

void SmrProxyExperiment::SetConfig ()
{
  this->SmrExperiment::SetConfig ();
  Config::SetDefault ("ns3::LispEtrItrApplication::EnableProxyMode", BooleanValue (true));
}
