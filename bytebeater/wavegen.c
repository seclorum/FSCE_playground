#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

// Constants
#define SAMPLE_RATE 44100  // 44.1 kHz sample rate
#define DURATION 5         // Duration in seconds
#define AMPLITUDE 32767    // Maximum amplitude for 16-bit audio
#define FREQUENCY 440      // Frequency in Hz (for all waveforms)
#define WAVETABLE_SIZE 512 // Length of the wavetable


// FM parameters (carrier and modulator frequencies and modulation index)
double carrier_freq = 440.0;    // Carrier frequency (A4)
double mod_freq = 220.0;        // Modulator frequency (sub-audible)
double modulation_index = 5.0;  // Modulation index (controls the amount of FM)

uint8_t wavetable[WAVETABLE_SIZE];


// Enum to represent different wave types
typedef enum {
    SINE = 0,
    SQUARE = 1,
    FM = 2,
    SAMPLE = 3,
    TRIANGLE = 4
} WaveType;


// Function to generate sine wave sample
int16_t wavegen_sine(double t) {
    return (int16_t)(AMPLITUDE * sin(2 * M_PI * FREQUENCY * t));
}

// Function to generate square wave sample
int16_t wavegen_square(double t) {
    return (int16_t)(AMPLITUDE * (sin(2 * M_PI * FREQUENCY * t) >= 0 ? 1 : -1));
}

// Function to generate triangle wave sample
int16_t triangle_wave(double t) {
    return (int16_t)(AMPLITUDE * (2 * fabs(2 * (t * FREQUENCY - floor(t * FREQUENCY + 0.5))) - 1));
}

// Function to initialize the wavetable with 'interesting' data
void initialize_wavetable(uint8_t wavetable[WAVETABLE_SIZE]) {
    srand(time(NULL));  // Seed the random number generator

    // Fill the wavetable with interesting data: mix sine wave with random noise
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
        double t = (double)i / WAVETABLE_SIZE;  // Normalize index to [0, 1]
        
        // Generate a sine wave component
        double wavegen_sine = 0.5 * sin(2 * M_PI * t * 10); // 10 Hz sine wave component
        
        // Generate some random noise
        double noise = ((rand() % 256) - 128) / 128.0; // Random noise in range [-1, 1]
        
        // Combine sine wave and noise to form interesting data
        double combined_signal = wavegen_sine + noise;
        
        // Map the combined signal to a byte in the range [0, 255]
        wavetable[i] = (uint8_t)((combined_signal + 1) * 127.5); // Normalize to [0, 255]
    }
}


// Function to generate wavetable-based sample
int16_t wavegen_sample(uint8_t wavetable[WAVETABLE_SIZE], double t) {
    // Determine the index in the wavetable based on the time
    double index = fmod(t * SAMPLE_RATE, WAVETABLE_SIZE); // Wrap around the wavetable length
    int table_index = (int)index;
    
    // Map the wavetable byte to a 16-bit signed value
    return (int16_t)(AMPLITUDE * ((double)wavetable[table_index] / 255.0 * 2.0 - 1.0)); // Normalize to [-1, 1]
}

// FM synthesis function
int16_t wavegen_fm(double t, double carrier_freq, double mod_freq, double modulation_index) {
    // Generate the modulator wave (low-frequency oscillation)
    double mod_signal = modulation_index * sin(2 * M_PI * mod_freq * t);
    
    // Generate the carrier wave with frequency modulation
    double carrier_signal = sin(2 * M_PI * carrier_freq * t + mod_signal);
    
    // Scale the output to the 16-bit signed integer range
    return (int16_t)(AMPLITUDE * carrier_signal);
}


// Bytebeat function to choose waveform based on user
int16_t wavegen_select(double t, int wave_select) {
    switch (wave_select) {
        case SINE:
            return wavegen_sine(t);
        case SQUARE:
            return wavegen_square(t);
        case FM:
            return wavegen_fm(t, carrier_freq, mod_freq, modulation_index);
        case SAMPLE:
            return wavegen_sample(wavetable, t);
        case TRIANGLE:
        default:
            return triangle_wave(t);
    }
}


// Function to parse command-line argument and set wavegen_select
WaveType get_opts(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <wave_type>\n", argv[0]);
        fprintf(stderr, "Wave types: sine, square, fm, sample, triangle\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "sine") == 0) {
        return SINE;
    } else if (strcmp(argv[1], "square") == 0) {
        return SQUARE;
    } else if (strcmp(argv[1], "fm") == 0) {
        return FM;
    } else if (strcmp(argv[1], "sample") == 0) {
        return SAMPLE;
    } else if (strcmp(argv[1], "triangle") == 0) {
        return TRIANGLE;
    } else {
        fprintf(stderr, "Invalid wave type: %s\n", argv[1]);
        fprintf(stderr, "Valid wave types: sine, square, fm, sample, triangle\n");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[]) {
    // Pipe audio data to stdout (sox will read this)
    WaveType gen_select = get_opts(argc, argv);

	initialize_wavetable(wavetable);

    for (int i = 0; i < SAMPLE_RATE * DURATION; i++) {
        double t = (double)i / SAMPLE_RATE;  // Time in seconds

        int16_t sample = wavegen_select(t, gen_select);

        // Write the sample as little-endian 16-bit PCM to stdout
        fwrite(&sample, sizeof(int16_t), 1, stdout);
	
    }

    return 0;
}

