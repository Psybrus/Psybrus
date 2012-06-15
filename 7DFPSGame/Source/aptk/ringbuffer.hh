// To handle a circlular buffer for general purpose.
// Performs interpolation itself so beware.

#ifndef __APTK_RINGBUFFER__
#define __APTK_RINGBUFFER__

#define PI 3.14159
namespace aptk
{
	// Optimal size for us.
	const int RINGBUFFER_BUFFER_SIZE = 4096;

	// This class was written by Neil Richardson. Totally his own work.
	template <typename T, typename W>
	class Ringbuffer
	{
	public:
		Ringbuffer()
		{
			createBuffer(RINGBUFFER_BUFFER_SIZE);
		}
	
		Ringbuffer(int lengthSamp)
		{
			createBuffer(lengthSamp);
		}

		~Ringbuffer()
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
		
		T readData_(W ringbufferSize, W& lRingPos)
		{
			// Ergh. Calculate the integer we are using to
			// jump to a certain point to read from, and the
			// extra floating point info we need to interpolate.
			int ringbufferSampAbs = (int)lRingPos;
			W ringbufferSampLerp = lRingPos - (W)ringbufferSampAbs;

			// The two points we will be interpolating between
			// so we get a smoothed out result
			T* lpDataPointA = (mpBufferStart + ringbufferSampAbs) - 2;
			T* lpDataPointB = (mpBufferStart + ringbufferSampAbs) - 1;
			
			// Pull them into a valid range
			while (lpDataPointA < mpBufferStart)
				lpDataPointA += (mpBufferEnd - mpBufferStart);

			while (lpDataPointB < mpBufferStart)
				lpDataPointB += (mpBufferEnd - mpBufferStart);

			// Move along a bit.
			lRingPos += ringbufferSize;
			if (lRingPos > (mpBufferEnd - mpBufferStart))
			{
				lRingPos -= (mpBufferEnd - mpBufferStart);
			}
			
			// Now to return the value				
			return (*lpDataPointB + ((*lpDataPointA - *lpDataPointB) * ringbufferSampLerp));
		}

		T readData(W ringbufferSize)
		{
			W posAVal = readData_(ringbufferSize, mRingPosA);
			W posBVal = readData_(ringbufferSize, mRingPosB);
			
			W posAw = fabs(mRingPosA - (W)(mpBufferPos - mpBufferStart));
			W posBw = fabs(mRingPosB - (W)(mpBufferPos - mpBufferStart));
			
			// Hann Window.
			W posAattn = 0.5 * (1 - cos ( (2 * PI * posAw) / (RINGBUFFER_BUFFER_SIZE - 1)));
			W posBattn = 0.5 * (1 - cos ( (2 * PI * posBw) / (RINGBUFFER_BUFFER_SIZE - 1)));

						
			return (posAVal * posAattn) + (posBVal * posBattn);
		}
	
	public:
		void createBuffer(int size)
		{
			mpBufferStart = new W[size];
			mpBufferPos = mpBufferStart;			
			mpBufferEnd = mpBufferStart + size;
			mRingPosA = 0;
			mRingPosB = size / 2;
			
			for(register int i = 0; i < size; ++i)
				mpBufferStart[i] = 0;		
		}
		
	private:
		T* mpBufferStart;
		T* mpBufferEnd;
		T* mpBufferPos;
		W  mRingPosA;
		W  mRingPosB;
	};
}

#endif
