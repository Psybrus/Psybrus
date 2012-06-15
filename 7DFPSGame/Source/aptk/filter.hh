// Nice simple filter which does nothing by itself

#ifndef __APTK_FILTER__
#define __APTK_FILTER__

#ifndef PI
#define PI  3.14159265
#endif

/*
Documentation:

* T aptk::Filter<T,W>::procData(const T& inData)
	- This will process the data passed by 'inData' using
	  the filter. By default it will probably return a crap
	  value.
	  
* void aptk::Filter<T,W::reset()
	- Will reset the internal buffers to zero to prepare for
	  new incoming data.
	 
*/

namespace aptk
{
	template <typename T, typename W>
	class Filter
	{
	public:
		Filter()
		{
			reset();
		}

		T procData(const T& inData)
		{
			register T lNewSample;
	
			// Calculate the new sample
			lNewSample = (T)(mInCoef[0] * inData +
			                 mInCoef[1] * mInBuf[0] +
			                 mInCoef[2] * mInBuf[1] -
			                 mOutCoef[0] * mOutBuf[0] -
			                 mOutCoef[1] * mOutBuf[1]);
	
			// Shift along the input buffer
			mInBuf[1] = mInBuf[0];
			mInBuf[0] = (W)inData;
	
			// Shift along the output buffer
			mOutBuf[1] = mOutBuf[0];
			mOutBuf[0] = lNewSample;
			
			return lNewSample;
		}
		
		void reset()
		{
			mInBuf[0] = 0;
			mInBuf[1] = 0;
			mOutBuf[0] = 0;
			mOutBuf[1] = 0;
			
			mInCoef[0] = 1;
			mInCoef[1] = 0;
			mInCoef[2] = 0;
			mOutCoef[0] = 0;
			mOutCoef[1] = 0;
		}

		void lerp( const Filter& A, const Filter& B, W Value )
		{
			mInCoef[0] = A.mInCoef[0] + ( B.mInCoef[0] - A.mInCoef[0] ) * Value;
			mInCoef[1] = A.mInCoef[1] + ( B.mInCoef[1] - A.mInCoef[1] ) * Value;
			mInCoef[2] = A.mInCoef[2] + ( B.mInCoef[2] - A.mInCoef[2] ) * Value;
			mOutCoef[0] = A.mOutCoef[0] + ( B.mOutCoef[0] - A.mOutCoef[0] ) * Value;
			mOutCoef[1] = A.mOutCoef[1] + ( B.mOutCoef[1] - A.mOutCoef[1] ) * Value;
		}
		
	protected:
		// These accessors are only because
		// the compiler bitched at me since
		// templating and inhertance apparently
		// don't mix too well. Bah!
		void InCoef(const int i, const W v)
		{
			mInCoef[i] = v;
		}
	
		void OutCoef(const int i, const W v)
		{
			mOutCoef[i] = v;
		}

		W InCoef(const int i) const
		{
			return mInCoef[i];
		}
	
		W OutCoef(const int i) const
		{
			return mOutCoef[i];
		}
			
	private:	
		W mInCoef[3];
		W mOutCoef[2];
		W mInBuf[2];
		W mOutBuf[2];
	};
}
	
#endif
