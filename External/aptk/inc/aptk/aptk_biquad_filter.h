/*
 * Copyright (c) 2012 Neil Richardson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __APTK_BIQUAD_FILTER_H__
#define __APTK_BIQUAD_FILTER_H__

/* 
 * aptk_biquad_filter_coeff
 */
typedef struct
{
	float in[3];
	float out[2];
} aptk_biquad_filter_coeff;

/* 
 * aptk_biquad_filter_buffer
 */
typedef struct
{
	float in[2];
	float out[2];
} aptk_biquad_filter_buffer;

/*
 * aptk_biquad_filter_buffer_init
 */
extern int aptk_biquad_filter_buffer_init(aptk_biquad_filter_buffer* buffer);

/*
 * aptk_biquad_filter_buffer_destroy
 */
extern int aptk_biquad_filter_buffer_destroy(aptk_biquad_filter_buffer* buffer);

/*
 * aptk_biquad_process
 * @param coeff Coefficients.
 * @param buffer Buffer data for internal processing.
 * @param invalues Input values to process.
 * @param outvalues Output values.
 * @param numvalues Number of values to process.
 */
extern int aptk_biquad_filter_process(const aptk_biquad_filter_coeff* coeff, aptk_biquad_filter_buffer* buffer, const float* invalues, float* outvalues, int numvalues);

/*
 * aptk_biquad_filter_coeff_passthrough
 */
extern int aptk_biquad_filter_coeff_passthrough(aptk_biquad_filter_coeff* coeff);

/*
 * aptk_biquad_filter_coeff_lowpass
 */
extern int aptk_biquad_filter_coeff_lowpass(aptk_biquad_filter_coeff* coeff, float rate, float cutoff, float resonance);

/*
 * aptk_biquad_filter_coeff_highpass
 */
extern int aptk_biquad_filter_coeff_highpass(aptk_biquad_filter_coeff* coeff, float rate, float cutoff, float resonance);



#endif
