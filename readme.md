# Multiprogramming Operating System (MOS) - Phase 2 Simulator

A comprehensive simulation of a Multiprogramming Operating System (MOS) written in C. This project models a simplified virtual machine incorporating paged memory allocation, instruction execution, dynamic page-fault handling, interrupt processing, and error termination.

## Project Description

This simulator implements a **2-Pass Multi-Programming Operating System (MOS)** architecture with virtual memory management. The simulated system separates user-space virtual addresses from physical memory frames, resolving them dynamically through page tables. It simulates CPU components, registers, physical memory, and operating system-level routines like interrupt handling (Supervisor, Program, and Time interrupts).

### System Specifications & Architecture

```
                                    +-----------------------+
                                    |     Physical Memory   |
                                    |    M[300] (30 frames) |
                                    +-----------+-----------+
                                                ^
                                                | Real Address (RA)
+-----------------------+           +-----------+-----------+
|  Virtual Address (VA) |           |  Address Translation  |
|      00 to 99         |---------->|    p = VA / 10        |
+-----------------------+           |    d = VA % 10        |
                                    |    RA = Frame * 10 + d|
                                    +-----------+-----------+
                                                ^
                                                |
                                    +-----------+-----------+
                                    |  Page Table Register  |
                                    |    PTR (Base Frame)   |
                                    +-----------------------+
```

*   **Main Memory ($M$):** 300 words of memory (`M[300][4]`). Each word is 4 characters/bytes. The physical memory is divided into 30 frames (each containing 10 words).
*   **Virtual Memory Space:** A job has a virtual address space of 100 words (00 to 99), which is mapped to 10 virtual pages (Page 0 to Page 9).
*   **Page Table Register (PTR):** A pointer register containing the physical base address of the Page Table (stored in a randomly allocated physical frame). The page table translates virtual page numbers ($0-9$) to physical frame numbers ($0-29$).
*   **Instruction Register (IR):** 4-byte register holding the currently executing instruction.
*   **General Purpose Register ($R$):** 4-byte accumulator register for arithmetic and data transfer operations.
*   **Instruction Counter (IC):** 2-digit counter containing the virtual address of the next instruction to be fetched.
*   **Toggle Register ($C$):** 1-bit boolean register storing the status of comparison operations (true/false).
*   **Interrupt Registers:**
    *   **SI (Supervisor Interrupt):** Handles OS-level I/O operations and termination.
    *   **PI (Program Interrupt):** Handles hardware/execution errors like invalid opcodes, operand errors, and page faults.
    *   **TI (Time Interrupt):** Triggers when the user job execution time exceeds the predefined limit.

---

## Instruction Set Architecture (ISA)

The simulator executes instructions of format `OPxx` where `OP` is a 2-character opcode and `xx` is a 2-digit virtual memory address operand (except for `H` which has no operand).

| Instruction | Name | Description | Time (TTC) |
| :--- | :--- | :--- | :--- |
| **`GDxx`** | Get Data | Read a card (up to 40 characters / 10 words) from the input file and store it starting at virtual address `xx` (addresses `xx` to `xx+9`). | +1 |
| **`PDxx`** | Put Data | Write 10 words starting at virtual address `xx` from memory to the output file. | +1 |
| **`LRxx`** | Load Register | Load the 4-byte word from virtual address `xx` into the general-purpose register $R$. | +1 |
| **`SRxx`** | Store Register | Store the 4-byte contents of register $R$ into the virtual address `xx`. | +1 |
| **`CRxx`** | Compare Register | Compare the contents of register $R$ with the 4-byte word at virtual address `xx`. Sets Toggle $C$ to `1` if equal, else `0`. | +1 |
| **`BTxx`** | Branch on True | Branch to virtual address `xx` by updating the Instruction Counter ($IC = xx$) if Toggle $C$ is `1`. | +1 |
| **`H`** | Halt | Terminate execution of the current job. | 0 |

---

## Interrupt & Error Handling (MOS Control Loop)

The simulator checks and processes three types of interrupts:

### 1. Supervisor Interrupts (`SI`)
*   **`SI = 1` (GD - Read):** Invokes the reader subroutine. Maps the virtual target pages dynamically; if page faults occur during reading, physical frames are allocated.
*   **`SI = 2` (PD - Write):** Invokes the writer subroutine. Checks and increments the Line Limit Counter (`LLC`).
*   **`SI = 3` (Halt - End of Job):** Terminates the job with status `NO ERROR`.

