/*
 * $Id: patest_sine_formats.c,v 1.2.4.2 2003/02/12 01:39:29 philburk Exp $
 * patest_sine_formats.c
 * Play a sine wave using the Portable Audio api for several seconds.
 * Test various data formats.
 *
 * Author: Phil Burk
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.audiomulch.com/portaudio/
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdio.h>
#include <math.h>
#include "portaudio.h"

#define NUM_SECONDS        (5)
#define SAMPLE_RATE        (44100)
#define FRAMES_PER_BUFFER  (512)
#define LEFT_FREQ          ((2 * SAMPLE_RATE)/FRAMES_PER_BUFFER)  /* So we hit 1.0 */
#define RIGHT_FREQ         (500.0)
#define AMPLITUDE          (0.9)

/* Select ONE format for testing. */
#define TEST_UINT8    (0)
#define TEST_INT8     (0)
#define TEST_INT16    (0)
#define TEST_INT32    (1)
#define TEST_FLOAT32  (0)

#if TEST_UINT8
#define TEST_FORMAT         paUInt8
typedef unsigned char       SAMPLE_t;
#define SAMPLE_ZERO         (0x80)
#define DOUBLE_TO_SAMPLE(x) (SAMPLE_ZERO + (SAMPLE_t)(127.0 * (x)))
#define FORMAT_NAME         "Unsigned 8 Bit"

#elif TEST_INT8
#define TEST_FORMAT         paInt8
typedef char                SAMPLE_t;
#define SAMPLE_ZERO         (0)
#define DOUBLE_TO_SAMPLE(x) (SAMPLE_ZERO + (SAMPLE_t)(127.0 * (x)))
#define FORMAT_NAME         "Signed 8 Bit"

#elif TEST_INT16
#define TEST_FORMAT         paInt16
typedef short               SAMPLE_t;
#define SAMPLE_ZERO         (0)
#define DOUBLE_TO_SAMPLE(x) (SAMPLE_ZERO + (SAMPLE_t)(32767 * (x)))
#define FORMAT_NAME         "Signed 16 Bit"

#elif TEST_INT32
#define TEST_FORMAT         paInt32
typedef long               SAMPLE_t;
#define SAMPLE_ZERO         (0)
#define DOUBLE_TO_SAMPLE(x) (SAMPLE_ZERO + (SAMPLE_t)(0x7FFFFFFF * (x)))
#define FORMAT_NAME         "Signed 32 Bit"

#elif TEST_FLOAT32
#define TEST_FORMAT         paFloat32
typedef float               SAMPLE_t;
#define SAMPLE_ZERO         (0.0)
#define DOUBLE_TO_SAMPLE(x) ((SAMPLE_t)(x))
#define FORMAT_NAME         "Float 32 Bit"
#endif

#ifndef M_PI
#define M_PI  (3.14159265)
#endif


typedef struct
{
    double left_phase;
    double right_phase;
    unsigned int framesToGo;
}
paTestData;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback( void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           PaTimestamp outTime, void *userData )
{
    paTestData *data = (paTestData*)userData;
    SAMPLE_t *out = (SAMPLE_t *)outputBuffer;
    int i;
    int framesToCalc;
    int finished = 0;
    (void) outTime; /* Prevent unused variable warnings. */
    (void) inputBuffer;

    if( data->framesToGo < framesPerBuffer )
    {
        framesToCalc = data->framesToGo;
        data->framesToGo = 0;
        finished = 1;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        data->framesToGo -= framesPerBuffer;
    }

    for( i=0; i<framesToCalc; i++ )
    {
        data->left_phase += (LEFT_FREQ / SAMPLE_RATE);
        if( data->left_phase > 1.0) data->left_phase -= 1.0;
        *out++ = DOUBLE_TO_SAMPLE( AMPLITUDE * sin( (data->left_phase * M_PI * 2. )));

        data->right_phase += (RIGHT_FREQ / SAMPLE_RATE);
        if( data->right_phase > 1.0) data->right_phase -= 1.0;
        *out++ = DOUBLE_TO_SAMPLE( AMPLITUDE * sin( (data->right_phase * M_PI * 2. )));
    }
    /* zero remainder of final buffer */
    for( ; i<(int)framesPerBuffer; i++ )
    {
        *out++ = SAMPLE_ZERO; /* left */
        *out++ = SAMPLE_ZERO; /* right */
    }
    return finished;
}
/*******************************************************************/
int main(void);
int main(void)
{
    PortAudioStream *stream;
    PaError err;
    paTestData data;
    int totalSamps;

    printf("PortAudio Test: output " FORMAT_NAME "\n");


    data.left_phase = data.right_phase = 0.0;
    data.framesToGo = totalSamps =  NUM_SECONDS * SAMPLE_RATE; /* Play for a few seconds. */
    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    err = Pa_OpenStream(
              &stream,
              paNoDevice,/* default input device */
              0,              /* no input */
              TEST_FORMAT,
              NULL,
              Pa_GetDefaultOutputDeviceID(), /* default output device */
              2,          /* stereo output */
              TEST_FORMAT,
              NULL,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              0,              /* number of buffers, if zero then use default minimum */
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              patestCallback,
              &data );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;

    printf("Waiting %d seconds for sound to finish.\n", NUM_SECONDS ); fflush(stdout);
    while( Pa_StreamActive( stream ) ) Pa_Sleep(10);

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
    Pa_Terminate();

    printf("PortAudio Test Finished: " FORMAT_NAME "\n");

    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
