// High Shelf Filter
// Based on code from gtkGEP: http://gtkgep.prv.pl/

#ifndef __APTK_HIGHSHELF__
#define __APTK_HIGHSHELF__

#include "filter.hh"

/*
Documentation:

* void aptk::HighShelf<T,W>::setup(W cutFreq, W gain, W Q, W sampleRate)
	- Will setup the filters coefficients.

*/

namespace aptk
{
	template <typename T, typename W>
	class HighShelf: public Filter<T, W>
	{
	public:
		void setup(W cutFreq, W gain, W Q, W sampleRate)
		{
			W A, omega, sn, cs, alpha, beta;
			W norm;

			A=pow(10.0f, gain / 40.0f);
			omega=2.0f * PI * cutFreq / sampleRate;
			sn=sin(omega);
			cs=cos(omega);
			alpha=sn / (2.0 * Q);
			beta=sqrt((A * A + 1.0) / Q - ((A - 1.0) * (A - 1.0)));

			norm=(A + 1.0) - (A - 1.0) * cs + beta * sn;
			InCoef(0, (A * ((A + 1.0) + (A - 1.0) * cs + beta * sn)) / norm);
			InCoef(1, (-2.0 * A * ((A - 1.0) + (A + 1.0) * cs)) / norm);
			InCoef(2, (A * ((A + 1.0) + (A - 1.0) * cs - beta * sn)) / norm);
			OutCoef(0, (2.0 * ((A - 1.0) - (A + 1.0) * cs)) / norm);
			OutCoef(1, ((A + 1.0) - (A - 1.0) * cs - beta * sn) / norm);			
		}
	};
};

#endif
