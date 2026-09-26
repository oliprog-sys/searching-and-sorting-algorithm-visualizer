/*******************************************************************************
 * ALGO VISION - Searching & Sorting Algorithm Visualizer
 * Single Translation Unit: main.cpp (OpenGL + FreeGLUT)
 * MSVC 2022 & GCC 15 Compatible - 100% Unclipped, Multi-Line Array Preview
 ******************************************************************************/

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>

#include <GL/glut.h>

// ============================================================================
// CONSTANTS & ENUMS
// ============================================================================
enum class ScreenState {
    CONFIG_SCREEN,
    SORTING_VIEW,
    SEARCHING_VIEW
};

enum class InputMode {
    MANUAL,
    RANDOM
};

enum class Category {
    SORTING,
    SEARCHING
};

enum class SortAlgorithm {
    BUBBLE_SORT,
    SELECTION_SORT,
    INSERTION_SORT,
    MERGE_SORT
};

enum class SearchAlgorithm {
    LINEAR_SEARCH,
    BINARY_SEARCH,
    JUMP_SEARCH
};

enum class OperationType {
    IDLE,
    VISIT,
    COMPARE,
    SWAP,
    OVERWRITE,
    BOUND_UPDATE,
    FOUND,
    NOT_FOUND,
    COMPLETED
};

enum class TextRole {
    DISPLAY,
    SUBTITLE,
    BODY,
    MONO
};

struct Color4f {
    float r, g, b, a;
    Color4f() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color4f(float r_, float g_, float b_, float a_ = 1.0f)
        : r(r_), g(g_), b(b_), a(a_) {}
};

namespace Palette {
    const Color4f BG_DARK         = Color4f(0.08f, 0.10f, 0.14f, 1.0f);
    const Color4f PANEL_BG        = Color4f(0.12f, 0.15f, 0.20f, 0.98f);
    const Color4f CARD_BG         = Color4f(0.12f, 0.15f, 0.20f, 0.98f);
    const Color4f CARD_BORDER     = Color4f(0.22f, 0.27f, 0.36f, 0.90f);
    const Color4f CARD_BORDER_ACT = Color4f(0.00f, 0.80f, 1.00f, 1.0f);

    const Color4f BTN_IDLE        = Color4f(0.16f, 0.20f, 0.28f, 1.0f);
    const Color4f BTN_HOVER       = Color4f(0.24f, 0.30f, 0.42f, 1.0f);
    const Color4f BTN_ACTIVE      = Color4f(0.00f, 0.58f, 0.96f, 1.0f);
    const Color4f BTN_LAUNCH      = Color4f(0.00f, 0.75f, 0.45f, 1.0f);
    const Color4f BTN_LAUNCH_HOV  = Color4f(0.00f, 0.90f, 0.55f, 1.0f);
    const Color4f BTN_DANGER      = Color4f(0.90f, 0.22f, 0.25f, 1.0f);
    const Color4f BTN_PLAYING     = Color4f(1.00f, 0.55f, 0.00f, 1.0f);

    const Color4f BAR_DEFAULT     = Color4f(0.00f, 0.65f, 1.00f, 1.0f);
    const Color4f COMPARE         = Color4f(1.00f, 0.82f, 0.00f, 1.0f);
    const Color4f ACTIVE_OP       = Color4f(1.00f, 0.18f, 0.22f, 1.0f);
    const Color4f SUCCESS         = Color4f(0.00f, 0.95f, 0.50f, 1.0f);
    const Color4f INACTIVE        = Color4f(0.18f, 0.22f, 0.28f, 0.4f);
    const Color4f BOUNDARY        = Color4f(0.75f, 0.40f, 1.00f, 1.0f);

    const Color4f TEXT_WHITE      = Color4f(0.96f, 0.98f, 1.00f, 1.0f);
    const Color4f TEXT_MUTED      = Color4f(0.60f, 0.68f, 0.78f, 1.0f);
    const Color4f TEXT_ACCENT     = Color4f(0.00f, 0.85f, 1.00f, 1.0f);
}

// ============================================================================
// DATA STRUCTURES
// ============================================================================
struct AlgorithmSnapshot {
    std::vector<int> arrayState;
    std::vector<bool> finalized;
    OperationType opType = OperationType::IDLE;
    int indexA = -1;
    int indexB = -1;
    int boundL = -1;
    int boundR = -1;
    int target = -1;
    int comparisons = 0;
    int swaps = 0;
    std::string description;
};

struct Button {
    int id = 0;
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    std::string label = "";
    Color4f color;
    bool isActive = false;
    bool isHovered = false;

    bool contains(float px, float py) const {
        return (px >= x && px <= (x + w) && py >= y && py <= (y + h));
    }
};

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
static void renderModernConfigDashboard();
static void renderSortingView();
static void renderSearchingView();
static void renderVisualizerUIOverlay();

