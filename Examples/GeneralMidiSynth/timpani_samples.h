#pragma once
#include <AudioStream.h>
#include <AudioSynthWavetable.h>

extern const sample_data timpani_samples[3];
const uint8_t timpani_ranges[] = {49, 63, 127, };

const instrument_data timpani = {3, timpani_ranges, timpani_samples };


extern const uint32_t sample_0_timpani_timpani[3968];

extern const uint32_t sample_1_timpani_timpani[3968];

extern const uint32_t sample_2_timpani_timpani[3968];