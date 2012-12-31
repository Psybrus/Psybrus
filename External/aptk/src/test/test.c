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

#include <aptk/aptk.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <portaudio.h>

#define PI						( 3.14159265358979323846f )
#define SQRT_2					( 1.41421356237309504880f )

aptk_biquad_filter_buffer g_buffer;
aptk_biquad_filter_coeff g_coeff;

float g_avgpitch = 0.0f;
float g_cycle = 0.0f;
float g_duty_cycle = ( 2.0 * PI ) / 44100.0;

int stream_callback(
    const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
	float calcfreq = 0.0f;
	int period = 0;
	const float* input_data = (const float*)input;
	float* output_data = (float*)output;

	float* processing_data = malloc(sizeof(float) * frameCount);

	aptk_acf_process(input_data, processing_data, frameCount);
	aptk_acf_largest_peak_period(processing_data, frameCount, &period);

	calcfreq = (float)44100.0 / (float)period;

	if(calcfreq > 0.0f)
	{
		g_avgpitch = g_avgpitch * 0.1f + calcfreq * 0.9f;
		printf("Frequency: %f hz (raw %f hz)\n", g_avgpitch, calcfreq);
	}


	if(output_data)
	{
		while(frameCount--)
		{
			g_cycle += g_duty_cycle * g_avgpitch;
			*output_data++ = sinf(g_cycle * 0.5) * 0.1f;

			if(g_cycle > (2.0*PI))
			{
				g_cycle -= 2.0*PI;
			}
		}
	}

	free(processing_data);

	// Write input to output if it exists.
	if(output_data)
	{
		//aptk_biquad_filter_process(&g_coeff, &g_buffer, input_data, output_data, frameCount);
		//aptk_acf_process(input_data, output_data, frameCount);
	}

	return 0;
}

PaStream* open_stream(PaDeviceIndex input_device, PaDeviceIndex output_device, double* sample_rate, unsigned long* frames_per_buffer)
{
	PaError retval = 0;
	PaStream* stream = 0;
	PaStreamParameters input_stream;
	PaStreamParameters output_stream;
	const PaDeviceInfo* input_device_info;
	const PaDeviceInfo* output_device_info;

	input_device_info = Pa_GetDeviceInfo(input_device);
	output_device_info = Pa_GetDeviceInfo(output_device);

	if(input_device != -1)
	{
		input_stream.device = input_device;
		input_stream.channelCount = 1;
		input_stream.sampleFormat = paFloat32;
		input_stream.suggestedLatency = input_device_info->defaultLowInputLatency;
		input_stream.hostApiSpecificStreamInfo = 0;
	}

	if(output_device != -1)
	{
		output_stream.device = output_device;
		output_stream.channelCount = 1;
		output_stream.sampleFormat = paFloat32;
		output_stream.suggestedLatency = output_device_info->defaultLowOutputLatency;
		output_stream.hostApiSpecificStreamInfo = 0;
	}

	if(input_device_info != 0)
	{
		*sample_rate = input_device_info->defaultSampleRate;
	}
	else if(output_device_info != 0)
	{
		*sample_rate = output_device_info->defaultSampleRate;
	}
	else 
	{
		printf("No devices!\n");
		exit(1);
	}
	
	*frames_per_buffer = 2048;

	retval = Pa_IsFormatSupported(input_device != -1 ? &input_stream : 0, output_device != -1 ? &output_stream : 0, *sample_rate);
	if(retval != paNoError)
	{
		printf("Error with stream format (%i)\n", retval);
		exit(1);
	}

	retval = Pa_OpenStream(&stream, input_device != -1 ? &input_stream : 0, output_device != -1 ? &output_stream : 0, *sample_rate, *frames_per_buffer, 0, stream_callback, 0);

	if(retval != paNoError)
	{
		printf("Error opening stream (%i)\n", retval);
		exit(1);
	}

	return stream;
}

void test_acf()
{
	int i = 0;
	int buffer_size = 128;
	float* input_data = malloc(buffer_size * sizeof(float));
	float* output_data = malloc(buffer_size * sizeof(float));
	float duty_cycle = ( 2.0 * PI ) / buffer_size;
	float freq = 1.0f;
	float calcfreq = 0.0f;
	int period = 0;

	for(freq = 1.0f; freq < 16.0f; freq += 1.0f)
	{
		for(i = 0; i < buffer_size; ++i)
		{
			input_data[i] = sinf((float)i * duty_cycle * freq);
		}

		aptk_acf_process(input_data, output_data, buffer_size);
		aptk_acf_largest_peak_period(output_data, buffer_size, &period);

		calcfreq = (float)buffer_size / (float)period;
		printf("In freq: %f, out freq: %f\n", freq, calcfreq);
	}
}