// ============================================================================
// 2D RENDERING & TYPOGRAPHY    
// ============================================================================
static void drawQuad(float x, float y, float w, float h, const Color4f& color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void drawQuadOutline(float x, float y, float w, float h, float thickness, const Color4f& color) {
    drawQuad(x, y, w, thickness, color);
    drawQuad(x, y + h - thickness, w, thickness, color);
    drawQuad(x, y, thickness, h, color);
    drawQuad(x + w - thickness, y, thickness, h, color);
}

static void* getFontForRole(TextRole role) {
    switch (role) {
    case TextRole::DISPLAY:  return GLUT_BITMAP_TIMES_ROMAN_24;
    case TextRole::SUBTITLE: return GLUT_BITMAP_HELVETICA_18;
    case TextRole::BODY:     return GLUT_BITMAP_HELVETICA_12;
    case TextRole::MONO:
    default:                 return GLUT_BITMAP_9_BY_15;
    }
}

static int getTextWidth(const std::string& str, TextRole role) {
    void* font = getFontForRole(role);
    int total = 0;
    for (char c : str) total += glutBitmapWidth(font, c);
    return total;
}

static void drawText(float x, float y, const std::string& str, TextRole role, const Color4f& color) {
    void* font = getFontForRole(role);
    glColor4f(color.r, color.g, color.b, color.a);
    glRasterPos2f(x, y);
    for (char c : str) glutBitmapCharacter(font, c);
}

// ============================================================================
// ALGORITHM TIMELINE ENGINE
// ============================================================================
class AlgorithmEngine {
public:
    static std::vector<AlgorithmSnapshot> generateBubbleSort(std::vector<int> arr) {
        std::vector<AlgorithmSnapshot> timeline;
        int comps = 0, swaps = 0;
        int n = static_cast<int>(arr.size());
        std::vector<bool> sortedFlags(static_cast<size_t>(n), false);

        AlgorithmSnapshot initial;
        initial.arrayState = arr;
        initial.finalized = sortedFlags;
        initial.description = "Bubble Sort initialized. Adjacent elements compared and swapped if inverted.";
        timeline.push_back(initial);

        for (int i = 0; i < n - 1; ++i) {
            bool swapped = false;
            for (int j = 0; j < n - i - 1; ++j) {
                comps++;
                AlgorithmSnapshot cmp;
                cmp.arrayState = arr;
                cmp.finalized = sortedFlags;
                cmp.opType = OperationType::COMPARE;
                cmp.indexA = j;
                cmp.indexB = j + 1;
                cmp.comparisons = comps;
                cmp.swaps = swaps;
                cmp.description = "Comparing arr[" + std::to_string(j) + "] (" + std::to_string(arr[j]) +
                                  ") and arr[" + std::to_string(j + 1) + "] (" + std::to_string(arr[j + 1]) + ").";
                timeline.push_back(cmp);

                if (arr[j] > arr[j + 1]) {
                    std::swap(arr[j], arr[j + 1]);
                    swaps++;
                    swapped = true;

                    AlgorithmSnapshot swp;
                    swp.arrayState = arr;
                    swp.finalized = sortedFlags;
                    swp.opType = OperationType::SWAP;
                    swp.indexA = j;
                    swp.indexB = j + 1;
                    swp.comparisons = comps;
                    swp.swaps = swaps;
                    swp.description = "Swapped inverted pair arr[" + std::to_string(j) + "] and arr[" + std::to_string(j + 1) + "].";
                    timeline.push_back(swp);
                }
            }
            sortedFlags[static_cast<size_t>(n - 1 - i)] = true;
            if (!swapped) break;
        }

        std::fill(sortedFlags.begin(), sortedFlags.end(), true);
        AlgorithmSnapshot done;
        done.arrayState = arr;
        done.finalized = sortedFlags;
        done.opType = OperationType::COMPLETED;
        done.comparisons = comps;
        done.swaps = swaps;
        done.description = "Bubble Sort complete. All elements ordered.";
        timeline.push_back(done);

        return timeline;
    }

    static std::vector<AlgorithmSnapshot> generateSelectionSort(std::vector<int> arr) {
        std::vector<AlgorithmSnapshot> timeline;
        int comps = 0, swaps = 0;
        int n = static_cast<int>(arr.size());
        std::vector<bool> sortedFlags(static_cast<size_t>(n), false);

        AlgorithmSnapshot initial;
        initial.arrayState = arr;
        initial.finalized = sortedFlags;
        initial.description = "Selection Sort initialized. Scans for smallest element and places it into index i.";
        timeline.push_back(initial);

        for (int i = 0; i < n - 1; ++i) {
            int minIdx = i;

            AlgorithmSnapshot base;
            base.arrayState = arr;
            base.finalized = sortedFlags;
            base.opType = OperationType::VISIT;
            base.indexA = minIdx;
            base.boundL = i;
            base.comparisons = comps;
            base.swaps = swaps;
            base.description = "Current minimum set to index " + std::to_string(minIdx) + " (" + std::to_string(arr[minIdx]) + ").";
            timeline.push_back(base);

            for (int j = i + 1; j < n; ++j) {
                comps++;
                AlgorithmSnapshot cmp;
                cmp.arrayState = arr;
                cmp.finalized = sortedFlags;
                cmp.opType = OperationType::COMPARE;
                cmp.indexA = minIdx;
                cmp.indexB = j;
                cmp.boundL = i;
                cmp.comparisons = comps;
                cmp.swaps = swaps;
                cmp.description = "Comparing minimum (" + std::to_string(arr[minIdx]) + ") with arr[" + std::to_string(j) + "] (" + std::to_string(arr[j]) + ").";
                timeline.push_back(cmp);

                if (arr[j] < arr[minIdx]) {
                    minIdx = j;
                    AlgorithmSnapshot newMin;
                    newMin.arrayState = arr;
                    newMin.finalized = sortedFlags;
                    newMin.opType = OperationType::VISIT;
                    newMin.indexA = minIdx;
                    newMin.boundL = i;
                    newMin.comparisons = comps;
                    newMin.swaps = swaps;
                    newMin.description = "New minimum found at index " + std::to_string(minIdx) + " (" + std::to_string(arr[minIdx]) + ").";
                    timeline.push_back(newMin);
                }
            }

            if (minIdx != i) {
                std::swap(arr[i], arr[minIdx]);
                swaps++;
                AlgorithmSnapshot swp;
                swp.arrayState = arr;
                swp.finalized = sortedFlags;
                swp.opType = OperationType::SWAP;
                swp.indexA = i;
                swp.indexB = minIdx;
                swp.boundL = i;
                swp.comparisons = comps;
                swp.swaps = swaps;
                swp.description = "Swapped index " + std::to_string(i) + " with minimum index " + std::to_string(minIdx) + ".";
                timeline.push_back(swp);
            }
            sortedFlags[static_cast<size_t>(i)] = true;
        }

        std::fill(sortedFlags.begin(), sortedFlags.end(), true);
        AlgorithmSnapshot done;
        done.arrayState = arr;
        done.finalized = sortedFlags;
        done.opType = OperationType::COMPLETED;
        done.comparisons = comps;
        done.swaps = swaps;
        done.description = "Selection Sort complete. Array fully sorted.";
        timeline.push_back(done);

        return timeline;
    }

    static std::vector<AlgorithmSnapshot> generateInsertionSort(std::vector<int> arr) {
        std::vector<AlgorithmSnapshot> timeline;
        int comps = 0, shifts = 0;
        int n = static_cast<int>(arr.size());
        std::vector<bool> sortedFlags(static_cast<size_t>(n), false);
        sortedFlags[0] = true;

        AlgorithmSnapshot initial;
        initial.arrayState = arr;
        initial.finalized = sortedFlags;
        initial.description = "Insertion Sort initialized. Shifts larger items to insert key in sorted partition.";
        timeline.push_back(initial);

        for (int i = 1; i < n; ++i) {
            int key = arr[i];
            int j = i - 1;

            AlgorithmSnapshot pick;
            pick.arrayState = arr;
            pick.finalized = sortedFlags;
            pick.opType = OperationType::VISIT;
            pick.indexA = i;
            pick.boundL = i;
            pick.comparisons = comps;
            pick.swaps = shifts;
            pick.description = "Extracted key " + std::to_string(key) + " at index " + std::to_string(i) + ". Searching insertion point.";
            timeline.push_back(pick);

            while (j >= 0) {
                comps++;
                AlgorithmSnapshot cmp;
                cmp.arrayState = arr;
                cmp.finalized = sortedFlags;
                cmp.opType = OperationType::COMPARE;
                cmp.indexA = j;
                cmp.indexB = j + 1;
                cmp.boundL = i;
                cmp.comparisons = comps;
                cmp.swaps = shifts;
                cmp.description = "Comparing key " + std::to_string(key) + " with arr[" + std::to_string(j) + "] (" + std::to_string(arr[j]) + ").";
                timeline.push_back(cmp);

                if (arr[j] > key) {
                    arr[j + 1] = arr[j];
                    shifts++;
                    AlgorithmSnapshot shf;
                    shf.arrayState = arr;
                    shf.finalized = sortedFlags;
                    shf.opType = OperationType::OVERWRITE;
                    shf.indexA = j + 1;
                    shf.indexB = j;
                    shf.boundL = i;
                    shf.comparisons = comps;
                    shf.swaps = shifts;
                    shf.description = "Shifted arr[" + std::to_string(j) + "] (" + std::to_string(arr[j]) + ") into position " + std::to_string(j + 1) + ".";
                    timeline.push_back(shf);
                    j--;
                } else {
                    break;
                }
            }

            arr[j + 1] = key;
            for (int k = 0; k <= i; ++k) sortedFlags[static_cast<size_t>(k)] = true;

            AlgorithmSnapshot placed;
            placed.arrayState = arr;
            placed.finalized = sortedFlags;
            placed.opType = OperationType::SWAP;
            placed.indexA = j + 1;
            placed.boundL = i;
            placed.comparisons = comps;
            placed.swaps = shifts;
            placed.description = "Inserted key " + std::to_string(key) + " at sorted partition index " + std::to_string(j + 1) + ".";
            timeline.push_back(placed);
        }

        std::fill(sortedFlags.begin(), sortedFlags.end(), true);
        AlgorithmSnapshot done;
        done.arrayState = arr;
        done.finalized = sortedFlags;
        done.opType = OperationType::COMPLETED;
        done.comparisons = comps;
        done.swaps = shifts;
        done.description = "Insertion Sort complete. All partitions ordered.";
        timeline.push_back(done);

        return timeline;
    }

    static std::vector<AlgorithmSnapshot> generateMergeSort(std::vector<int> arr) {
        std::vector<AlgorithmSnapshot> timeline;
        int comps = 0, overwrites = 0;
        int n = static_cast<int>(arr.size());
        std::vector<bool> sortedFlags(static_cast<size_t>(n), false);

        AlgorithmSnapshot initial;
        initial.arrayState = arr;
        initial.finalized = sortedFlags;
        initial.description = "Merge Sort initialized. Divides array into halves, recursively sorts, and merges.";
        timeline.push_back(initial);

        auto merge = [&](auto& self, int left, int mid, int right) -> void {
            std::vector<int> leftSub(arr.begin() + left, arr.begin() + mid + 1);
            std::vector<int> rightSub(arr.begin() + mid + 1, arr.begin() + right + 1);

            size_t i = 0, j = 0;
            int k = left;

            AlgorithmSnapshot splitSnap;
            splitSnap.arrayState = arr;
            splitSnap.finalized = sortedFlags;
            splitSnap.opType = OperationType::BOUND_UPDATE;
            splitSnap.boundL = left;
            splitSnap.boundR = right;
            splitSnap.comparisons = comps;
            splitSnap.swaps = overwrites;
            splitSnap.description = "Merging subarrays [" + std::to_string(left) + ".." + std::to_string(mid) +
                                    "] and [" + std::to_string(mid + 1) + ".." + std::to_string(right) + "].";
            timeline.push_back(splitSnap);

            while (i < leftSub.size() && j < rightSub.size()) {
                comps++;
                AlgorithmSnapshot cmp;
                cmp.arrayState = arr;
                cmp.finalized = sortedFlags;
                cmp.opType = OperationType::COMPARE;
                cmp.indexA = left + static_cast<int>(i);
                cmp.indexB = mid + 1 + static_cast<int>(j);
                cmp.boundL = left;
                cmp.boundR = right;
                cmp.comparisons = comps;
                cmp.swaps = overwrites;
                cmp.description = "Comparing left (" + std::to_string(leftSub[i]) + ") vs right (" + std::to_string(rightSub[j]) + ").";
                timeline.push_back(cmp);

                overwrites++;
                if (leftSub[i] <= rightSub[j]) {
                    arr[k] = leftSub[i];
                    i++;
                } else {
                    arr[k] = rightSub[j];
                    j++;
                }

                AlgorithmSnapshot wrt;
                wrt.arrayState = arr;
                wrt.finalized = sortedFlags;
                wrt.opType = OperationType::OVERWRITE;
                wrt.indexA = k;
                wrt.boundL = left;
                wrt.boundR = right;
                wrt.comparisons = comps;
                wrt.swaps = overwrites;
                wrt.description = "Merged item into arr[" + std::to_string(k) + "] (" + std::to_string(arr[k]) + ").";
                timeline.push_back(wrt);
                k++;
            }

            while (i < leftSub.size()) {
                overwrites++;
                arr[k] = leftSub[i];
                AlgorithmSnapshot wrt;
                wrt.arrayState = arr;
                wrt.finalized = sortedFlags;
                wrt.opType = OperationType::OVERWRITE;
                wrt.indexA = k;
                wrt.boundL = left;
                wrt.boundR = right;
                wrt.comparisons = comps;
                wrt.swaps = overwrites;
                wrt.description = "Flushing remaining left subarray item into arr[" + std::to_string(k) + "].";
                timeline.push_back(wrt);
                i++;
                k++;
            }

            while (j < rightSub.size()) {
                overwrites++;
                arr[k] = rightSub[j];
                AlgorithmSnapshot wrt;
                wrt.arrayState = arr;
                wrt.finalized = sortedFlags;
                wrt.opType = OperationType::OVERWRITE;
                wrt.indexA = k;
                wrt.boundL = left;
                wrt.boundR = right;
                wrt.comparisons = comps;
                wrt.swaps = overwrites;
                wrt.description = "Flushing remaining right subarray item into arr[" + std::to_string(k) + "].";
                timeline.push_back(wrt);
                j++;
                k++;
            }

            if (left == 0 && right == n - 1) {
                std::fill(sortedFlags.begin(), sortedFlags.end(), true);
            }
        };

        auto mergeSortInternal = [&](auto& self, int left, int right) -> void {
            if (left >= right) return;
            int mid = left + (right - left) / 2;
            self(self, left, mid);
            self(self, mid + 1, right);
            merge(merge, left, mid, right);
        };

        mergeSortInternal(mergeSortInternal, 0, n - 1);

        std::fill(sortedFlags.begin(), sortedFlags.end(), true);
        AlgorithmSnapshot done;
        done.arrayState = arr;
        done.finalized = sortedFlags;
        done.opType = OperationType::COMPLETED;
        done.comparisons = comps;
        done.swaps = overwrites;
        done.description = "Merge Sort complete. All subproblems unified and ordered.";
        timeline.push_back(done);

        return timeline;
    }

    static std::vector<AlgorithmSnapshot> generateLinearSearch(const std::vector<int>& arr, int target) {
        std::vector<AlgorithmSnapshot> timeline;
        int comps = 0;
        int n = static_cast<int>(arr.size());
        std::vector<bool> inspected(static_cast<size_t>(n), false);

        AlgorithmSnapshot initial;
        initial.arrayState = arr;
        initial.finalized = inspected;
        initial.target = target;
        initial.description = "Linear Search initialized. Checking each cell sequentially for target: " + std::to_string(target);
        timeline.push_back(initial);

        bool found = false;
        for (int i = 0; i < n; ++i) {
            comps++;
            inspected[static_cast<size_t>(i)] = true;

            AlgorithmSnapshot visit;
            visit.arrayState = arr;
            visit.finalized = inspected;
            visit.opType = OperationType::COMPARE;
            visit.indexA = i;
            visit.target = target;
            visit.comparisons = comps;
            visit.description = "Inspecting index " + std::to_string(i) + " (Value: " + std::to_string(arr[i]) + ").";
            timeline.push_back(visit);

            if (arr[i] == target) {
                AlgorithmSnapshot hit;
                hit.arrayState = arr;
                hit.finalized = inspected;
                hit.opType = OperationType::FOUND;
                hit.indexA = i;
                hit.target = target;
                hit.comparisons = comps;
                hit.description = "Target " + std::to_string(target) + " found at index " + std::to_string(i) + "!";
                timeline.push_back(hit);
                found = true;
                break;
            }
        }

        if (!found) {
            AlgorithmSnapshot miss;
            miss.arrayState = arr;
            miss.finalized = inspected;
            miss.opType = OperationType::NOT_FOUND;
            miss.target = target;
            miss.comparisons = comps;
            miss.description = "Array fully searched. Target " + std::to_string(target) + " does not exist.";
            timeline.push_back(miss);
        }

        return timeline;
    }

    static std::vector<AlgorithmSnapshot> generateBinarySearch(const std::vector<int>& arr, int target) {
        std::vector<AlgorithmSnapshot> timeline;
        int comps = 0;
        int low = 0;
        int high = static_cast<int>(arr.size()) - 1;
        std::vector<bool> dummyFlags(arr.size(), false);

        AlgorithmSnapshot initial;
        initial.arrayState = arr;
        initial.finalized = dummyFlags;
        initial.target = target;
        initial.boundL = low;
        initial.boundR = high;
        initial.description = "Binary Search initialized on sorted array. Halving interval at each comparison.";
        timeline.push_back(initial);

        bool found = false;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            comps++;

            AlgorithmSnapshot step;
            step.arrayState = arr;
            step.finalized = dummyFlags;
            step.opType = OperationType::COMPARE;
            step.indexA = mid;
            step.boundL = low;
            step.boundR = high;
            step.target = target;
            step.comparisons = comps;
            step.description = "Checking mid index " + std::to_string(mid) + " (Value: " + std::to_string(arr[mid]) +
                               ") within [" + std::to_string(low) + ".." + std::to_string(high) + "].";
            timeline.push_back(step);

            if (arr[mid] == target) {
                AlgorithmSnapshot hit;
                hit.arrayState = arr;
                hit.finalized = dummyFlags;
                hit.opType = OperationType::FOUND;
                hit.indexA = mid;
                hit.boundL = low;
                hit.boundR = high;
                hit.target = target;
                hit.comparisons = comps;
                hit.description = "Target " + std::to_string(target) + " matched at index " + std::to_string(mid) + "!";
                timeline.push_back(hit);
                found = true;
                break;
            }

            if (arr[mid] < target) {
                low = mid + 1;
                AlgorithmSnapshot upd;
                upd.arrayState = arr;
                upd.finalized = dummyFlags;
                upd.opType = OperationType::BOUND_UPDATE;
                upd.indexA = mid;
                upd.boundL = low;
                upd.boundR = high;
                upd.target = target;
                upd.comparisons = comps;
                upd.description = "arr[" + std::to_string(mid) + "] < target. Discarding left half; search interval is now [" +
                                  std::to_string(low) + ".." + std::to_string(high) + "].";
                timeline.push_back(upd);
            } else {
                high = mid - 1;
                AlgorithmSnapshot upd;
                upd.arrayState = arr;
                upd.finalized = dummyFlags;
                upd.opType = OperationType::BOUND_UPDATE;
                upd.indexA = mid;
                upd.boundL = low;
                upd.boundR = high;
                upd.target = target;
                upd.comparisons = comps;
                upd.description = "arr[" + std::to_string(mid) + "] > target. Discarding right half; search interval is now [" +
                                  std::to_string(low) + ".." + std::to_string(high) + "].";
                timeline.push_back(upd);
            }
        }

        if (!found) {
            AlgorithmSnapshot miss;
            miss.arrayState = arr;
            miss.finalized = dummyFlags;
            miss.opType = OperationType::NOT_FOUND;
            miss.target = target;
            miss.comparisons = comps;
            miss.description = "Binary Search complete. Target " + std::to_string(target) + " not found.";
            timeline.push_back(miss);
        }

        return timeline;
    }

    static std::vector<AlgorithmSnapshot> generateJumpSearch(const std::vector<int>& arr, int target) {
        std::vector<AlgorithmSnapshot> timeline;
        int comps = 0;
        int n = static_cast<int>(arr.size());
        int step = static_cast<int>(std::floor(std::sqrt(static_cast<double>(n))));
        int prev = 0;
        std::vector<bool> dummyFlags(static_cast<size_t>(n), false);

        AlgorithmSnapshot initial;
        initial.arrayState = arr;
        initial.finalized = dummyFlags;
        initial.target = target;
        initial.description = "Jump Search initialized on sorted array. Block step m = sqrt(" + std::to_string(n) + ") = " + std::to_string(step) + ".";
        timeline.push_back(initial);

        int curr = 0;
        while (curr < n) {
            int rightEdge = static_cast<int>((std::min)(static_cast<size_t>(curr) + static_cast<size_t>(step), static_cast<size_t>(n)));
            int checkIdx = rightEdge - 1;

            comps++;
            AlgorithmSnapshot jmp;
            jmp.arrayState = arr;
            jmp.finalized = dummyFlags;
            jmp.opType = OperationType::COMPARE;
            jmp.indexA = checkIdx;
            jmp.boundL = curr;
            jmp.boundR = checkIdx;
            jmp.target = target;
            jmp.comparisons = comps;
            jmp.description = "Checking block boundary arr[" + std::to_string(checkIdx) + "] (" + std::to_string(arr[checkIdx]) + ").";
            timeline.push_back(jmp);

            if (arr[checkIdx] >= target) break;

            curr += step;
        }

        prev = curr;
        int rightBound = static_cast<int>((std::min)(static_cast<size_t>(curr) + static_cast<size_t>(step), static_cast<size_t>(n))) - 1;
        AlgorithmSnapshot blk;
        blk.arrayState = arr;
        blk.finalized = dummyFlags;
        blk.opType = OperationType::BOUND_UPDATE;
        blk.boundL = prev;
        blk.boundR = rightBound;
        blk.target = target;
        blk.comparisons = comps;
        blk.description = "Target is within block [" + std::to_string(prev) + ".." + std::to_string(rightBound) + "]. Scanning linearly.";
        timeline.push_back(blk);

        bool found = false;
        while (prev <= rightBound && prev < n) {
            comps++;
            AlgorithmSnapshot lin;
            lin.arrayState = arr;
            lin.finalized = dummyFlags;
            lin.opType = OperationType::COMPARE;
            lin.indexA = prev;
            lin.boundL = curr;
            lin.boundR = rightBound;
            lin.target = target;
            lin.comparisons = comps;
            lin.description = "Linear scan at index " + std::to_string(prev) + " (Value: " + std::to_string(arr[prev]) + ").";
            timeline.push_back(lin);

            if (arr[prev] == target) {
                AlgorithmSnapshot hit;
                hit.arrayState = arr;
                hit.finalized = dummyFlags;
                hit.opType = OperationType::FOUND;
                hit.indexA = prev;
                hit.target = target;
                hit.comparisons = comps;
                hit.description = "Target " + std::to_string(target) + " found at index " + std::to_string(prev) + "!";
                timeline.push_back(hit);
                found = true;
                break;
            }
            if (arr[prev] > target) break;
            prev++;
        }

        if (!found) {
            AlgorithmSnapshot miss;
            miss.arrayState = arr;
            miss.finalized = dummyFlags;
            miss.opType = OperationType::NOT_FOUND;
            miss.target = target;
            miss.comparisons = comps;
            miss.description = "Target " + std::to_string(target) + " does not exist in array.";
            timeline.push_back(miss);
        }

        return timeline;
    }
};

