# Virtual-Memory-Simulator
This C program simulates virtual memory management using different page replacement algorithms: NRU (Not Recently Used), LRU (Least Recently Used), Second-Chance, and Optimal (OTM). It processes a memory access log, calculates page faults, and tracks dirty pages (written back to disk).

### **Overview**
The program emulates how an operating system manages virtual memory using **page replacement algorithms** such as:
1. **NRU (Not Recently Used)**
2. **LRU (Least Recently Used)**
3. **Second-Chance Algorithm**
4. **Optimal Page Replacement (OTM)**

It reads a memory access log and simulates the process of managing virtual memory with a fixed-size physical memory. It calculates:
- **Page faults**: Number of times a requested page is not in memory.
- **Dirty pages**: Pages written back to disk when replaced.

---

### **Key Components**
1. **Structures**:
   - **Page**: Represents a virtual memory page with properties like reference bit (`R`), modification bit (`M`), validity status, and frame number in physical memory.
   - **Quadro**: Represents a physical memory frame, tracking whether it is occupied and the page it contains.

2. **Page Table and Physical Memory**:
   - **Page Table** (`tab_pags`): Tracks the state of all virtual memory pages.
   - **Frames** (`quadros`): Represents physical memory divided into fixed-size frames.

3. **Utility Functions**:
   - **`getIndicePag`**: Calculates the page index from a logical address.
   - **`calcular_qtd_paginas`**: Determines the number of virtual pages based on the page size.

---

### **Simulation Function (`sim_virtual`)**
This function handles the core simulation, performing the following steps:
1. **Initialization**:
   - Parse arguments such as the algorithm, memory size, and page size.
   - Allocate and initialize the page table and frames.
   - Open the input file containing memory references.

2. **Page Replacement Logic**:
   - **NRU Algorithm**:
     - Periodically resets reference bits.
     - Prioritizes replacement of pages based on combinations of `R` and `M` bits.
   - **LRU Algorithm**:
     - Tracks the last access time for each page.
     - Replaces the page least recently accessed.
   - **Second-Chance Algorithm**:
     - Implements a circular buffer.
     - Skips pages with `R=1` and gives them a "second chance" by resetting their reference bit.
   - **Optimal Algorithm (OTM)**:
     - Analyzes future memory references to replace the page that will not be used for the longest time.

3. **Output**:
   - After processing all references, it prints:
     - Total page faults.
     - Number of dirty pages written back to disk.

---

### **Main Function**
The `main` function validates input arguments and invokes `sim_virtual` with appropriate parameters:
- Algorithm name.
- Input file path.
- Page size in KB.
- Physical memory size in MB.

---

### **How It Works**
1. **Input Format**:
   The program expects a log file with lines formatted as:
   ```
   <logical_address> <R/W>
   ```
   Example:
   ```
   0x004F W
   0x00A3 R
   ```

2. **Execution Flow**:
   - Logical addresses are mapped to virtual pages using the page size.
   - Each access checks if the page is in memory:
     - If **present**, update access metadata (e.g., `R` and `M` bits).
     - If **absent**, trigger a **page fault** and use the chosen algorithm to decide which page to replace.

3. **Performance Metrics**:
   - **Page faults** indicate how often memory access requires loading a page from disk.
   - **Dirty pages** measure how many modified pages were written back to disk upon replacement.

---

### **Example Usage**
```bash
./simulator LRU memory.log 8 2
```
- **LRU**: Use Least Recently Used algorithm.
- **memory.log**: Input file with memory references.
- **8 KB**: Page size.
- **2 MB**: Physical memory size.

---

### **Output**
After execution, the program displays:
```
////////////////////////////////////////////

Algorithm: LRU
Input File: memory.log
Page Size: 8 KB
Physical Memory Size: 2 MB
Number of Page Faults: 123
Number of Dirty Pages Written to Disk: 15

////////////////////////////////////////////
```

---

This program provides a comprehensive simulation of virtual memory management, allowing experimentation with multiple algorithms and memory configurations.
