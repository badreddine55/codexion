*This project has been created as part of the 42 curriculum by badiyaf.*

# Codexion

## Description

Codexion is a multi-threaded C simulation based on the Dining Philosophers problem. A ring of "coders" compete for shared "dongles" to compile code. Each coder needs two dongles (left and right) at once to compile, then releases them to debug and refactor before requesting again. If a coder goes too long without compiling, it "burns out" and the simulation stops.

The project explores thread synchronization: mutexes, deadlock prevention, starvation prevention, timed cooldowns, and two dongle-scheduling policies (fifo and EDF).

## Instructions

```bash
make          # build
make clean    # remove object files
make fclean   # remove object files and binary
make re       # rebuild everything
```

Run:
```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Parameter | Description |
|---|---|
| `number_of_coders` | Number of coders (and dongles) in the ring |
| `time_to_burnout` | Max ms a coder can go without starting a compile |
| `time_to_compile` | Ms spent compiling (holding both dongles) |
| `time_to_debug` | Ms spent debugging (dongles released) |
| `time_to_refactor` | Ms spent refactoring (dongles released) |
| `number_of_compiles_required` | Compiles needed per coder before the sim stops normally |
| `dongle_cooldown` | Ms a dongle must sit idle after release before reuse |
| `scheduler` | `fifo` or `edf` |

Example:
```bash
./codexion 4 3000 200 200 200 8 100 fifo
```

## Blocking Cases Handled

**Deadlock.** A coder never holds one dongle while waiting for the other — acquisition checks both dongles together and only takes them if both are free at once. Locks on the two dongles are always taken in a fixed order (by memory address) so two coders can never lock them in opposite order and deadlock.

**Starvation.**
- Under `fifo`, each dongle keeps a queue of requesting coder IDs; a coder only gets the dongle once it's at the front of both queues it needs.
- Under `edf`, priority goes to whichever waiting coder has the closest burnout deadline (`last_compile_start + time_to_burnout`). Ties are broken by whoever has been waiting longest, not by coder ID — this avoids a fixed priority order that would starve some coders indefinitely.

**Cooldown.** Every dongle release records a timestamp. A dongle can't be re-acquired until `dongle_cooldown` ms have passed since that release.

**Burnout detection.** A separate monitor thread checks every coder's `last_compile_start` roughly once per millisecond. If more than `time_to_burnout` ms have passed since a coder started its last compile, the monitor logs `"burned out"` and stops the simulation. All coder waits are done in short slices so they notice the stop signal quickly.

**Log integrity.** All log lines go through one shared lock, so two coders' output can never interleave mid-line, and no message can print after the final `"burned out"` line.

## Thread Synchronization Mechanisms

Each coder runs in its own `pthread_t`. A separate monitor thread watches for burnout and normal completion.

Mutexes used:
- `dongle->lock` — protects each dongle's owner, queue, and release timestamp.
- `coder->meal_lock` — protects `last_compile_start` and `finished`, since both the coder thread and the monitor thread read/write them.
- `sim->log_lock` — serializes all console output.
- `sim->stop_lock` — protects the shared stop flag.
- `sim->finished_lock` — protects the count of coders that finished normally.

Example of the coder/monitor race this prevents: the coder thread updates `last_compile_start` when it starts compiling, while the monitor thread reads it continuously to check for burnout. Without a lock, one thread could read a half-written value. Both sides only touch this field while holding `meal_lock`:

```c
/* coder, starting a compile */
pthread_mutex_lock(&coder->meal_lock);
coder->last_compile_start = fr_get_time_ms();
pthread_mutex_unlock(&coder->meal_lock);

/* monitor, checking for burnout */
pthread_mutex_lock(&coder->meal_lock);
last_compile_start = coder->last_compile_start;
pthread_mutex_unlock(&coder->meal_lock);
```

The stop flag works the same way — any thread can check it safely:
```c
int fr_check_stop(t_sim *sim)
{
    int flag;

    pthread_mutex_lock(&sim->stop_lock);
    flag = sim->stop_flag;
    pthread_mutex_unlock(&sim->stop_lock);
    return (flag);
}
```
Every coder's sleep periods call this repeatedly in small increments, so once the flag is set, threads stop within a few milliseconds instead of finishing their current wait first.

## Resources

- [POSIX Threads Programming (LLNL)](https://hpc-tutorials.llnl.gov/posix/)
- [Dining Philosophers Problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Coffman Conditions](https://en.wikipedia.org/wiki/Coffman_conditions)
- [Earliest Deadline First Scheduling](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)

## AI Usage

AI was used as a learning and development assistant to explain concepts and help understand the project's behavior, particularly **Burnout, Cooldown, fifo, EDF, synchronization, and dongle management**.

It was also used to analyze test results, identify possible issues, and clarify the reasoning behind different scheduling behaviors.

The AI was used for **explanation, debugging guidance, and conceptual understanding**. The code was implemented and tested within the project, and the final results were verified through actual execution.

All code was reviewed and understood before being included in the project.