// ============================================================================
// APPLICATION STATE MACHINE
// ============================================================================
class Application {
public:
    int windowW = 1280;
    int windowH = 720;

    ScreenState screen = ScreenState::CONFIG_SCREEN;
    InputMode inputMode = InputMode::MANUAL;
    Category selectedCategory = Category::SORTING;

    SortAlgorithm currentSortAlg = SortAlgorithm::BUBBLE_SORT;
    std::vector<int> sortOriginal;
    std::vector<AlgorithmSnapshot> sortTimeline;
    size_t sortIndex = 0;
    bool sortPlaying = false;

    SearchAlgorithm currentSearchAlg = SearchAlgorithm::LINEAR_SEARCH;
    std::vector<int> searchOriginal;
    std::vector<AlgorithmSnapshot> searchTimeline;
    size_t searchIndex = 0;
    bool searchPlaying = false;
    int searchTarget = 42;
    std::string searchTargetInputStr = "42";

    int arraySize = 13;
    std::string manualInputStr = "42, 18, 65, 8, 93, 27, 51, 14, 76, 35, 88, 3, 59";
    bool typingInManualInput = false;
    bool typingInTargetInput = false;
    std::string notificationMessage = "";

    const std::vector<int> speedSteps = { 1, 2, 4, 6, 8, 12, 16, 24, 32, 48, 64 };
    size_t speedIndex = 3;

    std::vector<Button> buttons;

    int getOpsPerSecond() const {
        return speedSteps[speedIndex];
    }

    float getSpeedMultiplier() const {
        return static_cast<float>(getOpsPerSecond()) / 6.0f;
    }

    void addBtn(int id, float x, float y, float w, float h, const std::string& label, const Color4f& color = Color4f(), bool active = false) {
        Button b;
        b.id = id;
        b.x = x;
        b.y = y;
        b.w = w;
        b.h = h;
        b.label = label;
        b.color = color;
        b.isActive = active;
        buttons.push_back(b);
    }

    void init() {
        srand(static_cast<unsigned int>(time(nullptr)));
        parseManualInput();
        rebuildButtons();
    }

