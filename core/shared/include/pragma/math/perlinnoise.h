#ifndef __PERLINNOISE_H__
#define __PERLINNOISE_H__

#include <mathutil/glmutil.h>
#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>

class DLLNETWORK PerlinNoise
{
private:
	std::vector<int> m_permutation;
	double Fade(double t);
	double Lerp(double t,double a,double b);
	double Grad(int hash,double x,double y,double z);
public:
	PerlinNoise();
	PerlinNoise(unsigned int seed);
	double GetNoise(double x,double y,double z);
	double GetNoise(Vector3 &v);
};

#endif