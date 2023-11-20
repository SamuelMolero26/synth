#include <stdio.h>
#include "raylib.h"
#include <math.h>

#define SAMPLE_RATE 48000
#define STREAM_BUFFER_SIZE 1024
#define NUM_OSCILLATORS 128
#define SAMPLE_DURATION (1.0f / 44100)

//different wave forms
 typedef enum {
    SINE,
    SQUARE,
    TRIANGLE,
    SAWTOOTH
 }Waveform;

typedef struct{
    float phase;
    float phase_stride;
    float freq; 
    float phase_modulation; 
    float phase_amplitude; 
    Waveform wave_form;
    float transition_factor; 
 }Oscillator;

 

//  void setOscFrequency(Oscillator* osc, float frequency)
//  {
//     osc-> phase_stride = frequency * SAMPLE_DURATION;
//  }

 void zeroSignal(float* signal)
 {
    for(size_t t = 0; t < STREAM_BUFFER_SIZE; t++)
    {
        signal[t] = 0.0f;
    }
 }

 void UpdateOsc(Oscillator* osc, float phase_modulation){
    osc->phase += ((osc->freq + phase_modulation) * SAMPLE_DURATION);
    if(osc->phase >= 1.0f)
    {
        osc->phase -= 1.0f;
    }

    if(osc->phase < 0.0f)
    {
        osc->phase += 1.0f;
    }
 }

// float sineWaveOsc(Oscillator* osc){
//     float sample1 = sinf(2.0f * PI * osc->phase) * osc->phase_amplitude;
//     float sample2 = sinf(2.0f * PI * ((osc->phase + osc->phase_stride) >= 1.0f ? 0.0f : (osc->phase + osc->phase_stride))) * osc->phase_amplitude;
    
//     float calculations =  sample1 + osc->phase * (sample2 - sample1); //Linear interpolation - 102 assigment came in handy
//     return calculations; 
//     // return sinf(2.0f * PI * osc->phase) * osc->phase_amplitude;

//  }

 //support different waveforms

float waveformOsc(Oscillator* osc)
{
    switch (osc->wave_form){
        case SINE:
            return sinf(2.0f * PI * osc->phase) * osc->phase_amplitude;
        case SQUARE:
            return (osc->phase < 0.5f) ? osc->phase_amplitude : -osc->phase_amplitude;
        case TRIANGLE:
            return 1.0f - 4.0f * fabsf(osc->phase - 0.5f);
        case SAWTOOTH:
            return 2.0f * (osc->phase - floorf(osc->phase + 0.5f)) * osc->phase_amplitude;
        default:
            return 0.0f;
    }

}

 void accumulateSignal(float* signal, Oscillator* osc, Oscillator* lfo)
 {
    
    //osc -> phase += osc -> phase_stride;
    const float alpha = 0.1f; // Adjust the filter coefficient
    for(size_t j  = 0; j < STREAM_BUFFER_SIZE; j++)
    {
        
        UpdateOsc(lfo,0.0f);
        UpdateOsc(osc,waveformOsc(lfo));

        //smooth transition
        float current_sample = waveformOsc(osc);
        osc->phase_amplitude *= 1.0f - osc->transition_factor;
        osc->phase_amplitude += osc->transition_factor * osc->phase_amplitude;
        signal[j] += current_sample;

        if(signal[j] > 1.0f){ signal[j] = 1.0f;}
        else if (signal[j] < -1.0f){signal[j] = -1.0f; }




        //updating transition factor
        osc->transition_factor += 0.001f;
        if (osc->transition_factor > 1.0f){osc->transition_factor = 0.0f;}

        //low pass filtering for the signal
        if(j > 0){
            signal[j] = alpha * signal[j] + (1.0f - alpha) * signal[j - 1];
        }
    }
 }



void main(){

    const int screen_width = 1024;
    const int screen_height = 768;
    InitWindow(screen_width, screen_height, "Synth");
    SetTargetFPS(60);
    InitAudioDevice();


    unsigned int sample_rate = SAMPLE_RATE;
    SetAudioStreamBufferSizeDefault(STREAM_BUFFER_SIZE);
    AudioStream synthStream = LoadAudioStream(SAMPLE_RATE,sizeof(float) * 8, 1);

    SetAudioStreamVolume(synthStream, 0.008f);
    PlayAudioStream(synthStream);
    

    // float frequency = 5.0f;
    Oscillator osc[NUM_OSCILLATORS] = {0};
    //low frequency
    Oscillator lfo = { .phase = 0.0f, .freq = 2.0f, .phase_amplitude = 20.0f };
    
    float signal[STREAM_BUFFER_SIZE];

    while(WindowShouldClose() == false)
    {

        // to check make sure eveythong works fine
        Vector2 mouse_pos = GetMousePosition();
        float normalize_mouse_x = (mouse_pos.x /(float)screen_width);
        float normalized_mouse_y = (mouse_pos.y / (float)screen_height);
        float base_freq = 25.0f + (normalize_mouse_x * 200.0f);
        lfo.freq = 1.0f + (normalized_mouse_y * 1.0f);
        lfo.phase_amplitude = 50.0f + (normalize_mouse_x * 50.0f);

        float audio_duration = 0.0f;
        if(IsAudioStreamProcessed(synthStream) == true)
        {
            
            const double audio_start_time = GetTime(); 
            zeroSignal(signal);
    
            for(size_t i = 0; i < NUM_OSCILLATORS; i++)
            {
                // osc.phase_stride = frequency * sample_duration;
                if(i % 2 == 0)
                {
                    //float normalized_index = (float)i / NUM_OSCILLATORS;
                    osc[i].freq = base_freq * (i+1);
                    osc[i].phase_amplitude = 1.0f / (NUM_OSCILLATORS * 4.0f); 
                    // setOscFrequency(&osc[i], frequency);

                    osc[i].wave_form = (Waveform)(i % 4); // Select waveform based on oscillator index
                    accumulateSignal(signal, &osc[i], &lfo);
                }
                

            }
        
            UpdateAudioStream(synthStream, signal, STREAM_BUFFER_SIZE);
            audio_duration = GetTime()  - audio_start_time; 

        }
        // printf("Frequency: %f\n", osc[0].freq);

        BeginDrawing();
        ClearBackground(BLACK);

        // if(IsAudioStreamPlaying(synthStream))
        // {
        //     DrawText(TextFormat("Frequency: %f",osc[0].freq), 100, 125, 20, GREEN);
        // }
        DrawText("Synth", 100, 100, 20,GREEN);
        
        
        // for(size_t i  = 0; i < 1024; i++)
        // {
        //     DrawPixel(i, (728/2) + (int)(signal[i] * 100), GREEN);
        //     // signal[i] = sinf((float)i * 0.05f); 
        // }
        for (size_t i = 1; i < STREAM_BUFFER_SIZE; i++) {
            DrawLineEx((Vector2){(float)(i - 1), (728 / 2) + (int)(signal[i - 1] * 80)},
               (Vector2){(float)i, (728 / 2) + (int)(signal[i] * 80)},
               2.0f, GREEN);
        }
        const float total_frame_time = GetFrameTime();
        DrawText(TextFormat("Frame time: %f, Audio frame time: %f", total_frame_time, audio_duration), 10, 10 , 20, GREEN); 
        EndDrawing();

    }
    CloseAudioDevice();
    CloseWindow(); 


    
}