    void parseManualInput() {
        std::vector<int> parsed;
        std::stringstream ss(manualInputStr);
        std::string token;
        while (std::getline(ss, token, ',')) {
            size_t start = token.find_first_not_of(" \t\r\n");
            size_t end = token.find_last_not_of(" \t\r\n");
            if (start != std::string::npos && end != std::string::npos) {
                token = token.substr(start, end - start + 1);
                try {
                    int val = std::stoi(token);
                    if (val >= 1 && val <= 100) {
                        parsed.push_back(val);
                    }
                } catch (...) {}
            }
        }
        if (parsed.size() >= 3 && parsed.size() <= 50) {
            sortOriginal = parsed;
            searchOriginal = parsed;
            arraySize = static_cast<int>(parsed.size());
            if (!searchOriginal.empty()) {
                searchTarget = searchOriginal[0];
                searchTargetInputStr = std::to_string(searchTarget);
            }
        }
    }

    void generateRandomArray() {
        sortOriginal.clear();
        searchOriginal.clear();
        for (int i = 0; i < arraySize; ++i) {
            int val = 3 + rand() % 97;
            sortOriginal.push_back(val);
            searchOriginal.push_back(val);
        }
        if (!searchOriginal.empty()) {
            searchTarget = searchOriginal[rand() % searchOriginal.size()];
            searchTargetInputStr = std::to_string(searchTarget);
        }
        std::string s = "";
        for (size_t i = 0; i < sortOriginal.size(); ++i) {
            s += std::to_string(sortOriginal[i]);
            if (i + 1 < sortOriginal.size()) s += ", ";
        }
        manualInputStr = s;
    }

    void buildSortTimeline() {
        sortPlaying = true;
        sortIndex = 0;
        notificationMessage = "";
        switch (currentSortAlg) {
        case SortAlgorithm::BUBBLE_SORT:
            sortTimeline = AlgorithmEngine::generateBubbleSort(sortOriginal);
            break;
        case SortAlgorithm::SELECTION_SORT:
            sortTimeline = AlgorithmEngine::generateSelectionSort(sortOriginal);
            break;
        case SortAlgorithm::INSERTION_SORT:
            sortTimeline = AlgorithmEngine::generateInsertionSort(sortOriginal);
            break;
        case SortAlgorithm::MERGE_SORT:
            sortTimeline = AlgorithmEngine::generateMergeSort(sortOriginal);
            break;
        }
    }

    void buildSearchTimeline() {
        searchPlaying = true;
        searchIndex = 0;
        notificationMessage = "";

        if (currentSearchAlg == SearchAlgorithm::BINARY_SEARCH || currentSearchAlg == SearchAlgorithm::JUMP_SEARCH) {
            if (!std::is_sorted(searchOriginal.begin(), searchOriginal.end())) {
                std::sort(searchOriginal.begin(), searchOriginal.end());
                notificationMessage = "Notice: Binary & Jump Search require sorted data. Array automatically sorted!";
            }
        }

        switch (currentSearchAlg) {
        case SearchAlgorithm::LINEAR_SEARCH:
            searchTimeline = AlgorithmEngine::generateLinearSearch(searchOriginal, searchTarget);
            break;
        case SearchAlgorithm::BINARY_SEARCH:
            searchTimeline = AlgorithmEngine::generateBinarySearch(searchOriginal, searchTarget);
            break;
        case SearchAlgorithm::JUMP_SEARCH:
            searchTimeline = AlgorithmEngine::generateJumpSearch(searchOriginal, searchTarget);
            break;
        }
    }

    void rebuildButtons() {
        buttons.clear();

        if (screen == ScreenState::CONFIG_SCREEN) {
            // Section 1: Array Input Mode Tabs
            addBtn(101, 55.0f, 175.0f, 155.0f, 44.0f, "Manual Input", Palette::BTN_IDLE, inputMode == InputMode::MANUAL);
            addBtn(102, 225.0f, 175.0f, 175.0f, 44.0f, "Random Generator", Palette::BTN_IDLE, inputMode == InputMode::RANDOM);

            if (inputMode == InputMode::RANDOM) {
                addBtn(103, 400.0f, 240.0f, 160.0f, 42.0f, "Generate Array", Palette::BTN_IDLE);
            }

            // Section 2: Category Tabs
            float card2X = 665.0f;
            addBtn(201, card2X + 20.0f, 175.0f, 150.0f, 44.0f, "Sorting", Palette::BTN_IDLE, selectedCategory == Category::SORTING);
            addBtn(202, card2X + 185.0f, 175.0f, 150.0f, 44.0f, "Searching", Palette::BTN_IDLE, selectedCategory == Category::SEARCHING);

            if (selectedCategory == Category::SORTING) {
                // 2x2 Clean Responsive Grid to Prevent Overflow
                float btnW = 160.0f;
                float btnH = 42.0f;
                float col1X = card2X + 20.0f;
                float col2X = card2X + 195.0f;
                float row1Y = 270.0f;
                float row2Y = 325.0f;

                addBtn(301, col1X, row1Y, btnW, btnH, "Bubble Sort", Palette::BTN_IDLE, currentSortAlg == SortAlgorithm::BUBBLE_SORT);
                addBtn(302, col2X, row1Y, btnW, btnH, "Selection Sort", Palette::BTN_IDLE, currentSortAlg == SortAlgorithm::SELECTION_SORT);
                addBtn(303, col1X, row2Y, btnW, btnH, "Insertion Sort", Palette::BTN_IDLE, currentSortAlg == SortAlgorithm::INSERTION_SORT);
                addBtn(304, col2X, row2Y, btnW, btnH, "Merge Sort", Palette::BTN_IDLE, currentSortAlg == SortAlgorithm::MERGE_SORT);
            } else {
                float btnW = 145.0f;
                float btnH = 42.0f;
                float col1X = card2X + 20.0f;
                float col2X = card2X + 180.0f;
                float col3X = card2X + 340.0f;

                addBtn(311, col1X, 270.0f, btnW, btnH, "Linear Search", Palette::BTN_IDLE, currentSearchAlg == SearchAlgorithm::LINEAR_SEARCH);
                addBtn(312, col2X, 270.0f, btnW, btnH, "Binary Search", Palette::BTN_IDLE, currentSearchAlg == SearchAlgorithm::BINARY_SEARCH);
                addBtn(313, col3X, 270.0f, btnW, btnH, "Jump Search", Palette::BTN_IDLE, currentSearchAlg == SearchAlgorithm::JUMP_SEARCH);

                addBtn(314, card2X + 160.0f, 385.0f, 230.0f, 42.0f, "Pick Random Target", Palette::BTN_IDLE);
            }

            // Launch Action Button
            addBtn(400, 55.0f, static_cast<float>(windowH) - 105.0f, 280.0f, 54.0f, "START VISUALIZATION", Palette::BTN_LAUNCH);
        } else {
            addBtn(10, 25.0f, 20.0f, 145.0f, 40.0f, "< Back to Config", Palette::CARD_BG);

            float tabX = 185.0f;
            if (screen == ScreenState::SORTING_VIEW) {
                Color4f c1 = (currentSortAlg == SortAlgorithm::BUBBLE_SORT) ? Palette::BTN_ACTIVE : Palette::CARD_BG;
                Color4f c2 = (currentSortAlg == SortAlgorithm::SELECTION_SORT) ? Palette::BTN_ACTIVE : Palette::CARD_BG;
                Color4f c3 = (currentSortAlg == SortAlgorithm::INSERTION_SORT) ? Palette::BTN_ACTIVE : Palette::CARD_BG;
                Color4f c4 = (currentSortAlg == SortAlgorithm::MERGE_SORT) ? Palette::BTN_ACTIVE : Palette::CARD_BG;

                addBtn(20, tabX, 20.0f, 125.0f, 40.0f, "Bubble Sort", c1);
                addBtn(21, tabX + 135.0f, 20.0f, 130.0f, 40.0f, "Selection Sort", c2);
                addBtn(22, tabX + 275.0f, 20.0f, 130.0f, 40.0f, "Insertion Sort", c3);
                addBtn(23, tabX + 415.0f, 20.0f, 125.0f, 40.0f, "Merge Sort", c4);
            } else {
                Color4f c1 = (currentSearchAlg == SearchAlgorithm::LINEAR_SEARCH) ? Palette::BTN_ACTIVE : Palette::CARD_BG;
                Color4f c2 = (currentSearchAlg == SearchAlgorithm::BINARY_SEARCH) ? Palette::BTN_ACTIVE : Palette::CARD_BG;
                Color4f c3 = (currentSearchAlg == SearchAlgorithm::JUMP_SEARCH) ? Palette::BTN_ACTIVE : Palette::CARD_BG;

                addBtn(30, tabX, 20.0f, 140.0f, 40.0f, "Linear Search", c1);
                addBtn(31, tabX + 150.0f, 20.0f, 140.0f, 40.0f, "Binary Search", c2);
                addBtn(32, tabX + 300.0f, 20.0f, 140.0f, 40.0f, "Jump Search", c3);
            }

            float rx = static_cast<float>(windowW) - 340.0f;
            float ry = 75.0f;

            bool isPlay = (screen == ScreenState::SORTING_VIEW) ? sortPlaying : searchPlaying;
            Color4f playCol = isPlay ? Palette::BTN_PLAYING : Palette::SUCCESS;
            std::string playLbl = isPlay ? "PAUSE (Space)" : "PLAY (Space)";

            addBtn(40, rx, ry, 130.0f, 40.0f, playLbl, playCol);
            addBtn(41, rx + 140.0f, ry, 85.0f, 40.0f, "Step >", Palette::BTN_IDLE);
            addBtn(42, rx + 235.0f, ry, 85.0f, 40.0f, "Step <", Palette::BTN_IDLE);

            addBtn(43, rx, ry + 50.0f, 150.0f, 34.0f, "Replay (R)", Palette::BTN_IDLE);
            addBtn(44, rx + 160.0f, ry + 50.0f, 160.0f, 34.0f, "New Random Array", Palette::BTN_IDLE);

            addBtn(45, rx, ry + 92.0f, 150.0f, 32.0f, "Speed - [Down]", Palette::CARD_BG);
            addBtn(46, rx + 160.0f, ry + 92.0f, 160.0f, 32.0f, "Speed + [Up]", Palette::CARD_BG);

            addBtn(47, rx, ry + 132.0f, 150.0f, 32.0f, "Array Size -", Palette::CARD_BG);
            addBtn(48, rx + 160.0f, ry + 132.0f, 160.0f, 32.0f, "Array Size +", Palette::CARD_BG);

            if (screen == ScreenState::SEARCHING_VIEW) {
                addBtn(50, rx, ry + 172.0f, 150.0f, 32.0f, "Target: In Array", Palette::BTN_IDLE);
                addBtn(51, rx + 160.0f, ry + 172.0f, 160.0f, 32.0f, "Target: Missing", Palette::BTN_DANGER);
            }
        }
    }
};