### 2. Program Interrupts (`PI`)
*   **`PI = 1` (Opcode Error):** Triggers when the instruction read into `IR` is invalid/unknown.
*   **`PI = 2` (Operand Error):** Triggers when an operand contains non-digit characters.
*   **`PI = 3` (Page Fault):** Triggers when trying to access a page that has not been mapped yet. If valid (e.g., during memory loading or memory writes), a frame is allocated at random, page table updated, and program execution resumes.

### 3. Time Interrupts (`TI`)
*   **`TI = 2` (Time Limit Exceeded):** Triggers when the Total Time Counter (`TTC`) exceeds the Time Limit (`TL`) specified in the `$AMJ` header.

### Termination & Error Log Codes

At job termination, the output file receives a log block identifying the exit state:

| Code | Label | Cause |
| :--- | :--- | :--- |
| **`0`** | `NO ERROR` | Normal completion via `H` instruction. |
| **`1`** | `TIME LIMIT EXCEEDED` | Executed instruction cycles `TTC` exceeded the job's `TL`. |
| **`2`** | `LINE LIMIT EXCEEDED` | Written output lines `LLC` exceeded the job's `LL`. |
| **`3`** | `OPERAND ERROR` | Instruction operand was invalid (e.g., `LRAB`). |
| **`4`** | `OPCODE ERROR` | Instruction code was unrecognized (e.g., `XX20`). |

---

## Project Structure

*   [`globals.h`](file:///home/ayush/OS_Assignment/CP_Phase2/globals.h) / [`globals.c`](file:///home/ayush/OS_Assignment/CP_Phase2/globals.c): Define variables for storage (`M`), registers (`IR`, `R`, `C`, `IC`), counters (`TTC`, `LLC`), limits (`TL`, `LL`), interrupt states (`SI`, `PI`, `TI`), page tables, and execution status parameters.
*   [`utils.h`](file:///home/ayush/OS_Assignment/CP_Phase2/utils.h) / [`utils.c`](file:///home/ayush/OS_Assignment/CP_Phase2/utils.c): Contain string manipulation utilities, limit parser (`countLimit`), and the random page-frame generator (`randomNumberGenerator`).
*   [`code.c`](file:///home/ayush/OS_Assignment/CP_Phase2/code.c): Core OS driver program implementing:
    *   `load()`: Reads instructions into virtual memory, setting up the initial Page Table.
    *   `executeProgram()`: Main simulation clock cycle parsing instructions, translating addresses, handling page faults, updating registers, and setting interrupts.
    *   `MOS()`: Simulates the kernel interface handling the supervisor, program, and timer interrupts.
    *   `read()` & `write()`: Low-level I/O routines executing file inputs and outputs.
    *   `terminate()`: Appends job logs (Job ID, termination status, counters) to `output.txt`.
*   [`input.txt`](file:///home/ayush/OS_Assignment/CP_Phase2/input.txt): User jobs stream containing control cards (`$AMJ`, `$DTA`, `$END`), user instructions, and program inputs.
*   [`output.txt`](file:///home/ayush/OS_Assignment/CP_Phase2/output.txt): File containing printed data outputs and termination logs.

---

## Input Card Specification

Jobs in `input.txt` must follow the format below:
1.  **Job Header Card:**
    `$AMJ{JobID(4-digit)}{TimeLimit(4-digit)}{LineLimit(4-digit)}`
    *Example:* `$AMJ040100120006` (Job 0401, Time Limit 12 instructions, Line Limit 6 output lines).
2.  **Instruction Card(s):** User program instructions (loaded into memory page-by-page).
3.  **Data Card Marker:**
    `$DTA`
4.  **Data Card(s):** Input lines to be read by `GD` instructions.
5.  **Job End Card:**
    `$END{JobID(4-digit)}`
    *Example:* `$END0401`

---

## How to Build and Run the Simulator

### Prerequisites
*   A GCC compiler or any compatible C compiler.
*   Standard terminal environment.

### Steps to Run

1.  Open your terminal in the project directory:
    ```bash
    cd "/home/ayush/OS_Assignment/CP_Phase2/"
    ```

2.  Compile the source files:
    ```bash
    gcc code.c utils.c globals.c -o code
    ```

3.  Ensure the input stream is configured in [`input.txt`](file:///home/ayush/OS_Assignment/CP_Phase2/input.txt).

4.  Execute the binary:
    ```bash
    ./code
    ```

5.  Review the generated outputs and status reports in [`output.txt`](file:///home/ayush/OS_Assignment/CP_Phase2/output.txt). A console log will also display real-time memory state dumps and page allocation updates.