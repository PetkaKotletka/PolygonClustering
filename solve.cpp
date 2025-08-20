#include <algorithm>
#include <climits>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

struct Point {
    long long x, y;
};

struct Polygon {
    int index; // To keep the original order
    long long left_x; // For sorting
    vector<Point> points;
};

struct Node {
    vector<unique_ptr<Node>> children;
    const Polygon* polygon;
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
    p.index = -1;
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
        } else if (intersects_ray_segment(p, s1, s2)) {
            ++num_intersections;
        }
    }
    return num_intersections % 2 == 1;
}

unique_ptr<Node> build_tree(const vector<unique_ptr<Polygon>>& polygons) {
    unique_ptr<Node> root = make_unique<Node>();
    root->polygon = nullptr;

    root->children.resize(polygons.size());
    for (size_t i = 0; i < polygons.size(); ++i) {
        root->children[i] = make_unique<Node>();
        root->children[i]->polygon = polygons[i].get();
    }

    return root;
}

void create_clusters(Node* root) {
    // We assume that polygons are sorted by the X coordinate of their leftmost point
    vector<unique_ptr<Node>> clusters;
    bool found_cluster;

    for (size_t child_idx = 0; child_idx < root->children.size(); ++child_idx) {
        found_cluster = false;

        for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
            if (is_inside(*clusters[cluster_idx]->polygon, root->children[child_idx]->polygon->points[0])) {
                clusters[cluster_idx]->children.push_back(move(root->children[child_idx]));
                found_cluster = true;
                break;
            }
        }

        if (!found_cluster) {
            clusters.push_back(move(root->children[child_idx]));
        }
    }

    root->children = move(clusters);
}

void get_borders(Node* root, vector<vector<int>>& borders) {
    // We assume that polygons are sorted by the X coordinate of their leftmost point
    create_clusters(root);

    for (size_t child_idx = 0; child_idx < root->children.size(); ++child_idx) {
        create_clusters(root->children[child_idx].get());

        vector<int> cluster_borders(root->children[child_idx]->children.size() + 1);
        cluster_borders[0] = root->children[child_idx]->polygon->index;

        for (size_t grandchild_idx = 0; grandchild_idx < root->children[child_idx]->children.size(); ++grandchild_idx) {
            cluster_borders[grandchild_idx + 1] = root->children[child_idx]->children[grandchild_idx]->polygon->index;
        }

        borders.push_back(move(cluster_borders));

        for (size_t grandchild_idx = 0; grandchild_idx < root->children[child_idx]->children.size(); ++grandchild_idx) {
            get_borders(root->children[child_idx]->children[grandchild_idx].get(), borders);
        }
    }
}

int main() {
    int num_polygons;
    cin >> num_polygons;
    vector<unique_ptr<Polygon>> polygons(num_polygons);

    for (int i = 0; i < num_polygons; ++i) {
        polygons[i] = make_unique<Polygon>();
        cin >> *polygons[i];
        polygons[i]->index = i;
    }

    sort(polygons.begin(), polygons.end(), [](const unique_ptr<Polygon>& p1, const unique_ptr<Polygon>& p2) {
        return p1->left_x < p2->left_x;
    });

    unique_ptr<Node> root = build_tree(polygons);

    vector<vector<int>> borders;
    get_borders(root.get(), borders);

    cout << borders.size() << endl;
    for (size_t i = 0; i < borders.size(); ++i) {
        for (size_t j = 0; j < borders[i].size(); ++j) {
            cout << borders[i][j] << " ";
        }
        cout << endl;
    }
}