static Application app;

// ============================================================================
// VISUALIZER RENDERING IMPLEMENTATION
// ============================================================================
static void renderSortingView() {
    if (app.sortTimeline.empty()) return;
    const AlgorithmSnapshot& snap = app.sortTimeline[app.sortIndex];
    const std::vector<int>& arr = snap.arrayState;
    int n = static_cast<int>(arr.size());

    float leftMargin = 40.0f;
    float rightMargin = 360.0f;
    float topMargin = 85.0f;
    float bottomMargin = 160.0f;

    float renderW = static_cast<float>(app.windowW) - leftMargin - rightMargin;
    float renderH = static_cast<float>(app.windowH) - topMargin - bottomMargin;
    float baselineY = topMargin + renderH;

    drawQuad(leftMargin, baselineY + 2.0f, renderW, 2.0f, Palette::CARD_BORDER);

    float slotW = renderW / static_cast<float>(n);
    float barW = slotW * 0.78f;
    float gap = slotW * 0.22f;

    int maxVal = 1;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] > maxVal) maxVal = arr[i];
    }

    for (int i = 0; i < n; ++i) {
        float x = leftMargin + static_cast<float>(i) * slotW + (gap * 0.5f);
        float h = (static_cast<float>(arr[i]) / static_cast<float>(maxVal)) * (renderH - 40.0f);
        float y = baselineY - h;

        Color4f barColor = Palette::BAR_DEFAULT;

        if (snap.opType == OperationType::COMPLETED || (i < static_cast<int>(snap.finalized.size()) && snap.finalized[static_cast<size_t>(i)])) {
            barColor = Palette::SUCCESS;
        } else if (i == snap.indexA || i == snap.indexB) {
            if (snap.opType == OperationType::COMPARE) {
                barColor = Palette::COMPARE;
            } else if (snap.opType == OperationType::SWAP || snap.opType == OperationType::OVERWRITE) {
                barColor = Palette::ACTIVE_OP;
            } else if (snap.opType == OperationType::VISIT) {
                barColor = Palette::ACTIVE_OP;
            }
        } else if (snap.boundL >= 0 && snap.boundR >= 0) {
            if (i >= snap.boundL && i <= snap.boundR) {
                barColor = Palette::BOUNDARY;
            } else {
                barColor = Palette::INACTIVE;
            }
        }

        drawQuad(x, y, barW, h, barColor);
        drawQuad(x, y, barW, 4.0f, Color4f(1.0f, 1.0f, 1.0f, 0.6f));

        if (barW >= 16.0f) {
            std::string valStr = std::to_string(arr[i]);
            int sw = getTextWidth(valStr, TextRole::MONO);
            drawText(x + (barW - static_cast<float>(sw)) * 0.5f, baselineY + 18.0f, valStr, TextRole::MONO, Palette::TEXT_WHITE);
        }
    }
}

static void renderSearchingView() {
    if (app.searchTimeline.empty()) return;
    const AlgorithmSnapshot& snap = app.searchTimeline[app.searchIndex];
    const std::vector<int>& arr = snap.arrayState;
    int n = static_cast<int>(arr.size());

    float leftMargin = 40.0f;
    float rightMargin = 360.0f;
    float topMargin = 100.0f;
    float bottomMargin = 160.0f;

    float renderW = static_cast<float>(app.windowW) - leftMargin - rightMargin;
    float centerY = topMargin + (static_cast<float>(app.windowH) - topMargin - bottomMargin) * 0.40f;

    float cellSpacing = 8.0f;
    float cellW = (renderW - static_cast<float>(n - 1) * cellSpacing) / static_cast<float>(n);
    if (cellW > 62.0f) cellW = 62.0f;
    if (cellW < 14.0f) cellW = 14.0f;
    float cellH = 62.0f;

    float actualTotalW = static_cast<float>(n) * cellW + static_cast<float>(n - 1) * cellSpacing;
    float startX = leftMargin + (std::max)(0.0f, (renderW - actualTotalW) * 0.5f);

    for (int i = 0; i < n; ++i) {
        float x = startX + static_cast<float>(i) * (cellW + cellSpacing);
        float y = centerY - cellH * 0.5f;

        Color4f cellCol = Palette::CARD_BG;

        if (snap.boundL >= 0 && snap.boundR >= 0) {
            if (i >= snap.boundL && i <= snap.boundR) {
                cellCol = Palette::CARD_BG;
            } else {
                cellCol = Palette::INACTIVE;
            }
        }

        if (i == snap.indexA) {
            if (snap.opType == OperationType::COMPARE) {
                cellCol = Palette::COMPARE;
            } else if (snap.opType == OperationType::FOUND) {
                cellCol = Palette::SUCCESS;
            }
        }

        if (snap.opType == OperationType::NOT_FOUND) {
            cellCol = Palette::ACTIVE_OP;
        }

        drawQuad(x, y, cellW, cellH, cellCol);
        drawQuadOutline(x, y, cellW, cellH, 1.5f, Palette::CARD_BORDER);

        if (i == snap.boundL) {
            drawQuad(x, y - 8.0f, cellW, 4.0f, Palette::BOUNDARY);
        }
        if (i == snap.boundR) {
            drawQuad(x, y + cellH + 4.0f, cellW, 4.0f, Palette::BOUNDARY);
        }

        if (cellW >= 16.0f) {
            std::string vStr = std::to_string(arr[i]);
            int sw = getTextWidth(vStr, TextRole::SUBTITLE);
            Color4f tc = (cellCol.r > 0.8f && cellCol.g > 0.7f) ? Palette::BG_DARK : Palette::TEXT_WHITE;
            drawText(x + (cellW - static_cast<float>(sw)) * 0.5f, y + cellH * 0.62f, vStr, TextRole::SUBTITLE, tc);

            std::string idxStr = std::to_string(i);
            int isw = getTextWidth(idxStr, TextRole::BODY);
            drawText(x + (cellW - static_cast<float>(isw)) * 0.5f, y + cellH + 18.0f, idxStr, TextRole::BODY, Palette::TEXT_MUTED);
        }
    }
}

