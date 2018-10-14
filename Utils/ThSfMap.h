#pragma once
#include <map>
#include <mutex>


template<typename TKey, typename TValue>
class CThSfMap
{
public:
	CThSfMap(std::map<TKey, TValue>* pContainer) :m_pThSfMapContainer(pContainer) {}
	CThSfMap() :m_pThSfMapContainer(new std::map<TKey, TValue>) {}
	~CThSfMap()
	{
		delete m_pThSfMapContainer;
		m_pThSfMapContainer = nullptr;
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

	bool find(const TValue& value)
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
					break;
				}
			}
		}
		return bRet;
	}

	bool find(const TValue& value, TKey& key)
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
					key = itr->first;
					break;
				}
			}
		}
		return bRet;
	}

protected:
	std::mutex m_mtxThSfMap;
	std::map<TKey, TValue>* m_pThSfMapContainer;
};