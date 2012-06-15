// Resonant Highpass
// Coefficient calculation taken from http://www.musicdsp.org

#ifndef __APTK_HIGHPASSRES__
#define __APTK_HIGHPASSRES__

#include "filter.hh"

/*
Documentation:

* void aptk::HighpassRes<T,W>::setup(W coff, W res, W sampleRate)
	- Will setup the filters coefficients.

*/

namespace aptk
{
	template <typename T, typename W>
	class HighpassRes: public Filter<T, W>
	{
	public:
		void setup(W coff, W res, W sampleRate)
		{
			W lInitCoef = (W)((tan(PI * coff / sampleRate)));
			W lInitCoefPw2 = lInitCoef * lInitCoef;
			W lResonance = (W)(((sqrt((W)2.0f) - 0.1f) * (1.0f - res)) + 0.1f);

			InCoef(0, (W)(1.0f / (1.0f + (lResonance * lInitCoef) + lInitCoefPw2)));
			InCoef(1, -2.0f * InCoef(0));
			InCoef(2, InCoef(0));

			OutCoef(0, (W)(2.0f * InCoef(0) * (lInitCoefPw2 - 1.0f)));
			OutCoef(1, (W)(InCoef(0) * (1.0f - lResonance * lInitCoef + lInitCoefPw2)));
		}
	};
};

#endif
