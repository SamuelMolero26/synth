#include <stdio.h>
#include "raylib.h"
#include <math.h>


#define SAMPLE_RATE 44100
#define STREAM_BUFFER_SIZE 1024
#define NUM_OSCILLATORS 16
#define SAMPLE_DURATION (1.0f / 44100)

typedef struct{
    float phase;
    float phase_stride;
 }Oscillator;

 void setOscFrequency(Oscillator* osc, float frequency)
 {
    osc-> phase_stride = frequency * SAMPLE_DURATION;
 }

 void zeroSignal(float* signal)
 {
    for(size_t t = 0; t < STREAM_BUFFER_SIZE; t++)
    {
        signal[t] = 0.0f;
    }
 }

 void UpdateOsc(Oscillator* osc){
    osc->phase += osc->phase_stride;
    if(osc->phase >= 1.0f)
    {
        osc->phase -= 1.0f;
    }
 }
 void sineWaveOsc(Oscillator* osc){

    return sinf(2.0f * PI * osc->phase);

 }


 void accumulateSignal(float* signal, Oscillator* osc, float amplitude)
 {
    //osc -> phase += osc -> phase_stride;
    for(size_t j  = 0; j < STREAM_BUFFER_SIZE; j++)
    {
        UpdateOsc(osc);
        signal[j] += sinf(2.0f * PI * osc->phase) * amplitude;
    }
 }

void main(){

    const int screen_width = 1024;
    const int screen_height = 768;
    InitWindow(screen_width, screen_height, "Synth");
    SetTargetFPS(60);
    InitAudioDevice();


    unsigned int sample_rate = SAMPLE_RATE; //human rate
    SetAudioStreamBufferSizeDefault(STREAM_BUFFER_SIZE);
    AudioStream synthStream = LoadAudioStream(sample_rate,sizeof(float) * 8, 1);

    SetAudioStreamVolume(synthStream, 0.5f);
    PlayAudioStream(synthStream);
    

    float frequency = 5.0f;
    float sample_duration = (1.0f / sample_rate);
    
    Oscillator osc[NUM_OSCILLATORS] = {0};

    //low frequency
    Oscillator lfo = { .phase = 0.0f, .phase_stride =  1000.0f  * sample_duration}; 
    setOscFrequency(&lfo, 1.0f * 1024);
    float signal[STREAM_BUFFER_SIZE];

    float detune = 0.1f;

    while(WindowShouldClose() == false)
    {

        // to check make sure eveythong works fine
        Vector2 mouse_pos = GetMousePosition();
        float normalize_mouse_x = (mouse_pos.x /(float)screen_width);

        detune = 1.0f + normalize_mouse_x * 10.0f;
        if(IsAudioStreamProcessed(synthStream) == true)
        {
            //(220 - 25 hz) -> faster
            

            zeroSignal(signal);
            UpdateOsc(&lfo);
            float base_freq = 25.0f + (normalize_mouse_x * 400.0f);
            for(size_t i = 0; i < NUM_OSCILLATORS; i++)
            {
                // osc.phase_stride = frequency * sample_duration;
                if(i % 2 == 0)
                {
                    float normalized_index = (float)i / NUM_OSCILLATORS;
                    frequency = base_freq * i;
                    float phase_stride = frequency * sample_duration;
                    osc[i].phase_stride = phase_stride;
                    accumulateSignal(signal,&osc[i], 1.0f  / NUM_OSCILLATORS); 
                }
                

            }
        
            UpdateAudioStream(synthStream, signal, STREAM_BUFFER_SIZE);

        }

        BeginDrawing();
        ClearBackground(BLACK);
        if(IsAudioStreamPlaying(synthStream))
        {
            DrawText(TextFormat("Frequency: %f", frequency), 100, 125, 20, GREEN);
        }
        DrawText("Synth", 100, 100, 20,GREEN);
        
        
        for(size_t i  = 0; i < 1024; i++)
        {
            DrawPixel(i, (728/2) + (int)(signal[i] * 100), GREEN);
            // signal[i] = sinf((float)i * 0.05f); 
        }
        DrawFPS(10,10);
        EndDrawing();

    }
    CloseAudioDevice();
    CloseWindow(); 
    
};