static void renderVisualizerUIOverlay() {
    for (size_t i = 0; i < app.buttons.size(); ++i) {
        const Button& b = app.buttons[i];
        Color4f c = b.isHovered ? Palette::BTN_HOVER : b.color;
        drawQuad(b.x, b.y, b.w, b.h, c);
        drawQuadOutline(b.x, b.y, b.w, b.h, 1.5f, Palette::CARD_BORDER);

        int strW = getTextWidth(b.label, TextRole::BODY);
        drawText(b.x + (b.w - static_cast<float>(strW)) * 0.5f, b.y + b.h * 0.62f, b.label, TextRole::BODY, Palette::TEXT_WHITE);
    }

    if (!app.notificationMessage.empty()) {
        float notifW = 620.0f;
        float notifX = 185.0f;
        drawQuad(notifX, 68.0f, notifW, 26.0f, Palette::BOUNDARY);
        drawText(notifX + 14.0f, 85.0f, app.notificationMessage, TextRole::BODY, Palette::TEXT_WHITE);
    }

    float dockX = static_cast<float>(app.windowW) - 345.0f;
    float dockY = 70.0f;
    float dockW = 330.0f;
    float dockH = static_cast<float>(app.windowH) - 85.0f;

    drawQuad(dockX, dockY, dockW, dockH, Palette::PANEL_BG);
    drawQuadOutline(dockX, dockY, dockW, dockH, 1.5f, Palette::CARD_BORDER);

    bool isSorting = (app.screen == ScreenState::SORTING_VIEW);
    size_t curStep = isSorting ? app.sortIndex : app.searchIndex;
    size_t totalSteps = isSorting ? app.sortTimeline.size() : app.searchTimeline.size();

    float textY = dockY + 225.0f;
    drawText(dockX + 18.0f, textY, "LIVE METRICS & STATUS", TextRole::SUBTITLE, Palette::TEXT_ACCENT);
    textY += 28.0f;

    std::string stepStr = "Step: " + std::to_string(curStep + 1) + " / " + std::to_string(totalSteps);
    drawText(dockX + 18.0f, textY, stepStr, TextRole::BODY, Palette::TEXT_WHITE);
    textY += 22.0f;

    std::ostringstream spdSs;
    spdSs.precision(1);
    spdSs << "Speed: " << std::fixed << app.getSpeedMultiplier() << "x (" << app.getOpsPerSecond() << " ops/sec)";
    drawText(dockX + 18.0f, textY, spdSs.str(), TextRole::BODY, Palette::TEXT_MUTED);
    textY += 26.0f;

    if (totalSteps > 0 && curStep < totalSteps) {
        const AlgorithmSnapshot& snap = isSorting ? app.sortTimeline[curStep] : app.searchTimeline[curStep];

        float cardH = 50.0f;
        Color4f statusColor = Palette::BAR_DEFAULT;
        std::string statusTitle = "IDLE / RUNNING";

        if (snap.opType == OperationType::COMPARE) {
            statusColor = Palette::COMPARE;
            statusTitle = "COMPARING";
        } else if (snap.opType == OperationType::SWAP) {
            statusColor = Palette::ACTIVE_OP;
            statusTitle = "SWAPPING";
        } else if (snap.opType == OperationType::OVERWRITE) {
            statusColor = Palette::ACTIVE_OP;
            statusTitle = "OVERWRITING / MERGING";
        } else if (snap.opType == OperationType::VISIT) {
            statusColor = Palette::COMPARE;
            statusTitle = "INSPECTING";
        } else if (snap.opType == OperationType::BOUND_UPDATE) {
            statusColor = Palette::BOUNDARY;
            statusTitle = "PARTITIONING";
        } else if (snap.opType == OperationType::COMPLETED) {
            statusColor = Palette::SUCCESS;
            statusTitle = "SORTING COMPLETE";
        } else if (snap.opType == OperationType::FOUND) {
            statusColor = Palette::SUCCESS;
            statusTitle = "TARGET FOUND";
        } else if (snap.opType == OperationType::NOT_FOUND) {
            statusColor = Palette::ACTIVE_OP;
            statusTitle = "TARGET NOT FOUND";
        }

        drawQuad(dockX + 16.0f, textY, dockW - 32.0f, cardH, Palette::CARD_BG);
        drawQuadOutline(dockX + 16.0f, textY, dockW - 32.0f, cardH, 2.0f, statusColor);
        drawText(dockX + 26.0f, textY + 20.0f, "CURRENT OPERATION", TextRole::BODY, Palette::TEXT_MUTED);
        drawText(dockX + 26.0f, textY + 40.0f, statusTitle, TextRole::SUBTITLE, statusColor);
        textY += cardH + 18.0f;

        std::string cmpStr = "Comparisons: " + std::to_string(snap.comparisons);
        drawText(dockX + 18.0f, textY, cmpStr, TextRole::BODY, Palette::COMPARE);
        textY += 20.0f;

        if (isSorting) {
            std::string swpStr = "Swaps / Overwrites: " + std::to_string(snap.swaps);
            drawText(dockX + 18.0f, textY, swpStr, TextRole::BODY, Palette::ACTIVE_OP);
            textY += 20.0f;
        } else {
            std::string tgtStr = "Target: " + std::to_string(snap.target) + " | Range: [" + std::to_string(snap.boundL) + ".." + std::to_string(snap.boundR) + "]";
            drawText(dockX + 18.0f, textY, tgtStr, TextRole::BODY, Palette::BOUNDARY);
            textY += 20.0f;
        }

        textY += 6.0f;
        drawText(dockX + 18.0f, textY, "Narrative:", TextRole::BODY, Palette::TEXT_WHITE);
        textY += 16.0f;

        std::string desc = snap.description;
        size_t lineMax = 38;
        for (size_t i = 0; i < desc.length(); i += lineMax) {
            std::string line = desc.substr(i, lineMax);
            drawText(dockX + 18.0f, textY, line, TextRole::BODY, Palette::TEXT_MUTED);
            textY += 15.0f;
        }
    }

    // Complexity Card
    textY = dockY + dockH - 90.0f;
    drawQuad(dockX + 12.0f, textY, dockW - 24.0f, 80.0f, Palette::CARD_BG);
    drawQuadOutline(dockX + 12.0f, textY, dockW - 24.0f, 80.0f, 1.0f, Palette::CARD_BORDER);
    textY += 22.0f;

    if (isSorting) {
        if (app.currentSortAlg == SortAlgorithm::BUBBLE_SORT) {
            drawText(dockX + 22.0f, textY, "Bubble Sort Complexity", TextRole::BODY, Palette::COMPARE);
            drawText(dockX + 22.0f, textY + 20.0f, "Time: Best O(n), Avg/Worst O(n^2)", TextRole::BODY, Palette::TEXT_WHITE);
            drawText(dockX + 22.0f, textY + 38.0f, "Space: O(1) Auxiliary", TextRole::BODY, Palette::TEXT_MUTED);
        } else if (app.currentSortAlg == SortAlgorithm::SELECTION_SORT) {
            drawText(dockX + 22.0f, textY, "Selection Sort Complexity", TextRole::BODY, Palette::COMPARE);
            drawText(dockX + 22.0f, textY + 20.0f, "Time: Best/Avg/Worst O(n^2)", TextRole::BODY, Palette::TEXT_WHITE);
            drawText(dockX + 22.0f, textY + 38.0f, "Space: O(1) Auxiliary", TextRole::BODY, Palette::TEXT_MUTED);
        } else if (app.currentSortAlg == SortAlgorithm::INSERTION_SORT) {
            drawText(dockX + 22.0f, textY, "Insertion Sort Complexity", TextRole::BODY, Palette::COMPARE);
            drawText(dockX + 22.0f, textY + 20.0f, "Time: Best O(n), Avg/Worst O(n^2)", TextRole::BODY, Palette::TEXT_WHITE);
            drawText(dockX + 22.0f, textY + 38.0f, "Space: O(1) Auxiliary", TextRole::BODY, Palette::TEXT_MUTED);
        } else if (app.currentSortAlg == SortAlgorithm::MERGE_SORT) {
            drawText(dockX + 22.0f, textY, "Merge Sort Complexity", TextRole::BODY, Palette::COMPARE);
            drawText(dockX + 22.0f, textY + 20.0f, "Time: Best/Avg/Worst O(n log n)", TextRole::BODY, Palette::TEXT_WHITE);
            drawText(dockX + 22.0f, textY + 38.0f, "Space: O(n) Auxiliary", TextRole::BODY, Palette::TEXT_MUTED);
        }
    } else {
        if (app.currentSearchAlg == SearchAlgorithm::LINEAR_SEARCH) {
            drawText(dockX + 22.0f, textY, "Linear Search Complexity", TextRole::BODY, Palette::TEXT_ACCENT);
            drawText(dockX + 22.0f, textY + 20.0f, "Time: Best O(1), Avg/Worst O(n)", TextRole::BODY, Palette::TEXT_WHITE);
            drawText(dockX + 22.0f, textY + 38.0f, "Space: O(1)", TextRole::BODY, Palette::TEXT_MUTED);
        } else if (app.currentSearchAlg == SearchAlgorithm::BINARY_SEARCH) {
            drawText(dockX + 22.0f, textY, "Binary Search (Sorted Array)", TextRole::BODY, Palette::TEXT_ACCENT);
            drawText(dockX + 22.0f, textY + 20.0f, "Time: Best O(1), Avg/Worst O(log n)", TextRole::BODY, Palette::TEXT_WHITE);
            drawText(dockX + 22.0f, textY + 38.0f, "Space: O(1)", TextRole::BODY, Palette::TEXT_MUTED);
        } else {
            drawText(dockX + 22.0f, textY, "Jump Search (Sorted Array)", TextRole::BODY, Palette::TEXT_ACCENT);
            drawText(dockX + 22.0f, textY + 20.0f, "Time: O(sqrt(n))", TextRole::BODY, Palette::TEXT_WHITE);
            drawText(dockX + 22.0f, textY + 38.0f, "Space: O(1)", TextRole::BODY, Palette::TEXT_MUTED);
        }
    }

    // Graphical Swatch Legend (Bottom Bar)
    float legY = static_cast<float>(app.windowH) - 45.0f;
    float swatchSz = 14.0f;

    auto drawLegendItem = [&](float x, const Color4f& color, const std::string& label) {
        drawQuad(x, legY - swatchSz + 2.0f, swatchSz, swatchSz, color);
        drawQuadOutline(x, legY - swatchSz + 2.0f, swatchSz, swatchSz, 1.0f, Palette::TEXT_WHITE);
        drawText(x + swatchSz + 8.0f, legY, label, TextRole::BODY, Palette::TEXT_WHITE);
    };

    drawLegendItem(40.0f,  Palette::BAR_DEFAULT, "Default");
    drawLegendItem(170.0f, Palette::COMPARE,     "Comparing");
    drawLegendItem(310.0f, Palette::ACTIVE_OP,   "Active / Swap");
    drawLegendItem(470.0f, Palette::SUCCESS,     "Sorted / Found");
    drawLegendItem(630.0f, Palette::BOUNDARY,    "Boundary / Range");
}

