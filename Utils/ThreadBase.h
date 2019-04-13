#pragma once
#include <thread>
#include <mutex>

class CThreadBase
{
public:
	CThreadBase() :m_pThread(nullptr), m_bIsRunning(false), m_bNeedRunning(false) {}
	virtual ~CThreadBase() { stop(); }

	virtual void start();
	virtual void stop();
	bool isRunning();
	std::thread* getThread() { return m_pThread; }

protected:
	virtual bool run() { return false; }

	bool needRunning();

	bool threadSleep(long long llTimeEachLoop, int iLoopCount = 1);

#ifdef THREAD_BASE_MULTIPLE_SUPPORT
	std::mutex m_mutexMultiple;
#endif
	std::mutex m_mutexSingle;
	std::thread* m_pThread;

private:
	void loop();

	bool m_bIsRunning;
	bool m_bNeedRunning;
};

