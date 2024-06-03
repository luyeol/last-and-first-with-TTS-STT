// #pragma once
/*
g++ -o record record.cpp -lportaudio -lsndfile
./record
*/
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>

using namespace std;

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2
#define FRAMES_PER_BUFFER 512
#define DURATION 5

typedef struct {
    float *recordedSamples;
    int frameIndex;
    int maxFrameIndex;
} paTestData;

static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    paTestData *data = (paTestData *)userData;
    const float *rptr = (const float *)inputBuffer;
    float *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc = data->maxFrameIndex - data->frameIndex;
    long i;

    (void)outputBuffer; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;

    if (framesToCalc > (long)framesPerBuffer) {
        framesToCalc = (long)framesPerBuffer;
    }

    if (inputBuffer == NULL) {
        for (i = 0; i < framesToCalc; i++) {
            *wptr++ = 0.0f;  /* left */
            if (NUM_CHANNELS == 2) *wptr++ = 0.0f;  /* right */
        }
    } else {
        for (i = 0; i < framesToCalc; i++) {
            *wptr++ = *rptr++;  /* left */
            if (NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return paContinue;
}

int main() {

    PaStreamParameters inputParameters;
    PaStream *stream;
    PaError err = paNoError;
    paTestData data;
    int totalFrames = DURATION * SAMPLE_RATE;
    int numSamples = totalFrames * NUM_CHANNELS;
    int numBytes = numSamples * sizeof(float);
    SF_INFO sfinfo;
    SNDFILE *outfile;

    data.maxFrameIndex = totalFrames;
    data.frameIndex = 0;
    data.recordedSamples = (float *)malloc(numBytes);

    if (data.recordedSamples == NULL) {
        cerr << "Could not allocate record array." << endl;
    }
    for (int i = 0; i < numSamples; i++) data.recordedSamples[i] = 0;

    err = Pa_Initialize();
    if (err != paNoError) {
        cerr << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    }

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        cerr << "Error: No default input device." << endl;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&stream, &inputParameters, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, recordCallback, &data);
    if (err != paNoError) {
        cerr << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        cerr << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    }

    // cout << "Recording for " << DURATION << " seconds." << endl;
    Pa_Sleep(DURATION * 1000);

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        cerr << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        cerr << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    }

    Pa_Terminate();

    sfinfo.frames = totalFrames;
    sfinfo.samplerate = SAMPLE_RATE;
    sfinfo.channels = NUM_CHANNELS;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    outfile = sf_open("recorded.wav", SFM_WRITE, &sfinfo);
    if (!outfile) {
        cerr << "Error opening output file!" << endl;
    }

    sf_write_float(outfile, data.recordedSamples, numSamples);
    sf_close(outfile);

    free(data.recordedSamples);
    // cout << "Recording saved as recorded.wav" << endl;

    return 0;
}