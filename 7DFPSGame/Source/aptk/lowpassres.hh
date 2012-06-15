// Resonant Lowpass
// Coefficient calculation taken from http://www.musicdsp.org

#ifndef __APTK_LOWPASSRES__
#define __APTK_LOWPASSRES__

#include "filter.hh"

/* 
Documentation:

* void aptk::LowpassRes<T,W>::setup(W coff, W res, W sampleRate)
	- Will setup the filters coefficients.

*/

namespace aptk
{
	template <typename T, typename W>
	class LowpassRes: public Filter<T, W>
	{
	public:
		void setup(W coff, W res, W sampleRate)
		{
			W lInitCoef = (W)(1.0f / (tan(PI * coff / sampleRate)));
			W lInitCoefPw2 = lInitCoef * lInitCoef;
			W lResonance = (W)(((sqrt((W)2.0f) - 0.1f) * (1.0f - res)) + 0.1f);

			InCoef(0, (W)(1.0f / (1.0f + (lResonance * lInitCoef) + lInitCoefPw2)));
			InCoef(1, 2.0f * InCoef(0));
			InCoef(2, InCoef(0));

			OutCoef(0, (W)(2.0f * InCoef(0) * (1.0f - lInitCoefPw2)));
			OutCoef(1, (W)(InCoef(0) * (1.0f - lResonance * lInitCoef + lInitCoefPw2)));
		}
	};
};

#endif

