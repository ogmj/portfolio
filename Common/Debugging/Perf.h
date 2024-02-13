#pragma once
#include <iostream>
#include"../Util/MemoryPool.h"
#include<unordered_map>
#include<map>
#include"../Util/Singleton.h"
#include<chrono>
#include<numeric>
#include<algorithm>
#include "../../network/include/thread/Sync.h"
#include "../../Common/Console/Console.h"

static CriticalSection perf_cs("perf");

class perf_tag
{
public:
	perf_tag() : m_ProcessTime(0), m_Cnt(0)
	{
	};
	~perf_tag() {};

	std::chrono::microseconds m_ProcessTime;
	int	m_Cnt;
};

class profile_tag
{
public:
	profile_tag(const char * _key, std::chrono::microseconds _time, int _cnt) : m_strName(_key), m_ProcessTime(_time), m_Cnt(_cnt)
	{
	};
	~profile_tag() {};

	std::string m_strName;
	std::chrono::microseconds m_ProcessTime;
	int	m_Cnt;
};

//class perf_mng : public MemoryPool<perf_mng, 1>
class perf_mng
{
public:
	perf_mng()
	{
		m_bSwitch = false;
	}
	~perf_mng() {};
	void add(std::string& _key, std::chrono::microseconds _process_milliseconds)
	{
		THREAD_SYNCHRONIZE_END(perf_cs);
		PerfIt it = m_mapPerf.find(_key);
		if (it != m_mapPerf.end())
		{
			(*it).second.m_ProcessTime += _process_milliseconds;
			(*it).second.m_Cnt += 1;
		}
		else
		{
			//신규 추가
			m_mapPerf[_key].m_ProcessTime = _process_milliseconds;
			m_mapPerf[_key].m_Cnt = 1;
			//_cprint("_key:%s, process_time:%d\n", _key.c_str(), _process_milliseconds);
		}
	}

	void report(const char* _sz_file_name)
	{
		THREAD_SYNCHRONIZE_END(perf_cs);
		//std::sort(m_mapPerf.begin(), m_mapPerf.end());
		for (auto i : m_mapPerf)
		{
			m_TotalTime += i.second.m_ProcessTime;
			//_cprint("report1 %s %d\n", i.first.c_str(), i.second.m_ProcessTime);
			auto it = std::lower_bound(m_vecProfile.begin(), m_vecProfile.end(), i.second.m_ProcessTime, [](profile_tag _tag, std::chrono::microseconds tm)
			{
				return _tag.m_ProcessTime > tm;
			});
			profile_tag pt(i.first.c_str(), i.second.m_ProcessTime, i.second.m_Cnt);
			//_cprint("report2 %s %d\n", pt.m_strName.c_str(), pt.m_ProcessTime);
			if (m_vecProfile.empty() == true)
			{
				m_vecProfile.emplace_back(pt);
			}
			else
			{
				m_vecProfile.insert(it, pt);
			}
		}
		//for (auto i : m_mapPerf)
		//{
		//	auto t = std::chrono::duration_cast<std::chrono::milliseconds>(i.second.m_ProcessTime);
		//}

		//파일로 출력
		log(_sz_file_name);
		m_vecProfile.clear();
		m_TotalTime = std::chrono::microseconds(0);
		m_mapPerf.clear();
	}

	void log(const char* _sz_file_name)
	{
		std::chrono::system_clock::time_point ProfileEndTime = std::chrono::system_clock::now();
		std::chrono::microseconds ProfileTime = std::chrono::duration_cast<std::chrono::microseconds>(ProfileEndTime - m_ProfileStartTime);

		FILE* fp = fopen(_sz_file_name, "a+");
		if (!fp) return;

		THREAD_SYNCHRONIZE_END(perf_cs);
		// print time 
		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(fp, "%04d/%02d/%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		// print header 
		const char* pBar = "--------------------------------------------+-------+-----+--------------------";
		fprintf(fp, "%s\n", pBar);
		fprintf(fp, " function name                                 count ratio   microseconds\n");
		fprintf(fp, "%s\n", pBar);

		// print stuff 
		float f = 0;
		for (auto& i : m_vecProfile)
		{
			f = 0;
			if (m_TotalTime.count())
			{
				f = i.m_ProcessTime * 100 / m_TotalTime;
			}

			fprintf(fp, " %-42s %8d  %5.1f%% %d\n", i.m_strName.c_str(),
				i.m_Cnt,
				f,
				i.m_ProcessTime,
				m_TotalTime);
		}

		// print tail 
		fprintf(fp, "%s\n", pBar);
		char szTmp[256];
		sprintf(szTmp, "%d second %d ms", ProfileTime / 1000000, ProfileTime % 1000000);
		for (int i = 0; i < strlen(pBar) - strlen(szTmp); i++)
		{
			fprintf(fp, " ");
		}
		fprintf(fp, "%s\n\n", szTmp);

		fclose(fp);
	}

	void profile()
	{
		if (m_bSwitch == true)
		{
			m_bSwitch = false;
			report("profile.txt");
		}
		else
		{
			m_bSwitch = true;
			m_ProfileStartTime = std::chrono::system_clock::now();

		}
	}
	bool get_switch()
	{
		return m_bSwitch;
	}


private:
	bool m_bSwitch;
	std::chrono::system_clock::time_point m_ProfileStartTime;

	std::chrono::microseconds m_TotalTime;
	//std::unordered_map<std::string, std::chrono::milliseconds> m_mapPerf;
	//using PerfIt = std::unordered_map<std::string, std::chrono::milliseconds>::iterator;
	//std::map<std::string, std::chrono::milliseconds> m_mapPerf;
	//using PerfIt = std::map<std::string, std::chrono::milliseconds>::iterator;

	std::map<std::string, perf_tag> m_mapPerf;
	using PerfIt = std::map<std::string, perf_tag>::iterator;

	std::vector<profile_tag> m_vecProfile;
	using ProfileIt = std::vector<profile_tag>::iterator;
};

class perf
{
public:
	perf(std::string _key) : m_strKey(_key)
	{
		
		if (Singleton<perf_mng>::GetStaticInstance().get_switch())
		{
			m_tpStartTime = std::chrono::system_clock::now();
		}
	}
	~perf()
	{
		if (Singleton<perf_mng>::GetStaticInstance().get_switch())
		{
			std::chrono::system_clock::time_point tp_process_time = std::chrono::system_clock::now();
			std::chrono::microseconds process_time = std::chrono::duration_cast<std::chrono::microseconds>(tp_process_time - m_tpStartTime);
			//std::cout << process_time.count() << "\n";
			Singleton<perf_mng>::GetStaticInstance().add(m_strKey, process_time);
		}
	}
private:
	std::string m_strKey;
	std::chrono::system_clock::time_point m_tpStartTime;
};


#define PERF(_key) perf _perf(_key);