int main(int argc, char* argv[])
{
	PaError retval = 0;
	PaHostApiIndex host_api_count = 0;
	PaHostApiIndex default_host_api = 0;
	const PaHostApiInfo* host_api_info = 0;
	PaDeviceIndex device_count = 0;
	PaDeviceIndex default_input_device = 0;
	PaDeviceIndex default_output_device = 0;
	PaDeviceIndex selected_input_device = 0;
	PaDeviceIndex selected_output_device = 0;
	const PaDeviceInfo* device_info = 0;
	double sample_rate = 0.0;
	unsigned long frames_per_buffer = 0;
	PaStream* stream = 0;
	int i = 0;

	// Initialize
	retval = Pa_Initialize();
	if(retval != paNoError)
	{
		exit(1);
	}

	// Enumerate host apis.
	host_api_count = Pa_GetHostApiCount();
	default_host_api = Pa_GetDefaultHostApi();

	printf("Host API count: %i\n", host_api_count);
	printf("Default Host API: %i\n", default_host_api);

	for(i = 0; i < host_api_count; ++i)
	{
		host_api_info = Pa_GetHostApiInfo(i);

		printf("Host API %i:\n", i);
		printf(" - Name: %s\n", host_api_info->name);
		printf(" - Device Count: %i\n", host_api_info->deviceCount);
		printf(" - Default Input Device Index: %i\n", host_api_info->defaultInputDevice);
		printf(" - Default Output Device Index: %i\n", host_api_info->defaultOutputDevice);
	}

	// Enumerate devices.
	device_count = Pa_GetDeviceCount();
	default_input_device = Pa_GetDefaultInputDevice();
	default_output_device = Pa_GetDefaultOutputDevice();

	printf("Device count: %i\n", device_count);
	printf("Default Input Device: %i\n", default_input_device);
	printf("Default Output Device: %i\n", default_output_device);

	for(i = 0; i < device_count; ++i)
	{
		device_info = Pa_GetDeviceInfo(i);
		host_api_info = Pa_GetHostApiInfo(device_info->hostApi);
	
		printf("Device %i:\n", i);
		printf(" - Name: %s\n", device_info->name);
		printf(" - Host API Name: %s\n", host_api_info->name );
		printf(" - Input Channels: %i\n", device_info->maxInputChannels );
		printf(" - Output Channels: %i\n", device_info->maxOutputChannels );
		printf(" - Default low input latency: %f ms\n", device_info->defaultLowInputLatency * 1000.0 );
		printf(" - Default low output latency: %f ms\n", device_info->defaultLowOutputLatency * 1000.0 );
		printf(" - Default high input latency: %f ms\n", device_info->defaultHighInputLatency * 1000.0 );
		printf(" - Default high output latency: %f ms\n", device_info->defaultHighOutputLatency * 1000.0 );
		printf(" - Default sample rate: %f\n", device_info->defaultSampleRate );
	}

	printf("\n\n");

	// Device selection:
	printf("Input Device:\n");
	for(i = 0; i < device_count; ++i)
	{
		device_info = Pa_GetDeviceInfo(i);
		host_api_info = Pa_GetHostApiInfo(device_info->hostApi);
	
		if(device_info->maxInputChannels > 0)
		{
			printf("%i: %s (%s, SR: %f)\n", i, device_info->name, host_api_info->name, device_info->defaultSampleRate);
		}
	}

	printf("Choice? ");
	scanf("%i", &selected_input_device);

	printf("Output Device:\n");
	for(i = 0; i < device_count; ++i)
	{
		device_info = Pa_GetDeviceInfo(i);
		host_api_info = Pa_GetHostApiInfo(device_info->hostApi);
	
		if(device_info->maxOutputChannels > 0)
		{
			printf("%i: %s (%s, SR: %f)\n", i, device_info->name, host_api_info->name, device_info->defaultSampleRate);
		}
	}

	printf("Choice? ");
	scanf("%i", &selected_output_device);

	stream = open_stream(selected_input_device, selected_output_device, &sample_rate, &frames_per_buffer);

	aptk_biquad_filter_buffer_init(&g_buffer);
	aptk_biquad_filter_coeff_passthrough(&g_coeff);
	//aptk_biquad_filter_coeff_highpass(&g_coeff, sample_rate, 10.0f, 0.0f);


	test_acf();

	
	Pa_StartStream(stream);
	getch();
	Pa_StopStream(stream);
	
	/*
	FILE* file = NULL;
	int filesize = 0;
	short* rawdata = NULL;
	float* indata = NULL;
	float* outdata = NULL;
	int samples = 0;
	int i;
	aptk_biquad_filter_buffer buffer;
	aptk_biquad_filter_coeff coeff;

	file = fopen(argv[1], "rb");
	if(file)
	{
		fseek(file, 0, SEEK_END);
		filesize = ftell(file);
		fseek(file, 0, SEEK_SET);
		rawdata = (short*)malloc(filesize);
		fread(rawdata, filesize, 1, file);
		fclose(file);

		samples = filesize / sizeof(short);
		indata = (float*)malloc(samples * sizeof(float));
		outdata = (float*)malloc(samples * sizeof(float));

		for(i = 0; i < samples; ++i)
		{
			indata[i] = (float)rawdata[i] / (float)SHRT_MAX;
		}

		aptk_biquad_filter_buffer_init(&buffer);
		//aptk_biquad_filter_coeff_passthrough(&coeff);
		aptk_biquad_filter_coeff_lowpass(&coeff, 44100.0f, 1000.0f, 0.0f);
		aptk_biquad_filter_process(&coeff, &buffer, indata, outdata, samples);

		for(i = 0; i < samples; ++i)
		{
			rawdata[i] = (short)(outdata[i] * (float)SHRT_MAX);
		}

		file = fopen(argv[2], "wb+");
		if(file)
		{
			fwrite(rawdata, samples * sizeof(short), 1, file);
			fclose(file);
		}
	}
	*/

	return 0;
}
