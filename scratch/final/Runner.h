#ifndef GUARD_RUNNER
#define GUARD_RUNNER

class Runner
{
private:
  int m_nRuns, m_seed, m_nSender, m_nReceiver;

public:
  Runner (int nRuns, int seed, int nSender, int nReceiver);
  int Run ();
};

#endif
