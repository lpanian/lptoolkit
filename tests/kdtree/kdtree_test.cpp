#include <iostream>
#include <cmath>
#include "toolkit/vec.hh"
#include "toolkit/kdtree.hh"

int main()
{
	std::mt19937 gen(1234);
	std::normal_distribution<float> d(10.f, 5.f);

	static const int kNumPoints = 10000;
	std::vector<v3f> points(kNumPoints);
	for(int i = 0; i < kNumPoints; ++i)
		points[i].Set(d(gen), d(gen), d(gen));

	KdTree<v3f> kd;
	kd.Init(&points[0], points.size(), 2);

	int badCount = 0;
	static const int kNumTests = 1000;
	for(int test = 0; test < kNumTests; ++test)
	{
		v3f testPt(d(gen), d(gen), d(gen));
		const int nearest = kd.Nearest(testPt);
		const float nearestD2 = LengthSq(kd.GetPoint(nearest) - testPt);

		float best = std::numeric_limits<float>::max();
		int bestIdx = -1;
		for(int i = 0; i < kd.GetNumPoints(); ++i)
		{
			const float d2 = LengthSq(kd.GetPoint(i) - testPt);
			if(d2 > 0.f && d2 < best) {
				best = d2;
				bestIdx = i;
			}
		}

		if(nearest != bestIdx && best != nearestD2) {
			std::cerr << "Test pt " << test << " kdnearest: " << nearest << " but brute force nearest: " << bestIdx << std::endl;
			std::cerr << "    testPt " << testPt << std::endl
				<< "    nearestDist2 " << nearestD2 << std::endl
				<< "    brute D2 " << best << std::endl;
			++badCount;
		}
	}

	if(badCount > 0) {
		std::cerr << badCount << " points do not match brute force nearest." << std::endl;
	} else {
		std::cout << "Success!" << std::endl;
	}

	return 0;
}