// ============================================================================
// CONFIG DASHBOARD RENDERING (MULTI-LINE FULL DISPLAY)
// ============================================================================
static void renderModernConfigDashboard() {
    glClearColor(Palette::BG_DARK.r, Palette::BG_DARK.g, Palette::BG_DARK.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Top Header
    drawText(55.0f, 55.0f, "ALGO VISION", TextRole::DISPLAY, Palette::TEXT_ACCENT);
    drawText(275.0f, 52.0f, "// Interactive Algorithm Visualizer", TextRole::SUBTITLE, Palette::TEXT_MUTED);

    drawQuad(55.0f, 75.0f, static_cast<float>(app.windowW) - 110.0f, 2.0f, Palette::CARD_BORDER);

    // Section 1 Card (Array Configuration)
    float card1X = 45.0f;
    float card1Y = 95.0f;
    float card1W = 590.0f;
    float card1H = 405.0f;

    drawQuad(card1X, card1Y, card1W, card1H, Palette::CARD_BG);
    drawQuadOutline(card1X, card1Y, card1W, card1H, 1.5f, Palette::CARD_BORDER);
    drawText(card1X + 20.0f, card1Y + 35.0f, "1. ARRAY CONFIGURATION", TextRole::SUBTITLE, Palette::TEXT_WHITE);

    if (app.inputMode == InputMode::MANUAL) {
        float boxX = card1X + 20.0f;
        float boxY = card1Y + 140.0f;
        float boxW = card1W - 40.0f;
        float boxH = 46.0f;

        drawQuad(boxX, boxY, boxW, boxH, Color4f(0.06f, 0.08f, 0.12f, 1.0f));
        drawQuadOutline(boxX, boxY, boxW, boxH, app.typingInManualInput ? 2.0f : 1.0f,
                        app.typingInManualInput ? Palette::TEXT_ACCENT : Palette::CARD_BORDER);

        std::string textToDisplay = app.manualInputStr + (app.typingInManualInput ? "_" : "");
        drawText(boxX + 16.0f, boxY + 28.0f, textToDisplay, TextRole::MONO, Palette::TEXT_WHITE);

        drawText(boxX, boxY + 68.0f, "Click box to edit. Enter 3-50 integers (1-100) separated by commas.", TextRole::BODY, Palette::TEXT_MUTED);
    } else {
        float sliderX = card1X + 20.0f;
        float sliderY = card1Y + 160.0f;
        float sliderW = 320.0f;

        drawQuad(sliderX, sliderY, sliderW, 6.0f, Palette::CARD_BORDER);

        float norm = static_cast<float>(app.arraySize - 3) / (50.0f - 3.0f);
        float handleX = sliderX + norm * sliderW;
        drawQuad(handleX - 10.0f, sliderY - 8.0f, 20.0f, 22.0f, Palette::TEXT_ACCENT);

        std::string szStr = "Array Size: " + std::to_string(app.arraySize);
        drawText(sliderX, sliderY - 18.0f, szStr, TextRole::BODY, Palette::TEXT_WHITE);
    }

    // MULTI-LINE AUTO-WRAPPING DISPLAY: Renders every single item without truncation dots
    float textStartX = card1X + 20.0f;
    float currentTextY = card1Y + 245.0f;
    float maxLineWidth = card1W - 40.0f;

    std::string header = "Current Array (" + std::to_string(app.sortOriginal.size()) + " items): ";
    std::string lineBuffer = header;

    for (size_t i = 0; i < app.sortOriginal.size(); ++i) {
        std::string numToken = std::to_string(app.sortOriginal[i]) + (i + 1 < app.sortOriginal.size() ? ", " : "");
        
        if (getTextWidth(lineBuffer + numToken, TextRole::MONO) > maxLineWidth) {
            drawText(textStartX, currentTextY, lineBuffer, TextRole::MONO, Palette::TEXT_MUTED);
            currentTextY += 16.0f;
            lineBuffer = "  " + numToken;
        } else {
            lineBuffer += numToken;
        }
    }
    if (!lineBuffer.empty()) {
        drawText(textStartX, currentTextY, lineBuffer, TextRole::MONO, Palette::TEXT_MUTED);
    }

    // Speed Controls inside Section 1
    float spdSliderX = card1X + 20.0f;
    float spdSliderY = card1Y + 360.0f;
    float spdSliderW = 320.0f;

    drawQuad(card1X + 20.0f, card1Y + 305.0f, card1W - 40.0f, 1.0f, Palette::CARD_BORDER);
    drawText(card1X + 20.0f, card1Y + 330.0f, "SIMULATION PLAYBACK SPEED", TextRole::BODY, Palette::TEXT_WHITE);

    drawQuad(spdSliderX, spdSliderY, spdSliderW, 6.0f, Palette::CARD_BORDER);
    float spdNorm = static_cast<float>(app.speedIndex) / static_cast<float>(app.speedSteps.size() - 1);
    float spdHandleX = spdSliderX + spdNorm * spdSliderW;
    drawQuad(spdHandleX - 10.0f, spdSliderY - 8.0f, 20.0f, 22.0f, Palette::TEXT_ACCENT);

    std::ostringstream spdSs;
    spdSs.precision(1);
    spdSs << std::fixed << app.getSpeedMultiplier() << "x (" << app.getOpsPerSecond() << " ops/sec)";
    drawText(spdSliderX + spdSliderW + 18.0f, spdSliderY + 5.0f, spdSs.str(), TextRole::BODY, Palette::TEXT_ACCENT);

    // Section 2 Card (Operation & Algorithm Selection)
    float card2X = 665.0f;
    float card2Y = 95.0f;
    float card2W = static_cast<float>(app.windowW) - card2X - 45.0f;
    float card2H = 405.0f;

    drawQuad(card2X, card2Y, card2W, card2H, Palette::CARD_BG);
    drawQuadOutline(card2X, card2Y, card2W, card2H, 1.5f, Palette::CARD_BORDER);
    drawText(card2X + 20.0f, card2Y + 35.0f, "2. ALGORITHM SELECTION", TextRole::SUBTITLE, Palette::TEXT_WHITE);

    drawText(card2X + 20.0f, card2Y + 150.0f, "SELECT ACTIVE ALGORITHM:", TextRole::BODY, Palette::TEXT_MUTED);

    if (app.selectedCategory == Category::SEARCHING) {
        drawText(card2X + 20.0f, card2Y + 250.0f, "TARGET VALUE TO SEARCH:", TextRole::BODY, Palette::TEXT_WHITE);

        float tgtBoxX = card2X + 20.0f;
        float tgtBoxY = card2Y + 290.0f;
        float tgtBoxW = 100.0f;
        float tgtBoxH = 42.0f;

        drawQuad(tgtBoxX, tgtBoxY, tgtBoxW, tgtBoxH, Color4f(0.06f, 0.08f, 0.12f, 1.0f));
        drawQuadOutline(tgtBoxX, tgtBoxY, tgtBoxW, tgtBoxH, app.typingInTargetInput ? 2.0f : 1.0f,
                        app.typingInTargetInput ? Palette::TEXT_ACCENT : Palette::CARD_BORDER);

        std::string tgtDisp = app.searchTargetInputStr + (app.typingInTargetInput ? "_" : "");
        drawText(tgtBoxX + 16.0f, tgtBoxY + 28.0f, tgtDisp, TextRole::MONO, Palette::TEXT_ACCENT);

        if (app.currentSearchAlg == SearchAlgorithm::BINARY_SEARCH || app.currentSearchAlg == SearchAlgorithm::JUMP_SEARCH) {
            drawText(card2X + 20.0f, card2Y + 360.0f, "* Note: Binary & Jump Search require sorted data and will auto-sort.", TextRole::BODY, Palette::ACTIVE_OP);
        }
    }

    // Launch Area
    drawQuad(45.0f, static_cast<float>(app.windowH) - 130.0f, static_cast<float>(app.windowW) - 90.0f, 1.5f, Palette::CARD_BORDER);

    std::string readyStatus = "CONFIGURED: ";
    if (app.selectedCategory == Category::SORTING) {
        if (app.currentSortAlg == SortAlgorithm::BUBBLE_SORT) readyStatus += "Bubble Sort";
        else if (app.currentSortAlg == SortAlgorithm::SELECTION_SORT) readyStatus += "Selection Sort";
        else if (app.currentSortAlg == SortAlgorithm::INSERTION_SORT) readyStatus += "Insertion Sort";
        else if (app.currentSortAlg == SortAlgorithm::MERGE_SORT) readyStatus += "Merge Sort";
    } else {
        if (app.currentSearchAlg == SearchAlgorithm::LINEAR_SEARCH) readyStatus += "Linear Search";
        else if (app.currentSearchAlg == SearchAlgorithm::BINARY_SEARCH) readyStatus += "Binary Search";
        else if (app.currentSearchAlg == SearchAlgorithm::JUMP_SEARCH) readyStatus += "Jump Search";
        readyStatus += " (Target: " + std::to_string(app.searchTarget) + ")";
    }
    readyStatus += " | Elements: " + std::to_string(app.sortOriginal.size());
    drawText(355.0f, static_cast<float>(app.windowH) - 72.0f, readyStatus, TextRole::SUBTITLE, Palette::TEXT_WHITE);

    // Draw Dashboard Buttons
    for (size_t i = 0; i < app.buttons.size(); ++i) {
        const Button& b = app.buttons[i];
        Color4f bg = b.isActive ? Palette::BTN_ACTIVE : (b.isHovered ? Palette::BTN_HOVER : b.color);
        Color4f borderCol = b.isActive ? Palette::CARD_BORDER_ACT : Palette::CARD_BORDER;

        if (b.id == 400) {
            bg = b.isHovered ? Palette::BTN_LAUNCH_HOV : Palette::BTN_LAUNCH;
            borderCol = Palette::SUCCESS;
        }

        drawQuad(b.x, b.y, b.w, b.h, bg);
        drawQuadOutline(b.x, b.y, b.w, b.h, b.isActive ? 2.0f : 1.0f, borderCol);

        TextRole textRole = (b.id == 400) ? TextRole::SUBTITLE : TextRole::BODY;
        int strW = getTextWidth(b.label, textRole);
        Color4f tc = (b.id == 400) ? Palette::BG_DARK : Palette::TEXT_WHITE;

        drawText(b.x + (b.w - static_cast<float>(strW)) * 0.5f, b.y + b.h * 0.62f, b.label, textRole, tc);
    }
}

// ============================================================================
// MAIN GLUT CALLBACKS & ENTRY POINT
// ============================================================================
static void display() {
    if (app.screen == ScreenState::CONFIG_SCREEN) {
        renderModernConfigDashboard();
    } else {
        glClearColor(Palette::BG_DARK.r, Palette::BG_DARK.g, Palette::BG_DARK.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (app.screen == ScreenState::SORTING_VIEW) {
            renderSortingView();
        } else if (app.screen == ScreenState::SEARCHING_VIEW) {
            renderSearchingView();
        }
        renderVisualizerUIOverlay();
    }

    glutSwapBuffers();
}

static void reshape(int w, int h) {
    if (h == 0) h = 1;
    app.windowW = w;
    app.windowH = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<double>(w), static_cast<double>(h), 0.0);

    app.rebuildButtons();
}

static void timer(int) {
    if (app.screen == ScreenState::SORTING_VIEW && app.sortPlaying) {
        if (app.sortIndex + 1 < app.sortTimeline.size()) {
            app.sortIndex++;
        } else {
            app.sortPlaying = false;
            app.rebuildButtons();
        }
    } else if (app.screen == ScreenState::SEARCHING_VIEW && app.searchPlaying) {
        if (app.searchIndex + 1 < app.searchTimeline.size()) {
            app.searchIndex++;
        } else {
            app.searchPlaying = false;
            app.rebuildButtons();
        }
    }

    glutPostRedisplay();
    int msec = (std::max)(10, 1000 / app.getOpsPerSecond());
    glutTimerFunc(msec, timer, 0);
}

static void mousePassiveMotion(int x, int y) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    bool needRedraw = false;

    for (size_t i = 0; i < app.buttons.size(); ++i) {
        bool hover = app.buttons[i].contains(fx, fy);
        if (app.buttons[i].isHovered != hover) {
            app.buttons[i].isHovered = hover;
            needRedraw = true;
        }
    }
    if (needRedraw) glutPostRedisplay();
}

