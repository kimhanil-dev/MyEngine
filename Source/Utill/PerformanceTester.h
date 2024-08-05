#pragma once
#include <memory>
#include <functional>
#include <mmsystem.h>
#include <format>
#include <fstream>

using namespace std;

class PerformanceTester
{
private:
	PerformanceTester();
	~PerformanceTester();

public:
	static PerformanceTester* Get(bool bDestroy = false)
	{
		static auto instance = unique_ptr<PerformanceTester,decltype(&PerformanceTester::Deleter)>(new PerformanceTester(), Deleter);
		if (bDestroy)
		{
			instance.reset();
			return nullptr;
		}
		return instance.get();
	}

public:
	void Test(const string& name,const string& tag, const unsigned int testCount, const function<void()> testCase, bool isOnlyAvg = false) const;
private:
	double mSecondsPerCount;

	static void Deleter(PerformanceTester* instance)
	{
		delete instance;
	}
};

