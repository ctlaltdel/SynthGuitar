#include "daisy_seed.h"
#include "daisysp.h"

int calc_velocity(bool note);

#define FILTER_CUTOFF_MIN       50
#define FILTER_CUTOFF_MAX       8000
#define FILTER_RESONANCE_MAX    0.97
#define ATTACK_MAX              2

#define NUM_OCTAVES_PER_STRING  2

#define RIBBON_SPAN_MIN         0.1
#define RIBBON_SPAN_SCALE       0.1

#define VELOCITY_SCALE          60.0f
#define VELOCITY_INTERVAL       480
#define VELOCITY_MIN            100
#define VELOCITY_MAX            6000

using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;

DaisySeed  hardware;
BlOsc osc_a[3];
Oscillator osc_b[3];
Chorus chorus;
ReverbSc   verb;
static Adsr env[3];
bool gate[3];
static MoogLadder flt;
GPIO string[3];

float ftemp, wet, dry, osc_a_level;
float ribbon_pos, ribbon_span, ribbon_scale, frequency_mult_base, offset_low = 0.1, offset_high = 0.98;

float base_note[3];

bool mute = false;

int sample_count = 0, velocity = 0, velocity_count = 0;
long xminus2h, xminush, x;

enum ADC_Channels
{
    pitch_span = 0,
    pitch,
    osc_a_wfm,
    osc_b_wfm,
    osc_mix,
    filter,
    reverb_mix,
    env_attack_release,
    chorus_rate,
    chorus_depth,
    NUM_ADC_CHANNELS
};

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float sig=0.0, sigwet, env_out[3], freq, new_freq;
    bool vel_flag = false;

    if(!string[0].Read() || !string[1].Read() || !string[2].Read()) 
    {
        ribbon_pos = (hardware.adc.GetFloat(pitch) - offset_low) * ribbon_scale;
        if((ribbon_pos >= 0.0) && (ribbon_pos <= 1.0)) 
        {
//            vel_flag = true;
            ribbon_span = hardware.adc.GetFloat(pitch_span); 
            if(ribbon_span <= RIBBON_SPAN_MIN) ribbon_pos += (1.0f - ribbon_span) * RIBBON_SPAN_SCALE;
            freq = pow(frequency_mult_base, ribbon_pos); 
        }
        else freq = 1.0f;

        for(size_t j = 0; j < 3; j++)
        {
            if(!string[j].Read()) 
            {
                new_freq = freq * base_note[j];
               
                osc_a[j].SetFreq(new_freq);
                osc_b[j].SetFreq(new_freq);
                gate[j] = true; 
            }
            else gate[j] = false;
        }
    }
    else gate[0] = gate[1] = gate[2] = false;

/*    if (velocity_count++ >= VELOCITY_INTERVAL) 
    {
        velocity_count = 0;
        velocity = calc_velocity(vel_flag);
    }
*/
    //Fill the block with samples
    for(size_t i = 0; i < size; i += 2)
    {
        for(size_t k = 0; k < 3; k++)
        {
           env_out[k] = env[k].Process(gate[k]);
           osc_a[k].SetAmp(env_out[k]);
           osc_b[k].SetAmp(env_out[k]);
           sig += (osc_a[k].Process() * osc_a_level) + (osc_b[k].Process() * (1 - osc_a_level));
        }
//        sig = sig * (1.0f + (2 * (float)velocity / VELOCITY_MAX));
        sig = flt.Process(sig);
        sig = chorus.Process(sig);
        verb.Process( sig, 0, &sigwet, 0);
        if(mute == false) out[i] = out[i + 1] = (sig * dry + sigwet * wet);
        else out[i] = out[i + 1] = 0;
    }
}