static void mouseClick(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON) return;

    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);

    if (state == GLUT_DOWN) {
        if (app.screen == ScreenState::CONFIG_SCREEN) {
            if (app.inputMode == InputMode::MANUAL && fx >= 65.0f && fx <= 615.0f && fy >= 235.0f && fy <= 281.0f) {
                app.typingInManualInput = true;
                app.typingInTargetInput = false;
            } else if (app.selectedCategory == Category::SEARCHING && fx >= 685.0f && fx <= 785.0f && fy >= 385.0f && fy <= 427.0f) {
                app.typingInTargetInput = true;
                app.typingInManualInput = false;
            } else {
                app.typingInManualInput = false;
                app.typingInTargetInput = false;
            }

            if (app.inputMode == InputMode::RANDOM && fx >= 65.0f && fx <= 385.0f && fy >= 245.0f && fy <= 275.0f) {
                float norm = (fx - 65.0f) / 320.0f;
                norm = (std::max)(0.0f, (std::min)(1.0f, norm));
                app.arraySize = 3 + static_cast<int>(norm * (50.0f - 3.0f));
                app.generateRandomArray();
            }

            if (fx >= 65.0f && fx <= 385.0f && fy >= 445.0f && fy <= 475.0f) {
                float norm = (fx - 65.0f) / 320.0f;
                norm = (std::max)(0.0f, (std::min)(1.0f, norm));
                app.speedIndex = static_cast<size_t>(norm * (app.speedSteps.size() - 1));
            }
        }

        for (size_t i = 0; i < app.buttons.size(); ++i) {
            const Button& b = app.buttons[i];
            if (b.contains(fx, fy)) {
                switch (b.id) {
                case 101:
                    app.inputMode = InputMode::MANUAL;
                    app.parseManualInput();
                    app.rebuildButtons();
                    break;
                case 102:
                    app.inputMode = InputMode::RANDOM;
                    app.generateRandomArray();
                    app.rebuildButtons();
                    break;
                case 103:
                    app.generateRandomArray();
                    app.rebuildButtons();
                    break;
                case 201:
                    app.selectedCategory = Category::SORTING;
                    app.rebuildButtons();
                    break;
                case 202:
                    app.selectedCategory = Category::SEARCHING;
                    app.rebuildButtons();
                    break;

                case 301: app.currentSortAlg = SortAlgorithm::BUBBLE_SORT; app.rebuildButtons(); break;
                case 302: app.currentSortAlg = SortAlgorithm::SELECTION_SORT; app.rebuildButtons(); break;
                case 303: app.currentSortAlg = SortAlgorithm::INSERTION_SORT; app.rebuildButtons(); break;
                case 304: app.currentSortAlg = SortAlgorithm::MERGE_SORT; app.rebuildButtons(); break;

                case 311: app.currentSearchAlg = SearchAlgorithm::LINEAR_SEARCH; app.rebuildButtons(); break;
                case 312: app.currentSearchAlg = SearchAlgorithm::BINARY_SEARCH; app.rebuildButtons(); break;
                case 313: app.currentSearchAlg = SearchAlgorithm::JUMP_SEARCH; app.rebuildButtons(); break;

                case 314:
                    if (!app.searchOriginal.empty()) {
                        app.searchTarget = app.searchOriginal[rand() % app.searchOriginal.size()];
                        app.searchTargetInputStr = std::to_string(app.searchTarget);
                        app.rebuildButtons();
                    }
                    break;

                case 400:
                    if (app.selectedCategory == Category::SORTING) {
                        app.screen = ScreenState::SORTING_VIEW;
                        app.buildSortTimeline();
                    } else {
                        app.screen = ScreenState::SEARCHING_VIEW;
                        app.buildSearchTimeline();
                    }
                    app.rebuildButtons();
                    break;

                case 10:
                    app.screen = ScreenState::CONFIG_SCREEN;
                    app.sortPlaying = false;
                    app.searchPlaying = false;
                    app.rebuildButtons();
                    break;

                case 20: app.currentSortAlg = SortAlgorithm::BUBBLE_SORT; app.buildSortTimeline(); app.rebuildButtons(); break;
                case 21: app.currentSortAlg = SortAlgorithm::SELECTION_SORT; app.buildSortTimeline(); app.rebuildButtons(); break;
                case 22: app.currentSortAlg = SortAlgorithm::INSERTION_SORT; app.buildSortTimeline(); app.rebuildButtons(); break;
                case 23: app.currentSortAlg = SortAlgorithm::MERGE_SORT; app.buildSortTimeline(); app.rebuildButtons(); break;

                case 30: app.currentSearchAlg = SearchAlgorithm::LINEAR_SEARCH; app.buildSearchTimeline(); app.rebuildButtons(); break;
                case 31: app.currentSearchAlg = SearchAlgorithm::BINARY_SEARCH; app.buildSearchTimeline(); app.rebuildButtons(); break;
                case 32: app.currentSearchAlg = SearchAlgorithm::JUMP_SEARCH; app.buildSearchTimeline(); app.rebuildButtons(); break;

                case 40:
                    if (app.screen == ScreenState::SORTING_VIEW) app.sortPlaying = !app.sortPlaying;
                    else app.searchPlaying = !app.searchPlaying;
                    app.rebuildButtons();
                    break;
                case 41:
                    if (app.screen == ScreenState::SORTING_VIEW) {
                        app.sortPlaying = false;
                        if (app.sortIndex + 1 < app.sortTimeline.size()) app.sortIndex++;
                    } else {
                        app.searchPlaying = false;
                        if (app.searchIndex + 1 < app.searchTimeline.size()) app.searchIndex++;
                    }
                    app.rebuildButtons();
                    break;
                case 42:
                    if (app.screen == ScreenState::SORTING_VIEW) {
                        app.sortPlaying = false;
                        if (app.sortIndex > 0) app.sortIndex--;
                    } else {
                        app.searchPlaying = false;
                        if (app.searchIndex > 0) app.searchIndex--;
                    }
                    app.rebuildButtons();
                    break;
                case 43:
                    if (app.screen == ScreenState::SORTING_VIEW) {
                        app.sortIndex = 0;
                        app.sortPlaying = true;
                    } else {
                        app.searchIndex = 0;
                        app.searchPlaying = true;
                    }
                    app.rebuildButtons();
                    break;
                case 44:
                    app.generateRandomArray();
                    if (app.screen == ScreenState::SORTING_VIEW) app.buildSortTimeline();
                    else app.buildSearchTimeline();
                    app.rebuildButtons();
                    break;
                case 45:
                    if (app.speedIndex > 0) app.speedIndex--;
                    app.rebuildButtons();
                    break;
                case 46:
                    if (app.speedIndex + 1 < app.speedSteps.size()) app.speedIndex++;
                    app.rebuildButtons();
                    break;
                case 47:
                    if (app.arraySize > 6) {
                        app.arraySize -= 2;
                        app.generateRandomArray();
                        if (app.screen == ScreenState::SORTING_VIEW) app.buildSortTimeline();
                        else app.buildSearchTimeline();
                    }
                    break;
                case 48:
                    if (app.arraySize < 48) {
                        app.arraySize += 2;
                        app.generateRandomArray();
                        if (app.screen == ScreenState::SORTING_VIEW) app.buildSortTimeline();
                        else app.buildSearchTimeline();
                    }
                    break;
                case 50:
                    if (!app.searchOriginal.empty()) {
                        app.searchTarget = app.searchOriginal[rand() % app.searchOriginal.size()];
                        app.searchTargetInputStr = std::to_string(app.searchTarget);
                        app.buildSearchTimeline();
                    }
                    break;
                case 51:
                    app.searchTarget = 999;
                    app.searchTargetInputStr = "999";
                    app.buildSearchTimeline();
                    break;
                }
                glutPostRedisplay();
                return;
            }
        }
    }
}

static void keyboard(unsigned char key, int, int) {
    if (app.typingInManualInput) {
        if (key == 13) {
            app.typingInManualInput = false;
            app.parseManualInput();
            app.rebuildButtons();
        } else if (key == 8 || key == 127) {
            if (!app.manualInputStr.empty()) {
                app.manualInputStr.pop_back();
            }
        } else if ((key >= '0' && key <= '9') || key == ',' || key == ' ') {
            app.manualInputStr.push_back(static_cast<char>(key));
        }
        glutPostRedisplay();
        return;
    }

    if (app.typingInTargetInput) {
        if (key == 13) {
            app.typingInTargetInput = false;
            try {
                app.searchTarget = std::stoi(app.searchTargetInputStr);
            } catch (...) {}
            app.rebuildButtons();
        } else if (key == 8 || key == 127) {
            if (!app.searchTargetInputStr.empty()) {
                app.searchTargetInputStr.pop_back();
            }
        } else if (key >= '0' && key <= '9') {
            if (app.searchTargetInputStr.length() < 4) {
                app.searchTargetInputStr.push_back(static_cast<char>(key));
            }
        }
        glutPostRedisplay();
        return;
    }

    if (key == 32) {
        if (app.screen == ScreenState::SORTING_VIEW) app.sortPlaying = !app.sortPlaying;
        else if (app.screen == ScreenState::SEARCHING_VIEW) app.searchPlaying = !app.searchPlaying;
        app.rebuildButtons();
    } else if (key == 'r' || key == 'R') {
        if (app.screen == ScreenState::SORTING_VIEW) { app.sortIndex = 0; app.sortPlaying = true; }
        else { app.searchIndex = 0; app.searchPlaying = true; }
        app.rebuildButtons();
    } else if (key == 27) {
        if (app.screen != ScreenState::CONFIG_SCREEN) {
            app.screen = ScreenState::CONFIG_SCREEN;
            app.sortPlaying = false;
            app.searchPlaying = false;
            app.rebuildButtons();
        }
    }
    glutPostRedisplay();
}

static void specialKeys(int key, int, int) {
    if (key == GLUT_KEY_UP) {
        if (app.speedIndex + 1 < app.speedSteps.size()) {
            app.speedIndex++;
            app.rebuildButtons();
        }
    } else if (key == GLUT_KEY_DOWN) {
        if (app.speedIndex > 0) {
            app.speedIndex--;
            app.rebuildButtons();
        }
    } else if (key == GLUT_KEY_RIGHT) {
        if (app.screen == ScreenState::SORTING_VIEW) {
            app.sortPlaying = false;
            if (app.sortIndex + 1 < app.sortTimeline.size()) app.sortIndex++;
        } else if (app.screen == ScreenState::SEARCHING_VIEW) {
            app.searchPlaying = false;
            if (app.searchIndex + 1 < app.searchTimeline.size()) app.searchIndex++;
        }
        app.rebuildButtons();
    } else if (key == GLUT_KEY_LEFT) {
        if (app.screen == ScreenState::SORTING_VIEW) {
            app.sortPlaying = false;
            if (app.sortIndex > 0) app.sortIndex--;
        } else if (app.screen == ScreenState::SEARCHING_VIEW) {
            app.searchPlaying = false;
            if (app.searchIndex > 0) app.searchIndex--;
        }
        app.rebuildButtons();
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(app.windowW, app.windowH);
    glutInitWindowPosition(80, 50);
    glutCreateWindow("ALGO VISION - Searching & Sorting Algorithm Visualizer");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    app.init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(150, timer, 0);

    glutMainLoop();
    return 0;
}