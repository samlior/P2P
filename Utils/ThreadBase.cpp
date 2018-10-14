#include "ThreadBase.h"

#define CREATE_LOCK_GUARD_SINGLE lock_guard<mutex> lock(m_mutexSingle)

#ifdef THREAD_BASE_MULTIPLE_SUPPORT
#define CREATE_LOCK_GUARD_MULTIPLE lock_guard<mutex> lock(m_mutexMultiple)
#else
#define CREATE_LOCK_GUARD_MULTIPLE 
#endif

using namespace std;

void CThreadBase::start()
{
	CREATE_LOCK_GUARD_MULTIPLE;

	if (isRunning())
	{
		return;
	}

	if (!m_pThread)
	{
		{
			CREATE_LOCK_GUARD_SINGLE;
			m_bNeedRunning = true;
		}
		m_pThread = new std::thread(&CThreadBase::loop, this);
	}
}

void CThreadBase::stop()
{
	CREATE_LOCK_GUARD_MULTIPLE;

	if (!isRunning())
	{
		return;
	}

	if (m_pThread)
	{
		{
			CREATE_LOCK_GUARD_SINGLE;
			m_bNeedRunning = false;
		}
		m_pThread->join();
		delete m_pThread;
		m_pThread = nullptr;
	}
}

bool CThreadBase::isRunning()
{
	CREATE_LOCK_GUARD_SINGLE;
	return m_bIsRunning;
}

bool CThreadBase::needRunning()
{
	CREATE_LOCK_GUARD_SINGLE;
	return m_bNeedRunning;
}

void CThreadBase::loop()
{
	{
		CREATE_LOCK_GUARD_SINGLE;
		m_bIsRunning = true;
	}

	while (needRunning())
	{
		if (!run())
			break;
	}

	{
		CREATE_LOCK_GUARD_SINGLE;
		m_bIsRunning = false;
	}
}

bool CThreadBase::threadSleep(long long llTimeEachLoop, int iLoopCount /*= 1*/)
{
	for (; iLoopCount > 0; iLoopCount--)
	{
		this_thread::sleep_for(chrono::milliseconds(llTimeEachLoop));
		if (iLoopCount > 1 && !needRunning())
			return false;
	}
	return true;
}