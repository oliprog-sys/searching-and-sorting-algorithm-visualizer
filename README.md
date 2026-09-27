# ALGO VISION — Searching & Sorting Algorithm Visualizer

**ALGO VISION** is a feature-rich, real-time computer science algorithm visualizer implemented in C++ using OpenGL and FreeGLUT. Built as a single translation unit (`main.cpp`, ~2,000 lines), it offers an interactive **Configuration Dashboard**, multi-line unclipped array previewing, full manual/random array input modes, step-by-step state timeline playback, live execution status metrics, and theoretical complexity analysis.

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Key Features](#-key-features)
- [Supported Algorithms & Complexities](#-supported-algorithms--complexities)
- [Application State Machine](#-application-state-machine)
- [Configuration Dashboard & Input Modes](#-configuration-dashboard--input-modes)
- [Typography & Rendering Pipeline](#-typography--rendering-pipeline)
- [Controls & Keyboard Shortcuts](#-controls--keyboard-shortcuts)
- [Button ID Control Registry](#-button-id-control-registry)
- [Color Palette & Visual Indicators](#-color-palette--visual-indicators)
- [Architecture & Implementation Details](#-architecture--implementation-details)
  - [Snapshot Timeline Engine](#snapshot-timeline-engine)
  - [Execution Flow Diagram](#execution-flow-diagram)
- [Building & Running](#-building--running)
  - [Prerequisites](#prerequisites)
  - [MinGW / GCC Compilation](#mingw--gcc-compilation)
  - [MSVC / Visual Studio 2022 Compilation](#msvc--visual-studio-2022-compilation)
  - [Linux Compilation](#linux-compilation)

---

## 🌟 Overview

`main.cpp` encapsulates an interactive suite for demonstrating fundamental searching and sorting algorithms. Rather than running non-deterministic concurrent threads, ALGO VISION pre-calculates state transitions into an offline **snapshot timeline** (`AlgorithmSnapshot`). This architecture enables precise forward/backward step-by-step traversal, variable execution speeds ($0.2\times$ to $10.7\times$), manual array entry, and real-time metric tracking.

---

## ✨ Key Features

- 🚀 **Single Translation Unit**: 100% self-contained standard C++ source file (`main.cpp`) compatible with MSVC 2022 & GCC 15.
- 🎛️ **Modern Configuration Dashboard**: Setup screen allowing users to toggle input modes, type custom arrays, select algorithms, configure playback speeds, and launch visualizations.
- ✍️ **Manual & Random Input Modes**:
  - **Manual Mode**: Direct text box editing with comma-separated values (3–50 integers between 1 and 100) with interactive cursor (`_`).
  - **Random Generator**: Slider-controlled random element generation (3 to 50 items).
- 📜 **Multi-Line Unclipped Array Preview**: Custom auto-wrapping text engine that formats and renders every single array element on the configuration dashboard without clipping or truncation dots.
- 🔤 **Role-Based Typography System**: Centralized font manager mapping semantic roles (`DISPLAY`, `SUBTITLE`, `BODY`, `MONO`) to native FreeGLUT bitmap fonts (`TIMES_ROMAN_24`, `HELVETICA_18`, `HELVETICA_12`, `9_BY_15`).
- 🎬 **Timeline Engine**: Synchronously generates snapshot vectors enabling **Play / Pause / Step > / Step < / Replay / Dynamic Speed Scaling**.
- 📊 **Live Metrics Dock**: Displays active operation type, current step counter, comparison & swap accumulators, range boundary markers (`[low..high]`), step narratives, and Big-O theoretical complexities.
- 🔔 **Contextual Notifications**: Auto-notification banners (e.g., informing users when Binary or Jump Search automatically sorts unsorted data).

---

## 🧠 Supported Algorithms & Complexities

### 🔄 Sorting Algorithms

| Algorithm | Best Time | Average Time | Worst Time | Auxiliary Space | Operational Behavior |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **Bubble Sort** | $\mathcal{O}(n)$ | $\mathcal{O}(n^2)$ | $\mathcal{O}(n^2)$ | $\mathcal{O}(1)$ | Sequentially compares adjacent elements and swaps out-of-order pairs until the largest elements bubble to the end. |
| **Selection Sort** | $\mathcal{O}(n^2)$ | $\mathcal{O}(n^2)$ | $\mathcal{O}(n^2)$ | $\mathcal{O}(1)$ | Scans unsorted partition for the absolute minimum element and swaps it into index $i$. |
| **Insertion Sort** | $\mathcal{O}(n)$ | $\mathcal{O}(n^2)$ | $\mathcal{O}(n^2)$ | $\mathcal{O}(1)$ | Extracts key element and shifts larger elements rightward to insert key into sorted partition position. |
| **Merge Sort** | $\mathcal{O}(n \log n)$ | $\mathcal{O}(n \log n)$ | $\mathcal{O}(n \log n)$ | $\mathcal{O}(n)$ | Recursive divide-and-conquer algorithm splitting array into sub-intervals, sorting, and merging them back. |

### 🔍 Searching Algorithms

| Algorithm | Best Time | Average Time | Worst Time | Space Complexity | Requirements / Operational Behavior |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **Linear Search** | $\mathcal{O}(1)$ | $\mathcal{O}(n)$ | $\mathcal{O}(n)$ | $\mathcal{O}(1)$ | Checks elements sequentially from left to right. Works on unsorted arrays. |
| **Binary Search** | $\mathcal{O}(1)$ | $\mathcal{O}(\log n)$ | $\mathcal{O}(\log n)$ | $\mathcal{O}(1)$ | Halves interval `[low..high]` at mid index. **Requires sorted array** *(auto-sorts input if unsorted)*. |
| **Jump Search** | $\mathcal{O}(1)$ | $\mathcal{O}(\sqrt{n})$ | $\mathcal{O}(\sqrt{n})$ | $\mathcal{O}(1)$ | Steps ahead by fixed block size $m = \lfloor\sqrt{n}\rfloor$ until boundary exceeds target, then performs linear scan inside block. |

---

## 🖥️ Application State Machine

The visualizer operates across three primary screen states managed by `ScreenState`:

```mermaid
stateDiagram-v2
    [*] --> CONFIG_SCREEN : Application Launch
    
    state CONFIG_SCREEN {
        [*] --> SelectInputMode
        SelectInputMode --> ManualEntry : InputMode::MANUAL
        SelectInputMode --> RandomSlider : InputMode::RANDOM
        ManualEntry --> SelectAlgorithm
        RandomSlider --> SelectAlgorithm
        SelectAlgorithm --> Configured
    }

    CONFIG_SCREEN --> SORTING_VIEW : Click START (Category::SORTING)
    CONFIG_SCREEN --> SEARCHING_VIEW : Click START (Category::SEARCHING)

    state SORTING_VIEW {
        [*] --> TimelinePlayback
        TimelinePlayback --> StepInspection : Pause / Step Keys
        StepInspection --> TimelinePlayback : Play Key
    }

    state SEARCHING_VIEW {
        [*] --> GridInspection
        GridInspection --> TargetHit : Target Found
        GridInspection --> TargetMiss : Target Missing
    }

    SORTING_VIEW --> CONFIG_SCREEN : Click '< Back to Config' or Press ESC
    SEARCHING_VIEW --> CONFIG_SCREEN : Click '< Back to Config' or Press ESC
```

---

## 🎛️ Configuration Dashboard & Input Modes

### 1. Manual Input Mode (`InputMode::MANUAL`)
- Click the **Manual Input** text field box to gain focus (`typingInManualInput = true`).
- Type comma-separated integers (e.g. `42, 18, 65, 8, 93, 27, 51`).
- Supported keys inside text box: `0-9`, `,`, `Space`, `Backspace` (deletes characters), `Enter` (parses array).
- Validation automatically filters integers between $1$ and $100$, accepting arrays of length $3 \le n \le 50$.

### 2. Random Generator Mode (`InputMode::RANDOM`)
- Adjust the **Array Size Slider** (3 to 50 elements).
- Click **Generate Array** to instantly build a randomized data set.

### 3. Target Selection (Searching Category)
- Focus the **Target Input Box** to enter a specific integer target.
- Or click **Pick Random Target** to pick a random value present in the array.

---

## 🔤 Typography & Rendering Pipeline

The visualizer uses a role-based typography dispatcher (`getFontForRole`, `getTextWidth`, `drawText`):

| `TextRole` | FreeGLUT Font Handle | Pixel Metrics | Usage Context |
| :--- | :--- | :--- | :--- |
| **`TextRole::DISPLAY`** | `GLUT_BITMAP_TIMES_ROMAN_24` | Variable Height ~24px | Main Dashboard Header ("ALGO VISION") |
| **`TextRole::SUBTITLE`** | `GLUT_BITMAP_HELVETICA_18` | Variable Height ~18px | Section titles, operation card headings, visualizer values |
| **`TextRole::BODY`** | `GLUT_BITMAP_HELVETICA_12` | Variable Height ~12px | Button text, metrics, narratives, legends, complexity labels |
| **`TextRole::MONO`** | `GLUT_BITMAP_9_BY_15` | Fixed 9x15 Monospace | Array input text boxes, multi-line array previews |

---

## ⌨️ Controls & Keyboard Shortcuts

| Input | Scope | Action |
| :---: | :---: | :--- |
| `Spacebar` | Visualizer View | Toggle Play / Pause simulation |
| `Right Arrow` | Visualizer View | Step forward 1 snapshot frame |
| `Left Arrow` | Visualizer View | Step backward 1 snapshot frame |
| `Up Arrow` | All Views | Speed up animation (+ ops/sec) |
| `Down Arrow` | All Views | Slow down animation (- ops/sec) |
| `R` / `r` | Visualizer View | Reset simulation to step 1 (Replay) |
| `Esc` | Visualizer View | Return to Configuration Dashboard |
| `Left Mouse Click` | All Views | Interact with buttons, tabs, sliders, and input boxes |
| `Backspace` / `Delete` | Input Boxes | Delete last character in active text box |
| `Enter` (`Return`) | Input Boxes | Confirm text box input & apply state |

---

## 🔘 Button ID Control Registry

| Button ID | Screen Context | Purpose / Action |
| :---: | :---: | :--- |
| `101` | Config Dashboard | Switch to **Manual Input** mode tab |
| `102` | Config Dashboard | Switch to **Random Generator** mode tab |
| `103` | Config Dashboard | Generate new random array (Random mode) |
| `201` | Config Dashboard | Select **Sorting** algorithm category |
| `202` | Config Dashboard | Select **Searching** algorithm category |
| `301` | Config Dashboard | Select **Bubble Sort** algorithm |
| `302` | Config Dashboard | Select **Selection Sort** algorithm |
| `303` | Config Dashboard | Select **Insertion Sort** algorithm |
| `304` | Config Dashboard | Select **Merge Sort** algorithm |
| `311` | Config Dashboard | Select **Linear Search** algorithm |
| `312` | Config Dashboard | Select **Binary Search** algorithm |
| `313` | Config Dashboard | Select **Jump Search** algorithm |
| `314` | Config Dashboard | Pick random target value from array |
| `400` | Config Dashboard | **START VISUALIZATION** Launch button |
| `10` | Visualizer View | `< Back to Config` navigation button |
| `20`–`23` | Visualizer Header | Quick switch between Sorting algorithms |
| `30`–`32` | Visualizer Header | Quick switch between Searching algorithms |
| `40` | Visualizer Controls | Play / Pause toggle button |
| `41` | Visualizer Controls | Step Forward (`Step >`) |
| `42` | Visualizer Controls | Step Backward (`Step <`) |
| `43` | Visualizer Controls | Replay simulation (`Replay (R)`) |
| `44` | Visualizer Controls | Generate new random array |
| `45` | Visualizer Controls | Speed Down (`Speed - [Down]`) |
| `46` | Visualizer Controls | Speed Up (`Speed + [Up]`) |
| `47` | Visualizer Controls | Decrease array size by 2 |
| `48` | Visualizer Controls | Increase array size by 2 |
| `50` | Searching Controls | Set target to random value inside array |
| `51` | Searching Controls | Set target to missing value (`999`) |

---

## 🎨 Color Palette & Visual Indicators

| Visual Swatch | Color Constant | Color Code | Context & State Usage |
| :--- | :--- | :--- | :--- |
| 🟦 **Default Blue** | `Palette::BAR_DEFAULT` | `#00A6FF` | Idle / standard bar element or searching cell |
| 🟨 **Comparing** | `Palette::COMPARE` | `#FFD100` | Active comparison between elements or checking target |
| 🟥 **Active / Swap** | `Palette::ACTIVE_OP` | `#FF2E38` | Swapping elements, merge overwrites, or target missing |
| 🟩 **Sorted / Found** | `Palette::SUCCESS` | `#00F280` | Finalized sorted element, completed array, or search hit |
| 🟪 **Boundary** | `Palette::BOUNDARY` | `#BF66FF` | Merge sub-range `[left..right]` or search interval `[low..high]` |
| ⬛ **Inactive** | `Palette::INACTIVE` | `#2E3847` | Excluded partition range outside active search window |
| 🩵 **Accent Text** | `Palette::TEXT_ACCENT` | `#00D9FF` | Headers, highlighted metrics, and active borders |

---

## 🏗️ Architecture & Implementation Details

### Snapshot Timeline Engine (`AlgorithmSnapshot`)

Each frame of an algorithm's execution is frozen into an immutable data container:

```cpp
struct AlgorithmSnapshot {
    std::vector<int> arrayState;      // Complete snapshot of array values at step t
    std::vector<bool> finalized;     // Sorted flags per index
    OperationType opType;            // COMPARE, SWAP, OVERWRITE, VISIT, BOUND_UPDATE, FOUND, NOT_FOUND
    int indexA = -1;                 // Primary active element index
    int indexB = -1;                 // Secondary active element index
    int boundL = -1;                 // Left partition boundary marker
    int boundR = -1;                 // Right partition boundary marker
    int target = -1;                 // Search target value
    int comparisons = 0;             // Cumulative comparison counter
    int swaps = 0;                   // Cumulative swap / overwrite counter
    std::string description;         // Natural language narrative text
};
```

When an algorithm is launched, `AlgorithmEngine` executes the algorithm to completion in memory, building a `std::vector<AlgorithmSnapshot>`. The timer callback (`timer`) simply increments `sortIndex` or `searchIndex` based on `getOpsPerSecond()`.

---

## 🛠️ Building & Running

### Prerequisites
- C++17 compatible compiler (MinGW-w64 GCC 15+, MSVC 2022, or Clang).
- OpenGL graphics libraries (`opengl32`, `glu32`).
- FreeGLUT header files and library (`GL/glut.h`, `freeglut`).

---

### MinGW / GCC Compilation (Windows)

```bash
g++ -std=c++17 -O2 main.cpp -lfreeglut -lopengl32 -lglu32 -o AlgoVision.exe
./AlgoVision.exe
```

---

### MSVC / Visual Studio 2022 Compilation

1. Install FreeGLUT via `vcpkg`:
   ```cmd
   vcpkg install freeglut:x64-windows
   ```
2. Compile using Developer Command Prompt:
   ```cmd
   cl /std:c++17 /EHsc /O2 main.cpp /I"%VCPKG_ROOT%\installed\x64-windows\include" /link /LIBPATH:"%VCPKG_ROOT%\installed\x64-windows\lib" freeglut.lib opengl32.lib glu32.lib /SUBSYSTEM:CONSOLE /OUT:AlgoVision.exe
   ```
3. Run `AlgoVision.exe`.

---

### Linux Compilation

Install FreeGLUT development packages:

```bash
sudo apt-get update
sudo apt-get install build-essential freeglut3-dev libglu1-mesa-dev
```

Compile and run:

```bash
g++ -std=c++17 -O2 main.cpp -lglut -lGL -lGLU -o AlgoVision
./AlgoVision
```

---

## 📄 License & Credits

- Visualizer codebase developed in standard modern C++ and OpenGL.
- Renders 2D immediate mode vector primitives with FreeGLUT bitmap fonts (`GLUT_BITMAP_TIMES_ROMAN_24`, `GLUT_BITMAP_HELVETICA_18`, `GLUT_BITMAP_HELVETICA_12`, `GLUT_BITMAP_9_BY_15`).
