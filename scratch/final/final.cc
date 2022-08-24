#include <string>

#include "ns3/core-module.h"

#include "Runner.h"

#define N_ARGS 4

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Main");

std::string usage ()
{
  return "Usage: NS_LOG=\"*=level_error\" ./waf --run final --command-template '\%s number_of_runs seed number_of_sender number_of_receiver'";
}

int main (int argc, char const *argv[])
{
  LogComponentEnable ("Main", LOG_LEVEL_INFO);
  LogComponentEnable ("Runner", LOG_LEVEL_INFO);

  if (argc <= N_ARGS)
    {
      NS_LOG_ERROR ("Invalid number of arguments : got " << argc - 1 << ", expected " << N_ARGS << ".");
      NS_LOG_ERROR (usage ());
      return 1;
    }

  int n_runs, seed, n_sender, n_receiver;
  try {
      n_runs = std::stoi (argv[1]);
      seed = std::stoi (argv[2]);
      n_sender = std::stoi (argv[3]);
      n_receiver = std::stoi (argv[4]);
    }
  catch (std::invalid_argument &error)
    {
      NS_LOG_ERROR ("Invalid argument(s) : number_of_runs, seed, number_of_sender and number_of_receiver must be numbers.");
      NS_LOG_ERROR (usage ());
      return 1;
    }

  if (n_runs < 1 || n_sender < 1 || n_receiver < 1)
    {
      NS_LOG_ERROR ("Invalid argument(s) : number_of_runs, number_of_sender and number_of_receiver must be greater than zero.");
      NS_LOG_ERROR (usage ());
      return 1;
    }

  Runner runner = Runner (n_runs, seed, n_sender, n_receiver);
  return runner.Run ();
}
