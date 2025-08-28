#include <algorithm>
#include <climits>
#include <iostream>
#include <memory>
#include <list>
#include <vector>

using namespace std;

struct Point {
    long long x, y;
};

struct Polygon {
    long long left_x; // For sorting
    vector<Point> points;
};

struct Node {
    size_t pi; // Polygon index
    list<size_t> children;
};

Point operator-(const Point& p1, const Point& p2) {
    return Point{p1.x - p2.x, p1.y - p2.y};
}

istream& operator>>(istream& is, Point& p) {
    is >> p.x >> p.y;
    return is;
}

istream& operator>>(istream& is, Polygon& p) {
    size_t sz;
    is >> sz;
    p.left_x = LLONG_MAX;
    p.points.resize(sz);

    for (int i = 0; i < sz; ++i) {
        is >> p.points[i];
        if (p.points[i].x < p.left_x) {
            p.left_x = p.points[i].x;
        }
    }
    return is;
}

bool intersects_ray_segment(const Point& r, Point s1, Point s2) {
    // Assume that ray is horizontal and points to the right (positive X)

    if ((s1.y - r.y) * (s2.y - r.y) > 0) { // s1 and s2 are on the same side of the ray
        return false;
    }

    if (s1.y == s2.y) { // Segment is horizontal and lies on the ray
        return max(s1.x, s2.x) >= r.x;
    }

    if (s1.x > s2.x) {
        swap(s1, s2);
    }

    long long x;

    if (s1.y > s2.y) {
        x = s1.x + (s1.y - r.y) * (s2.x - s1.x) / (s1.y - s2.y);
    } else {
        x = s1.x + (r.y - s1.y) * (s2.x - s1.x) / (s2.y - s1.y);
    }

    return x >= r.x;
}

bool is_inside(const Polygon& polygon, const Point& p) {
    // Ray casting algorithm (with horizontal ray)
    int num_intersections = 0;
    for (int i = 0; i < polygon.points.size(); ++i) {
        Point s1 = polygon.points[i];
        Point s2 = polygon.points[(i + 1) % polygon.points.size()];

        if (s2.y == p.y) { // Handle the case when the ray passes through an s2 vertex
            if (s2.x < p.x) {
                continue;
            }

            Point s3 = polygon.points[(i + 2) % polygon.points.size()];
            if ((s1.y < p.y && s3.y >= p.y) || (s1.y >= p.y && s3.y < p.y)) { // Only count as intersection if s1-s2-s3 passes across the ray
                ++num_intersections;
            }
        } else if (s1.y == p.y) { // Skip when the ray passes through an s1 vertex but not an s2 vertex
            continue;
        } else if (intersects_ray_segment(p, move(s1), move(s2))) {
            ++num_intersections;
        }
    }
    return num_intersections % 2 == 1;
}

void get_borders(size_t ni, const vector<Node>& tree, vector<bool>& used, vector<vector<size_t>>& borders) {
    used[ni] = true;

    borders.emplace_back();
    borders.back().reserve(tree[ni].children.size() + 1);
    borders.back().push_back(tree[ni].pi);

    for (auto& ci : tree[ni].children) {
        used[ci] = true;
        borders.back().push_back(tree[ci].pi);
    }
    for (auto& ci : tree[ni].children) {
        for (auto& gci : tree[ci].children) {
            get_borders(gci, tree, used, borders);
        }
    }
}

int main() {
    int num_polygons;
    cin >> num_polygons;
    vector<Polygon> polygons(num_polygons);
    vector<Node> tree(num_polygons);
    vector<bool> used(num_polygons, false);
    vector<vector<size_t>> borders;

    for (int i = 0; i < num_polygons; ++i) {
        cin >> polygons[i];
        tree[i].pi = i;
    }

    sort(tree.begin(), tree.end(), [&polygons](const Node& n1, const Node& n2) {
        return polygons[n1.pi].left_x < polygons[n2.pi].left_x;
    });

    for (int i = 0; i < tree.size(); ++i) {
        for (int j = i - 1; j >= 0; --j) {
            if (is_inside(polygons[tree[j].pi], polygons[tree[i].pi].points[0])) {
                tree[j].children.push_back(i);
                break;
            }
        }
    }

    for (size_t ni = 0; ni < tree.size(); ++ni) {
        if (!used[ni]) {
            get_borders(ni, tree, used, borders);
        }
    }

    cout << borders.size() << endl;
    for (auto& border : borders) {
        for (auto& pi : border) {
            cout << pi << " ";
        }
        cout << endl;
    }
}
