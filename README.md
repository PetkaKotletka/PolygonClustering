# Polygon Clustering

Solves the problem of clustering non-intersecting polygons based on their containment relationships.

## Setup

Compile the C++ solver:
```bash
g++ -o solve solve.cpp
```

Install Python dependencies:
```bash
pip install -r requirements.txt
```

## Running

**Solver only:**
```bash
./solve < long_tests/1.txt
```

**With visualizer:**
```bash
python visualize.py
```

## Input Format

Text file structure:
- First line: number of polygons
- For each polygon:
  - First line: number of vertices
  - Following lines: x y coordinates (one pair per line)

Example:
```
2
4
0.0 0.0
1.0 0.0
1.0 1.0
0.0 1.0
3
2.0 2.0
3.0 2.0
2.5 3.0
```

## Output Format

- First line: number of clusters
- Following lines: space-separated polygon indices for each cluster

## Test Directories

- `long_tests/` - Long test cases for direct testing
- `vis_tests/` - Test files for visualizer

Add new test files in the appropriate directory following the input format.

## Visualizer Controls

**File Operations:**

- Select test from list to load
- Click "Solve" to run clustering algorithm
- Click "Create test" to enter editor mode

**Editor Mode:**

- Click to place polygon vertices
- Press Enter to complete polygon (minimum 3 vertices)
- Press Ctrl+Z to undo last polygon
- Mouse wheel: zoom in/out
- Arrow keys: pan view
- Click "Save" to save test file
- Click "Exit edit" to exit the Editor mode