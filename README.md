# FSCE: fun and simple coding examples..

This is a simple repo to demonstrate some fundamental concepts, and a simple way for a Dad to get involved in his teenage kids' homework.  ;)

In our scenario, there is a) an Arduino-based project with workshop code from school, and b) Darwin/Linux-based workstations to use for other purposes.

The idea is to share code between both an Arduino firmware, and a simple C-based binary build product.

We will demonstrate how to write C code that will produce:

- Arduino project code suitable to use in school workshops
- Simulator code, suitable to use in learning C

Two areas of the project will function in parallel, the Arduino school project code, and the 'test/hack/experiment' playground in the C-based binary environment.

This will provide cross-platform build scaffolding methods and other tooling for us to construct other things.

## Projects:

# trafficlight 

A traffic-light simulator project intended to introduce the idea of a basic state machine as it might work in a traffic control system.

# voiceTracker

A simple implementation of a synth voice-assignment tracker, intended for use in simple MIDI-driven synthesizers which need to keep track of a fixed number of voices in, e.g. a polyphonic synthesizer context.

- functions_named_like_this are utility/test/stub functions.
- functionsNamedLikeThis are implementation functions.

'make test' to see two test cases, first with fixed note assignments and then with random assignments.  Run voice_tracker manually to have a non-sorted test output.

There's a .lua implementation too, just for fun

# megaScheduler/

A directory containing a series of experiments resulting in a Cooperative Multitasking System in C.

🔍 Examined a coroutine-style macro

	Analyzed the hc_task_yield macro using __LINE__ and case labels.
	Explained how it enables coroutine-style task pausing/resuming without threads.

🧪 Created a basic test program

	Made a simple example called test_hc.c using a single coroutine task.
	Demonstrated how hc_task_yield and hc_task_delay work with a scheduler loop.

⚙️ Evolved into a micro cooperative scheduler

	Added support for multiple tasks.
	Introduced a simple round-robin scheduler with delay management.
	
📈 Advanced to a more capable system

	Added:

		Task priorities
		Task groups/tags for suspend/resume
		Alarms/timers
		Event flags
		Task restart/reset functionality

🔒 Focused on embedded-friendliness

	Kept everything in a single .c file.
	No dynamic allocation or OS dependencies.
	Built-in support for:
	Cooperative multitasking
	Debugging
	Logging

🐞 Caught and fixed a macro bug

	Fixed the case-not-in-switch compile error by wrapping coroutine logic in switch (task->state) { ... }.

📦 Finalized the full system (mega_hc.c)

	Created a robust single-file multitasking framework with:

		Task priority sorting
		Group suspend/resume
		Watchdog timers for stuck tasks
		CLI debug shell (basic)
		Coroutine-based task behavior
		Snapshot and restart logic
		Included heavy commenting and usage examples for clarity.

# Using the CLI in mega_hc.c
	The CLI in the program is a basic interface that allows you to interact with the multitasking system during runtime. Here's how to use it:

	How the CLI Works:

	Every 5 seconds, the program will prompt you for input.
	You can type a command, and the system will execute it.

	Available Commands:

		dump:
			Description: Displays the current state of all tasks in the system.
			Example:
			> dump
			Output: Lists each active task, its priority, group, suspension status, and wake-up time.

		suspend <group>:
			Description: Suspends all tasks in the specified group, preventing them from running.
			Example:
			> suspend 1
			Effect: Suspends tasks in Group 1 (you can change the group number to target different groups).

		resume <group>:
			Description: Resumes tasks in the specified group, allowing them to run again.
			Example:
			> resume 1
			Effect: Resumes tasks in Group 1 (tasks will continue based on their wake-up time and priority).

	How to Input Commands:

		You can enter commands by typing them and pressing Enter.

		The system processes the input and prints any relevant output (e.g., the state of tasks or confirmation of suspension/resumption).

	Notes:

		The CLI waits for input every 5 seconds, so it won’t block the program's execution.

		Task suspension and resumption only affect tasks within the same group, making it easy to manage related tasks.

	Example CLI Session:

			[CLI] > suspend 1
			[Log] Task 1 suspended.
			[CLI] > dump
			[Snapshot] Task States
			 - Task 0 | Prio 3 | Group 0 | Susp 0 | WT: 5000
			 - Task 1 | Prio 2 | Group 1 | Susp 1 | WT: 3000
			 - Task 2 | Prio 1 | Group 1 | Susp 0 | WT: 1000
			[CLI] > resume 1
			[Log] Task 1 resumed.
			[CLI] > dump
			[Snapshot] Task States
			 - Task 0 | Prio 3 | Group 0 | Susp 0 | WT: 5000
			 - Task 1 | Prio 2 | Group 1 | Susp 0 | WT: 3000
			 - Task 2 | Prio 1 | Group 1 | Susp 0 | WT: 1000



