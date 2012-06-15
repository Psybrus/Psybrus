// To handle a circlular buffer for delay
// Can be tapped as many times as required.

/* 
Documentation:

* aptk::Delay<T,W>::Delay()
	- Will create buffer of a default size. Don't rely on it
	  being large. Defaults to 32k for now.
	  
* aptk::Delay<T,W>::Delay(int size)
	- Will create a buffer of 'size' samples. This is the number
	  of samples in whatever datatype is templated.
	  
* void aptk::Delay<T,W>::pushData(const T& inData)
	- Will push the sample 'inData' onto the circular buffer.

* T aptk::Delay<T,W>::tapData(W delaySamp)
	- This will retrieve a sample of data 'delaySamp's behind the
	  last pushed value. It will wrap automatically, and cannot be
	  negative.
*/	

#ifndef __APTK_DELAY__
#define __APTK_DELAY__

namespace aptk
{
	const int DELAY_BUFFER_SIZE = (1024*32);

	// This class was written by Neil Richardson. Totally his own work.
	template <typename T, typename W>
	class Delay
	{
	public:
		Delay()
		{
			createBuffer(DELAY_BUFFER_SIZE);
		}
	
		Delay(int lengthSamp)
		{
			createBuffer(lengthSamp);
		}

		~Delay()
		{
			if (mpBufferStart)
				delete mpBufferStart;
		}
		
		void pushData(const T& inData)
		{
			*mpBufferPos = inData;
			mpBufferPos++;
			
			if (mpBufferPos == mpBufferEnd)
				mpBufferPos = mpBufferStart;
		}
		
		T tapData(W delaySamp) const
		{
			// Ergh. Calculate the integer we are using to
			// jump to a certain point to read from, and the
			// extra floating point info we need to interpolate.
			int delaySampAbs = (int)delaySamp;
			W delaySampLerp = delaySamp - (W)delaySampAbs;

			// The two points we will be interpolating between
			// so we get a smoothed out result
			T* lpDataPointA = (mpBufferPos - delaySampAbs) - 2;
			T* lpDataPointB = (mpBufferPos - delaySampAbs) - 1;
			
			// Pull them into a valid range
			while (lpDataPointA < mpBufferStart)
				lpDataPointA += (mpBufferEnd - mpBufferStart);

			while (lpDataPointB < mpBufferStart)
				lpDataPointB += (mpBufferEnd - mpBufferStart);

			// Now to return the value				
			return (*lpDataPointB + ((*lpDataPointA - *lpDataPointB) * delaySampLerp));
		}
	
	private:
		void createBuffer(int size)
		{
			mpBufferStart = new W[size];
			mpBufferPos = mpBufferStart;			
			mpBufferEnd = mpBufferStart + size;

			for(register int i = 0; i < size; ++i)
				mpBufferStart[i] = 0;		
		}
		
	private:
		T* mpBufferStart;
		T* mpBufferEnd;
		T* mpBufferPos;
	};
}

#endif
