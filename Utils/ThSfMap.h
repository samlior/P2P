#pragma once
#include <map>
#include <mutex>

template<typename TKey, typename TValue>
class CThSfMap
{
public:
	CThSfMap(std::map<TKey, TValue>* pContainer) :m_pThSfMapContainer(pContainer) {}
	CThSfMap() :m_pThSfMapContainer(new std::map<TKey, TValue>) {}
	virtual ~CThSfMap()
	{
		delete m_pThSfMapContainer;
		m_pThSfMapContainer = nullptr;
	}

	size_t size()
	{
		std::lock_guard<std::mutex> lock(m_mtxThSfMap);
		return m_pThSfMapContainer->size();
	}

	void add(const TKey& key, const TValue& value) { add(std::pair<TKey, TValue>(key, value)); }

	void add(const std::pair<TKey, TValue>& pr)
	{
		std::lock_guard<std::mutex> lock(m_mtxThSfMap);
		m_pThSfMapContainer->insert(pr);
	}

	bool get(const TKey& key, TValue& value) 
	{
		bool bRet = false;
		if (!m_pThSfMapContainer->empty())
		{
			std::lock_guard<std::mutex> lock(m_mtxThSfMap);
			auto itr = m_pThSfMapContainer->find(key);
			if (itr != m_pThSfMapContainer->end())
			{
				bRet = true;
				value = itr->second;
				m_pThSfMapContainer->erase(itr);
			}
		}
		return bRet;
	}

	bool remove(const TKey& key)
	{
		bool bRet = false;
		if (!m_pThSfMapContainer->empty())
		{
			std::lock_guard<std::mutex> lock(m_mtxThSfMap);
			auto itr = m_pThSfMapContainer->find(key);
			if (itr != m_pThSfMapContainer->end())
			{
				bRet = true;
				m_pThSfMapContainer->erase(itr);
			}
		}
		return bRet;
	}

	bool find(const TKey& key)
	{
		bool bRet = false;
		if (!m_pThSfMapContainer->empty())
		{
			std::lock_guard<std::mutex> lock(m_mtxThSfMap);
			bRet = m_pThSfMapContainer->find(key) != m_pThSfMapContainer->end();
		}
		return bRet;
	}

	bool find(const TKey& key, TValue& value)
	{
		bool bRet = false;
		if (!m_pThSfMapContainer->empty())
		{
			std::lock_guard<std::mutex> lock(m_mtxThSfMap);
			auto itr = m_pThSfMapContainer->find(key);
			if (itr != m_pThSfMapContainer->end())
			{
				bRet = true;
				value = itr->second;
			}
		}
		return bRet;
	}

	bool findByValue(const TValue& value)
	{
		bool bRet = false;
		if (!m_pThSfMapContainer->empty())
		{
			std::lock_guard<std::mutex> lock(m_mtxThSfMap);
			for (auto itr = m_pThSfMapContainer->begin(); itr != m_pThSfMapContainer->end(); ++itr)
			{
				if (itr->second == value)
				{
					bRet = true;
				}
			}
		}
		return bRet;
	}

	bool findByValue(const TValue& value, TKey& key)
	{
		bool bRet = false;
		if (!m_pThSfMapContainer->empty())
		{
			std::lock_guard<std::mutex> lock(m_mtxThSfMap);
			for (auto itr = m_pThSfMapContainer->begin(); itr != m_pThSfMapContainer->end(); ++itr)
			{
				if (itr->second == value)
				{
					key = itr->first;
					bRet = true;
				}
			}
		}
		return bRet;
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(m_mtxThSfMap);
		m_pThSfMapContainer->clear();
	}

protected:
	std::mutex m_mtxThSfMap;
	std::map<TKey, TValue>* m_pThSfMapContainer;
};