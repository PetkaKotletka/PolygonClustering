#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

struct Point {
    long long x, y;
};

struct Polygon {
    int num_points;
    int index; // To keep the original order
    long long left_x; // For sorting
    vector<Point> points;
};

struct Cluster { // Polygon with other polygons inside
    Polygon outer;
    vector<Polygon> inner;
};

Point operator-(const Point& p1, const Point& p2) {
    return Point{p1.x - p2.x, p1.y - p2.y};
}

istream& operator>>(istream& is, Point& p) {
    is >> p.x >> p.y;
    return is;
}

istream& operator>>(istream& is, Polygon& p) {
    is >> p.num_points;
    p.index = -1;
    p.left_x = 0;
    p.points.resize(p.num_points);
    for (int i = 0; i < p.num_points; ++i) {
        is >> p.points[i];
        if (i == 0 || p.points[i].x < p.left_x) {
            p.left_x = p.points[i].x;
        }
    }
    return is;
}

bool intersects_ray_segment(const Point& r, Point& s1, Point& s2) {
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
    for (int i = 0; i < polygon.num_points; ++i) {
        Point s1 = polygon.points[i];
        Point s2 = polygon.points[(i + 1) % polygon.num_points];
        if (s1.y == p.y) { // Handle the case when the ray passes through an s1 vertex
            continue;
        }
        if (s2.y == p.y) { // Handle the case when the ray passes through an s2 vertex
            if (s2.x < p.x) {
                continue;
            }

            Point s3 = polygon.points[(i + 2) % polygon.num_points];
            if ((s1.y < p.y && s2.y >= p.y) || (s1.y >= p.y && s2.y < p.y)) { // Only count as intersection if s1-s2-s3 passes across the ray
                ++num_intersections;
            }
        } else if (intersects_ray_segment(p, s1, s2)) {
            ++num_intersections;
        }
    }
    return num_intersections % 2 == 1;
}

vector<Cluster> get_clusters(const vector<Polygon>& polygons) {
    // We assume that polygons are sorted by the X coordinate of their leftmost point
    vector<Cluster> clusters;
    bool found_cluster;

    for (size_t pi = 0; pi < polygons.size(); ++pi) {
        found_cluster = false;

        for (size_t ci = 0; ci < clusters.size(); ++ci) {
            if (is_inside(clusters[ci].outer, polygons[pi].points[0])) {
                clusters[ci].inner.push_back(polygons[pi]);
                found_cluster = true;
                break;
            }
        }

        if (!found_cluster) {
            Cluster new_cluster;
            new_cluster.outer = polygons[pi];
            clusters.push_back(new_cluster);
        }
    }

    return clusters;
}

void get_borders(vector<Polygon>& polygons, vector<vector<int>>& borders) {
    // We assume that polygons are sorted by the X coordinate of their leftmost point
    vector<Cluster> outer_clusters = get_clusters(polygons);

    for (size_t i = 0; i < outer_clusters.size(); ++i) {
        vector<Cluster> inner_clusters = get_clusters(outer_clusters[i].inner);

        vector<int> cluster_borders(inner_clusters.size() + 1);
        cluster_borders[0] = outer_clusters[i].outer.index;
        for (size_t j = 0; j < inner_clusters.size(); ++j) {
            cluster_borders[j + 1] = inner_clusters[j].outer.index;
        }
        borders.push_back(cluster_borders);

        for (size_t j = 0; j < inner_clusters.size(); ++j) {
            get_borders(inner_clusters[j].inner, borders);
        }
    }
}

int main() {
    int num_polygons;
    cin >> num_polygons;
    vector<Polygon> polygons(num_polygons);
    for (int i = 0; i < num_polygons; ++i) {
        cin >> polygons[i];
        polygons[i].index = i;
    }

    sort(polygons.begin(), polygons.end(), [](const Polygon& p1, const Polygon& p2) {
        return p1.left_x < p2.left_x;
    });

    vector<vector<int>> borders;
    get_borders(polygons, borders);

    cout << borders.size() << endl;
    for (size_t i = 0; i < borders.size(); ++i) {
        for (size_t j = 0; j < borders[i].size(); ++j) {
            cout << borders[i][j] << " ";
        }
        cout << endl;
    }
}
