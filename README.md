# C Shell — Custom Unix Shell

A fully-featured custom Unix shell written in C, implementing a command-line interpreter with parsing, built-in commands, I/O redirection, piping, job control, and signal handling.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Building](#building)
- [Usage](#usage)
- [Feature Details](#feature-details)
  - [Part A — Shell Basics](#part-a--shell-basics)
  - [Part B — Shell Intrinsics](#part-b--shell-intrinsics)
  - [Part C — I/O Redirection & Pipes](#part-c--io-redirection--pipes)
  - [Part D — Sequential & Background Execution](#part-d--sequential--background-execution)
  - [Part E — Exotic Shell Intrinsics](#part-e--exotic-shell-intrinsics)

---

## Overview

This project implements a custom Unix shell from scratch in C. It replicates core functionality found in shells like `bash` and `zsh`, including an interactive prompt, command parsing via a context-free grammar, built-in commands (`hop`, `reveal`, `log`), I/O redirection (`<`, `>`, `>>`), command piping (`|`), sequential (`;`) and background (`&`) execution, and full job control with signal handling (`Ctrl-C`, `Ctrl-D`, `Ctrl-Z`, `fg`, `bg`, `ping`, `activities`).

---

## Features

| Category | Feature | Description |
|---|---|---|
| **Prompt** | Dynamic prompt | Displays `<Username@SystemName:path>` with `~` substitution for the shell's home directory |
| **Parsing** | CFG-based parser | Tokenizer and recursive-descent parser for the shell grammar supporting pipes, redirections, and delimiters |
| **Built-ins** | `hop` | Change directory (`~`, `.`, `..`, `-`, or a path), supports multiple sequential arguments |
| **Built-ins** | `reveal` | List directory contents with optional `-a` (show hidden) and `-l` (line-by-line) flags |
| **Built-ins** | `log` | Persistent command history (up to 15 entries) with `purge` and `execute <index>` subcommands |
| **I/O** | Input redirection | `command < file` — redirect stdin from a file |
| **I/O** | Output redirection | `command > file` (overwrite) and `command >> file` (append) |
| **I/O** | Piping | `cmd1 \| cmd2 \| ... \| cmdN` — connect stdout of one command to stdin of the next |
| **Execution** | Sequential | `cmd1 ; cmd2` — execute commands one after another |
| **Execution** | Background | `cmd &` — run a command in the background |
| **Job Control** | `fg` / `bg` | Bring background/stopped jobs to the foreground or resume them in the background |
| **Job Control** | `activities` | List all running and stopped background processes |
| **Job Control** | `ping` | Send a signal to a process by PID |
| **Signals** | `Ctrl-C` | Send `SIGINT` to the foreground process (shell stays alive) |
| **Signals** | `Ctrl-Z` | Send `SIGTSTP` to the foreground process, move it to background as stopped |
| **Signals** | `Ctrl-D` | Send `SIGKILL` to all child processes and exit the shell gracefully |

---

## Project Structure

```
shell/
├── Makefile                  # Build configuration (gcc, -Wall -g)
├── include/                  # Header files
│   ├── shell.h               # Global state, structs, and extern declarations
│   ├── parser.h              # Tokenizer and recursive-descent parser interface
│   ├── input.h               # Prompt display functions
│   ├── hop.h                 # hop (cd) command
│   ├── reveal.h              # reveal (ls) command
│   ├── log.h                 # log (history) command
│   ├── execute.h             # Top-level command execution dispatcher
│   ├── jobs.h                # Job splitting by ';' and '&' delimiters
│   ├── lets_go.h             # Pipe and I/O redirection parsing into cmd_grp structs
│   ├── static_run.h          # Pipeline execution engine
│   ├── store_logs.h          # Log persistence (read/write to file)
│   ├── linkedlist.h          # Background job linked list operations
│   ├── activites.h           # activities command
│   ├── bgfg.h                # fg and bg commands
│   └── ping.h                # ping command
└── src/                      # Source files
    ├── main.c                # Entry point, main loop, signal handlers
    ├── input.c               # Prompt generation (<user@host:path>)
    ├── parser.c              # CFG tokenizer and recursive-descent validator
    ├── execute.c             # Command dispatcher (built-ins vs. external, fg vs. bg)
    ├── jobs.c                # Splits parsed tokens into job groups by ';' and '&'
    ├── lets_go.c             # Splits a job into pipeline stages, handles '<', '>', '>>'
    ├── static_run.c          # Forks children, sets up pipes, and executes pipeline stages
    ├── hop.c                 # hop command implementation (chdir logic)
    ├── reveal.c              # reveal command implementation (opendir/readdir, sorting)
    ├── log.c                 # log command (print, purge, execute history)
    ├── store_logs.c          # Persistent log storage across sessions (reads/writes logs.txt)
    ├── linkedlist.c          # Background job linked list (add, remove, find)
    ├── activites.c           # activities command (list background jobs with status)
    ├── bgfg.c                # fg and bg command implementations
    └── ping.c                # ping command (send signal to PID)
```

---

## Building

```bash
cd shell
make
```

This compiles all source files and produces the executable `shell.out`.

To clean build artifacts:

```bash
make clean
```

**Requirements:** GCC and a Unix-like OS (Linux / macOS).

---

## Usage

```bash
# Start the shell
./shell.out

# The prompt appears as:
<username@hostname:~>
```

### Example Session

```
<rudy@iiit:~> hop ..
<rudy@iiit:/home> reveal
rudy
<rudy@iiit:/home> hop ~
<rudy@iiit:~> reveal -la
.git
.gitignore
Makefile
README.md
include
src
shell.out
<rudy@iiit:~> echo hello > output.txt
<rudy@iiit:~> cat < output.txt
hello
<rudy@iiit:~> cat output.txt | wc -l
1
<rudy@iiit:~> sleep 10 &
[1] 12345
<rudy@iiit:~> activities
[12345] : sleep 10 - Running
<rudy@iiit:~> ping 12345 9
Sent signal 9 to process with pid 12345
<rudy@iiit:~> log
hop ..
reveal
hop ~
reveal -la
echo hello > output.txt
cat < output.txt
cat output.txt | wc -l
sleep 10 &
ping 12345 9
<rudy@iiit:~> log purge
<rudy@iiit:~> log
<rudy@iiit:~>
```

---

## Feature Details

### Part A — Shell Basics

#### A.1: Shell Prompt
- Displays `<Username@SystemName:current_path>` using `getpwuid()` and `gethostname()`.
- The directory where the shell is launched becomes the shell's **home directory**.
- If the current working directory is inside the home directory, the home prefix is replaced with `~`.
- Otherwise, the full absolute path is shown.

#### A.2: User Input
- The shell reads user input via `fgets()`, strips trailing newlines, and re-displays the prompt after each command.

#### A.3: Input Parsing
- Implements a **tokenizer** that splits input into tokens (names, operators: `|`, `&`, `;`, `<`, `>`, `>>`).
- A **recursive-descent parser** validates input against the shell's context-free grammar:
  ```
  shell_cmd  →  cmd_group (('&' | ';') cmd_group)* '&'?
  cmd_group  →  atomic ('|' atomic)*
  atomic     →  name (name | input | output)*
  input      →  '<' name
  output     →  ('>' | '>>') name
  name       →  r"[^|&><;\s]+"
  ```
- Invalid syntax results in the message: `Invalid Syntax!`

---

### Part B — Shell Intrinsics

#### B.1: `hop`
- **Syntax:** `hop ((~ | . | .. | - | name)*)?`
- Changes the current working directory. Supports multiple arguments processed sequentially.
- `~` → home, `..` → parent, `.` → stay, `-` → previous directory, `name` → relative/absolute path.
- Prints `No such directory!` if a path doesn't exist.

#### B.2: `reveal`
- **Syntax:** `reveal (-(a | l)*)* (~ | . | .. | - | name)?`
- Lists directory contents sorted lexicographically (ASCII order).
- `-a` flag shows hidden files (those starting with `.`).
- `-l` flag displays one entry per line.
- Flags can be combined and repeated (e.g., `-lalalalaaaalal`).
- Prints `No such directory!` for invalid paths and `reveal: Invalid Syntax!` for too many arguments.

#### B.3: `log`
- **Syntax:** `log (purge | execute <index>)?`
- Command history persists across sessions in `logs.txt` (stored in the shell's home directory).
- Stores up to 15 commands. Oldest commands are overwritten when full.
- Duplicate consecutive commands are not stored.
- Commands containing `log` are not stored in history.
- `log` — prints history (oldest to newest).
- `log purge` — clears history.
- `log execute <index>` — re-executes the command at the given index (1-indexed, newest first).

---

### Part C — I/O Redirection & Pipes

#### C.1: Command Execution
- External commands are executed via `fork()` + `execvp()`.
- Built-in commands (`hop`, `log`, `reveal`, `fg`, `bg`, `ping`, `activities`) are executed directly in the parent or handled specially to avoid unnecessary forking.
- Unknown commands print: `Command not found!`

#### C.2: Input Redirection (`<`)
- Opens the specified file with `O_RDONLY` and redirects `STDIN_FILENO` via `dup2()`.
- If the file doesn't exist: `No such file or directory`.
- When multiple input redirections are present, only the last one takes effect.

#### C.3: Output Redirection (`>`, `>>`)
- `>` creates/truncates the file (`O_CREAT | O_TRUNC | O_WRONLY`).
- `>>` appends to the file (`O_WRONLY | O_CREAT | O_APPEND`).
- If the file can't be created: `Unable to create file for writing`.
- When multiple output redirections are present, only the last one takes effect.
- Input and output redirection work together (e.g., `cmd < in.txt > out.txt`).

#### C.4: Command Piping (`|`)
- Creates pipes via `pipe()` for each `|` operator.
- Each command in the pipeline runs in its own forked child process.
- `stdout` of command `i` connects to `stdin` of command `i+1`.
- File redirection and pipes work together (e.g., `cmd1 < in.txt | cmd2 > out.txt`).
- The parent waits for all processes in the pipeline to complete.

---

### Part D — Sequential & Background Execution

#### D.1: Sequential Execution (`;`)
- Commands separated by `;` execute one after another.
- The shell waits for each command to complete before starting the next.
- If one command fails, subsequent commands still execute.

#### D.2: Background Execution (`&`)
- Commands ending with `&` run in the background.
- The shell prints `[job_number] process_id` and immediately shows a new prompt.
- Background processes have their stdin redirected to `/dev/null`.
- On next user input, the shell checks for completed background processes and reports:
  - `command_name with pid process_id exited normally`
  - `command_name with pid process_id exited abnormally`

---

### Part E — Exotic Shell Intrinsics

#### E.1: `activities`
- Lists all background processes spawned by the shell.
- Output format: `[pid] : command_name - State` (State is `Running` or `Stopped`).
- Results are sorted lexicographically by command name.

#### E.2: `ping`
- **Syntax:** `ping <pid> <signal_number>`
- Sends `signal_number % 32` to the specified PID using `kill()`.
- Prints `Sent signal <signal_number> to process with pid <pid>` on success.
- Prints `No such process found` if the PID doesn't exist.

#### E.3: Signal Handling
- **Ctrl-C (`SIGINT`):** Forwards `SIGINT` to the current foreground process group. The shell itself does not terminate.
- **Ctrl-Z (`SIGTSTP`):** Forwards `SIGTSTP` to the current foreground process group, moving it to the background as a stopped job.
- **Ctrl-D (EOF):** Sends `SIGKILL` to all child processes, prints `logout`, and exits the shell.

#### E.4: `fg` and `bg`
- **`fg [job_number]`** — Brings a background/stopped job to the foreground. Sends `SIGCONT` if stopped. Waits for the job to finish or stop again. Uses the most recent job if no number is provided.
- **`bg [job_number]`** — Resumes a stopped background job by sending `SIGCONT`. Prints `Job already running` if the job is already running.
- Both print `No such job` if the specified job doesn't exist.

---

## Architecture

The shell follows a **read-eval-print loop (REPL)** architecture:

```
┌──────────────┐
│   main.c     │  Main loop: prompt → read → parse → execute → repeat
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   input.c    │  Generates the <user@host:path> prompt
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   parser.c   │  Tokenizes input, validates against CFG
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   execute.c  │  Dispatches commands: splits by ';' and '&', routes to built-ins or pipeline
└──────┬───────┘
       │
  ┌────┴─────┐
  ▼          ▼
┌────────┐ ┌──────────────┐
│ jobs.c │ │ static_run.c │  Pipeline execution: fork, pipe, dup2, execvp
└────────┘ └──────┬───────┘
                  │
             ┌────┴─────┐
             ▼          ▼
        ┌─────────┐ ┌────────────┐
        │lets_go.c│ │linkedlist.c│  I/O redirection parsing & job list management
        └─────────┘ └────────────┘
```

---

## System Calls Used

| System Call | Purpose |
|---|---|
| `fork()` | Create child processes for command execution |
| `execvp()` | Execute external commands |
| `pipe()` | Create pipes for command piping |
| `dup2()` | Redirect file descriptors (stdin/stdout) |
| `open()` / `close()` | File I/O for redirection |
| `waitpid()` | Wait for child processes (with `WUNTRACED` and `WNOHANG`) |
| `chdir()` / `getcwd()` | Change and get current working directory |
| `opendir()` / `readdir()` | List directory contents |
| `kill()` / `killpg()` | Send signals to processes/process groups |
| `setpgid()` | Set process group IDs for job control |
| `tcsetpgrp()` | Transfer terminal control between shell and foreground jobs |
| `signal()` | Install signal handlers for `SIGINT`, `SIGTSTP`, `SIGTTOU`, `SIGTTIN` |
| `getpwuid()` / `gethostname()` | Retrieve username and hostname for the prompt |
