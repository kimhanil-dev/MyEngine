#include "pch.h"
#include "PerformanceTester.h"

PerformanceTester::PerformanceTester()
{
	__int64 performFreq = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&performFreq);
	mSecondsPerCount = 1.0 / performFreq;
}

PerformanceTester::~PerformanceTester()
{
}

void PerformanceTester::Test(const string& name, const string& tag, const unsigned int testCount, const function<void()> testCase, bool isOnlyAvg) const
{
	__int64 startCount = 0;
	__int64 endCount = 0;

	ofstream ofs = ofstream(name +".txt",std::ios_base::app);
	_ASSERT(ofs.is_open());
	

	double totalTime = 0.0;
	double deltaTime = 0.0;

	for (unsigned int i = 0; i < testCount; ++i)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&startCount);
		testCase();
		QueryPerformanceCounter((LARGE_INTEGER*)&endCount);

		deltaTime = (endCount - startCount) * mSecondsPerCount;
		totalTime += deltaTime;

		if(!isOnlyAvg)
			ofs << format("[{}]: [time: {}]\n", i, deltaTime);
	}

	ofs << format("{}: Count: {},  Avg : {}\n", tag, testCount,totalTime);

	ofs.close();
}
