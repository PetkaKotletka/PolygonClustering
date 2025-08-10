#include <algorithm>
#include <iostream>
#include <vector>

constexpr double EPS = 1e-9;

using namespace std;

struct Point {
    double x, y;
};

struct Line {
    double a, b, c;
};

struct Polygon {
    int num_points;
    int index; // To keep the original order
    double left_x; // For sorting
    vector<Point> points;
};

struct Cluster { // Polygon with other polygons inside
    Polygon outer;
    vector<Polygon> inner;
};

Point operator-(const Point& p1, const Point& p2) {
    return Point{p1.x - p2.x, p1.y - p2.y};
}

Point operator+(const Point& p1, const Point& p2) {
    return Point{p1.x + p2.x, p1.y + p2.y};
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
    for (size_t i = 0; i < p.num_points; ++i) {
        is >> p.points[i];
        if (i == 0 || p.points[i].x < p.left_x) {
            p.left_x = p.points[i].x;
        }
    }
    return is;
}

int sign(double x) { // -1, 0, 1
    if (x < -EPS) {
        return -1;
    } else if (x > EPS) {
        return 1;
    } else {
        return 0;
    }
}

double cross_product(const Point& p1, const Point& p2) {
    return p1.x * p2.y - p1.y * p2.x;
}

double dot_product(const Point& p1, const Point& p2) {
    return p1.x * p2.x + p1.y * p2.y;
}

Line get_line(const Point& p1, const Point& p2) {
    Line l;
    l.a = p1.y - p2.y;
    l.b = p2.x - p1.x;
    l.c = p1.x * p2.y - p2.x * p1.y;
    return l;
}

Point intersect_lines(const Line& l1, const Line& l2) {
    Point p;
    p.x = (l1.b * l2.c - l1.c * l2.b) / (l1.a * l2.b - l1.b * l2.a);
    p.y = (l1.c * l2.a - l1.a * l2.c) / (l1.a * l2.b - l1.b * l2.a);
    return p;
}

bool intersects_ray_segment(const Point& r1, const Point& r2, const Point& s1, const Point& s2) {
    Point r_vec = r2 - r1;
    Point s1_vec = s1 - r1;
    Point s2_vec = s2 - r1;
    int cp1 = sign(cross_product(r_vec, s1_vec));
    int cp2 = sign(cross_product(r_vec, s2_vec));
    int dp1 = sign(dot_product(r_vec, s1_vec));
    int dp2 = sign(dot_product(r_vec, s2_vec));

    if (cp1 == cp2) {
        if (cp1 != 0) {
            return false;
        }
        // cp = 0
        if (dp1 >= 0 || dp2 >= 0) {
            return true;
        }
        return false;
    }

    // cp1 != cp2 => not parallel => there is an intersection point
    Point p = intersect_lines(get_line(r1, r2), get_line(s1, s2));
    if (sign(dot_product(r_vec, p - r1)) >= 0) {
        return true;
    }
    return false;
}

bool is_inside(const Polygon& polygon, const Point& p) {
    // Ray casting algorithm
    Point p2 = p + Point{123.45, 15.43}; // random ray
    int num_intersections = 0;
    for (size_t i = 0; i < polygon.num_points; ++i) {
        Point s1 = polygon.points[i];
        Point s2 = polygon.points[(i + 1) % polygon.num_points];
        if (intersects_ray_segment(p, p2, s1, s2)) {
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
