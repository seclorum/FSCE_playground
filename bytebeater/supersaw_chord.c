#include <stdio.h>
#include <math.h>

// Define frequencies for a chord sequence (Debussy-inspired)
float chord_sequence[][4] = {
    {1.0, 1.25, 1.5, 1.75}, // Root, major third, perfect fifth, major seventh
    {1.0, 1.259921, 1.498307, 1.887749}, // Next chord with slightly altered intervals
    {1.0, 1.189207, 1.414214, 1.781797}, // Add more chords as needed
};
int num_chords = sizeof(chord_sequence) / sizeof(chord_sequence[0]);
int chord_duration = 8000 * 2; // Duration of each chord in samples (2 seconds)

// Function to generate a detuned supersaw
unsigned char supersaw(int t, float base_freq) {
    float detune[] = {1.0, 1.03, 0.97}; // Detune ratios
    int num_detune = sizeof(detune) / sizeof(detune[0]);

    int value = 0;
    for (int i = 0; i < num_detune; i++) {
        value += ((int)(t * base_freq * detune[i]) & 255);
    }
    return value / num_detune;
}

int main() {
    unsigned int t = 0;

    // Play the chord sequence
    while (1) {
        // Determine the current chord
        int chord_index = (t / chord_duration) % num_chords;
        float *chord = chord_sequence[chord_index];

        // Combine all notes in the chord
        int combined_value = 0;
        for (int i = 0; i < 4; i++) { // Chord has 4 notes
            combined_value += supersaw(t, chord[i] * 7); // Base frequency is 7
			//combined_value += supersaw(t, chord[i] * 7 * 2.0); // Multiply by 2.0 for an octave up

        }

        // Normalize the combined value to fit 8-bit audio
        combined_value /= 4;

        // Output the waveform
        putchar(combined_value);

        t++;
    }

    return 0;
}

