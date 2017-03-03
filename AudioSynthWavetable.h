#pragma once

#include "Arduino.h"
#include "AudioStream.h"
#include <math.h>
#include <sample_data.h>
#include <stdint.h>

#define UNITY_GAIN INT32_MAX // Max amplitude
#define SAMPLES_PER_MSEC (AUDIO_SAMPLE_RATE_EXACT/1000.0)
#define AMP_DEF 69

// int n in range 1..log2(AUDIO_BLOCK_SAMPLES/2)-1 (1..7 for AUDIO_BLOCK_SAMPLES == 128)
// where AUDIO_BLOCK_SAMPLES%n == 0, higher == more smooth and more CPU usage
#define VIBRATO_SMOOTHNESS 7
#define VIBRATO_PERIOD AUDIO_BLOCK_SAMPLES/(1 << (VIBRATO_SMOOTHNESS-1))

class AudioSynthWavetable : public AudioStream
{
public:
	AudioSynthWavetable(void) : AudioStream(0, NULL) {}

	void setInstrument(const instrument_data& instrument) {
		cli();
		this->instrument = &instrument;
		current_sample = NULL;
		envelopeState = STATE_IDLE;
		state_change = true;
		sei();
	}

	void amplitude(float v) {
		v = (v < 0.0) ? 0.0 : (v > 1.0) ? 1.0 : v;
		tone_amp = (uint16_t)(32767.0*v);
	}

	static float midi_volume_transform(int midi_amp) {
		// scale midi_amp which is 0 t0 127 to be between
		// 0 and 1 using a logarithmic transformation
		return powf(midi_amp / 127.0, 4);
	}

	static float noteToFreq(int note) {
		//440.0 * pow(2.0, (note - 69) / 12.0);
		float exp = note * (1.0 / 12.0) + 3.0313597;
		return powf(2.0, exp);
	}

	static int freqToNote(float freq) {
		return (12.0 / 440.0) * log2f(freq) + 69.5;
	}

	// Defined in AudioSynthWavetable.cpp
	void stop(void);
	void playFrequency(float freq, int amp = AMP_DEF);
	void playNote(int note, int amp = AMP_DEF);
	bool isPlaying(void) { return envelopeState != STATE_IDLE; }
	virtual void update(void);

private:
	void setState(int note, int amp, float freq);
	void setFrequency(float freq);

	enum envelopeStateEnum { STATE_IDLE, STATE_DELAY, STATE_ATTACK, STATE_HOLD, STATE_DECAY, STATE_SUSTAIN, STATE_RELEASE };

	uint32_t milliseconds2count(float milliseconds) {
		if (milliseconds < 0.0) milliseconds = 0.0;
		// # of 8-sample units to process
		// Add 7 to round up
		return ((uint32_t)(milliseconds*SAMPLES_PER_MSEC) + 7) >> 3;
	}

	volatile const instrument_data* instrument = NULL;
	volatile const sample_data* current_sample = NULL;
	volatile uint32_t tone_phase = 0;
	volatile uint32_t tone_incr = 0;
	volatile uint16_t tone_amp = 0;
	volatile envelopeStateEnum  envelopeState = STATE_IDLE; // idle, delay, attack, hold, decay, sustain, release
	volatile int32_t count = 0; // how much time remains in this state, in 8 sample units
	volatile int32_t mult = 0; // attenuation, 0=off, 0x10000=unity gain
	volatile int32_t inc = 0; // amount to change mult on each sample
	volatile bool state_change = false;

	//vibrato members
	uint32_t vcount = 0;
	uint32_t vphase = 0x80000000; //vibrato starts on downward slope

	const uint32_t vdelay = 0.0 * SAMPLES_PER_MSEC/(2*VIBRATO_PERIOD);  
	const uint32_t vincr = 21.4 * VIBRATO_PERIOD * (UINT32_MAX / AUDIO_SAMPLE_RATE_EXACT);  //21.4 hz
	const float voffset_high_coef = 0.00463167440; // +8 cents coef
	const float voffset_low_coef = 0.00461032089; // -8 cents coef
};