void UpdateControlPots(void)
{
    ftemp = hardware.adc.GetFloat(filter);
    if(ftemp > 0.02)
    {
        mute = false;
        flt.SetFreq(fmap(ftemp, FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX));
        flt.SetRes(ftemp * FILTER_RESONANCE_MAX);
    }
    else mute = true;

    ftemp = hardware.adc.GetFloat(env_attack_release) * ATTACK_MAX;
    for(size_t i = 0; i < 3; i++)
    {
 //       env[i].SetTime(ADSR_SEG_ATTACK, ftemp);
 //       env[i].SetTime(ADSR_SEG_RELEASE, (1.0f - ftemp));
        env[i].SetTime(ADSR_SEG_ATTACK, 0.1);
        env[i].SetTime(ADSR_SEG_RELEASE, 0.1);
        osc_a[i].SetWaveform(int(fmap(hardware.adc.GetFloat(osc_a_wfm), 0, 2.5)));
        osc_b[i].SetWaveform(int(fmap(hardware.adc.GetFloat(osc_b_wfm), 0, 4.5)));
    }

    wet = hardware.adc.GetFloat(reverb_mix);
    dry = 1.0f - wet;

    osc_a_level = hardware.adc.GetFloat(osc_mix);

    ftemp = hardware.adc.GetFloat(chorus_rate);
    chorus.SetLfoFreq(ftemp, (ftemp * 0.9));
    ftemp = hardware.adc.GetFloat(chorus_depth);
    chorus.SetLfoDepth(ftemp, ftemp); 
}

int main(void)
{
    hardware.Configure();
    hardware.Init();
    hardware.SetAudioBlockSize(4);

//hardware.StartLog();
//System::Delay(500);

    float samplerate = hardware.AudioSampleRate();

    AdcChannelConfig adcConfig[10];
    adcConfig[pitch_span].InitSingle(A0);
    adcConfig[pitch].InitSingle(A1);
    adcConfig[osc_a_wfm].InitSingle(A2);
    adcConfig[osc_b_wfm].InitSingle(A3);
    adcConfig[osc_mix].InitSingle(A4);
    adcConfig[filter].InitSingle(A5);
    adcConfig[reverb_mix].InitSingle(A6);
    adcConfig[env_attack_release].InitSingle(A7);
    adcConfig[chorus_rate].InitSingle(A8);
    adcConfig[chorus_depth].InitSingle(A9);

    hardware.adc.Init(adcConfig, NUM_ADC_CHANNELS);
 
    string[0].Init(D3, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    string[1].Init(D2, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    string[2].Init(D1, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    flt.Init(samplerate);
    flt.SetFreq(880);
    flt.SetRes(0.7);

    chorus.Init(samplerate);
    chorus.SetLfoFreq(.33f, .2f);
    chorus.SetLfoDepth(1.f, 1.f);
    chorus.SetDelay(.75f, .9f);

    verb.Init(samplerate);
    verb.SetFeedback(0.95f);
    verb.SetLpFreq(18000.0f);

    base_note[0] = mtof(36); // C2
    base_note[1] = mtof(43); // G2
    base_note[2] = mtof(50); // D3

    for(size_t i = 0; i < 3; i++)
    {
        osc_a[i].Init(samplerate);
        osc_a[i].SetWaveform(osc_a[i].WAVE_SQUARE);
        osc_a[i].SetFreq(440);

        osc_b[i].Init(samplerate);
        osc_b[i].SetWaveform(osc_b[i].WAVE_SQUARE);
        osc_b[i].SetFreq(440);

        env[i].Init(samplerate);
        env[i].SetTime(ADSR_SEG_DECAY, 0.1);
        env[i].SetSustainLevel(0.25f);
    }
    
    frequency_mult_base = pow(2,NUM_OCTAVES_PER_STRING);
    ribbon_scale = 1.2f / (offset_high - offset_low);
    
    hardware.adc.Start();

    hardware.StartAudio(AudioCallback);

    for(;;) 
    {
        UpdateControlPots();
    }
}

int calc_velocity(bool note)
{
    long velocity;

    if(note)
    {
        switch(sample_count)
        {
            case 0:
                xminus2h = hardware.adc.Get(pitch);
                sample_count++;
                velocity = 0;
                break;

            case 1:
                xminush = hardware.adc.Get(pitch);
                sample_count++;
                velocity = 0;
                break;

            case 2:
                x = hardware.adc.Get(pitch);
                sample_count++;
                velocity = x - xminus2h;
                break;

            case 3:
                xminus2h = xminush;
                xminush = x;
                x = hardware.adc.Get(pitch);
                velocity = x - xminus2h;         
                break;                                     
        };
    }
    else
    {
        sample_count = 0;
        velocity = 0;
    }
//if(note) hardware.PrintLine("%d %u %u %u %d %d", velocity, xminus2h, xminush, x, sample_count);
    velocity = abs(velocity);
    if(velocity < VELOCITY_MIN) velocity = VELOCITY_MIN;
    else if(velocity > VELOCITY_MAX) velocity = VELOCITY_MAX;
    
    return velocity;
}