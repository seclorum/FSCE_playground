// 
// Basic trafficlight code
// RED/AMBER/GREEN lights
// 
// PRODUINO: Arduino Project code
// MAKE_CBIN: C bin file for simulation
//

// !J! hard-code this for now
#define MAKE_CBIN

#undef MAKE_PRODUINO

#ifdef MAKE_PRODUINO

#define LEDRED 2
#define LEDGREEN 3
#define LEDYELLOW 4

void project_code_setup()
{
	pinMode(LEDRED, OUTPUT);
	pinMode(LEDGREEN, OUTPUT);
	pinMode(LEDYELLOW, OUTPUT);
}

void project_code_loop()
{
	digitalWrite(LEDRED, HIGH);
	delay(5000);
	digitalWrite(LEDYELLOW, HIGH);
	delay(2000);
	digitalWrite(LEDRED, LOW);
	digitalWrite(LEDYELLOW, LOW);
	digitalWrite(LEDGREEN, HIGH);
	delay(5000);
	digitalWrite(LEDGREEN, LOW);
	delay(250);
	digitalWrite(LEDGREEN, HIGH);
	delay(250);
	digitalWrite(LEDGREEN, LOW);
	delay(250);
	digitalWrite(LEDGREEN, HIGH);
	delay(250);
	digitalWrite(LEDGREEN, LOW);
	delay(250);
	digitalWrite(LEDGREEN, HIGH);
	delay(250);
	digitalWrite(LEDGREEN, LOW);
	delay(250);
	digitalWrite(LEDGREEN, HIGH);
	delay(250);
	digitalWrite(LEDGREEN, LOW);
	digitalWrite(LEDYELLOW, HIGH);
	delay(2000);
	digitalWrite(LEDYELLOW, LOW);
}
 
// !J! We will use these to 'insert' the cbin
// functions once we know they work
void setup()
{
	project_code_setup();
}

void loop()
{
	project_code_loop();
}
#endif


#ifdef MAKE_CBIN
// traffic-light implementation as a state machine
#include <stdio.h>
#include <unistd.h>

// Enumeration for the traffic light states
typedef enum {
    TRAFFIC_STOP,      // Red solid for 5 seconds
    TRAFFIC_WARN_1,    // Amber blinking for 3 seconds
    TRAFFIC_WARN_2,    // Amber solid for 2 seconds
    TRAFFIC_GO,        // Green solid for 6 seconds
    TRAFFIC_FAULT_1,   // Amber blinking rapidly
    TRAFFIC_FAULT_2    // All lights on
} TrafficLightState;

// Timing constants (in arbitrary units)
#define STOP_TIME      5
#define WARN_1_TIME    3
#define WARN_2_TIME    2
#define GO_TIME        6

// State variables
TrafficLightState current_state = TRAFFIC_STOP;
int state_timer = 0;


// Function prototypes - declared in advance:

// Updates the current state of the machine
void state_update(void);
// sets the lights according to the given state
void set_lights(TrafficLightState state);



// Function implementations:

// Main update function for the state machine
void state_update() {

    // Advances the state machine based on current state and time values
    switch (current_state) {
        case TRAFFIC_STOP:
            if (++state_timer >= STOP_TIME) {
                current_state = TRAFFIC_WARN_1;
                state_timer = 0;
            }
            break;

        case TRAFFIC_WARN_1:
            if (++state_timer >= WARN_1_TIME) {
                current_state = TRAFFIC_WARN_2;
                state_timer = 0;
            }
            break;

        case TRAFFIC_WARN_2:
            if (++state_timer >= WARN_2_TIME) {
                current_state = TRAFFIC_GO;
                state_timer = 0;
            }
            break;

        case TRAFFIC_GO:
            if (++state_timer >= GO_TIME) {
                current_state = TRAFFIC_STOP;
                state_timer = 0;
            }
            break;

        case TRAFFIC_FAULT_1:
            // Remains in fault state until manually handled
            break;

        case TRAFFIC_FAULT_2:
            // Remains in fault state until manually handled
            break;

        default:
            // Reset to safe state if an unknown state is encountered
            current_state = TRAFFIC_STOP;
            state_timer = 0;
            break;
    }

    // Update the lights based on the current state
    set_lights(current_state);
}

// Function to control the lights based on the current state
void set_lights(TrafficLightState state) {
    switch (state) {
        case TRAFFIC_STOP:
            printf("Red: ON, Amber: OFF, Green: OFF\n");
            break;

        case TRAFFIC_WARN_1:
            if (state_timer % 2 == 0) {
                printf("Red: OFF, Amber: ON (blinking), Green: OFF\n");
            } else {
                printf("Red: OFF, Amber: OFF, Green: OFF\n");
            }
            break;

        case TRAFFIC_WARN_2:
            printf("Red: OFF, Amber: ON (solid), Green: OFF\n");
            break;

        case TRAFFIC_GO:
            printf("Red: OFF, Amber: OFF, Green: ON\n");
            break;

        case TRAFFIC_FAULT_1:
            if (state_timer % 1 == 0) { // Faster blinking than WARN_1
                printf("Red: OFF, Amber: ON (rapid blinking), Green: OFF\n");
            } else {
                printf("Red: OFF, Amber: OFF, Green: OFF\n");
            }
            break;

        case TRAFFIC_FAULT_2:
            printf("Red: ON, Amber: ON, Green: ON\n");
            break;

        default:
            printf("Unknown state\n");
            break;
    }
}

// Example main function to demonstrate the state machine
int main() {
    // Simulating state updates in a loop
    for (int i = 0; i < 30; i++) {
        state_update();
        // Simulate time passing
        sleep(1); // Replace with delay if on embedded system
    }

    return 0;
}
#endif
// MAKE_CBIN





