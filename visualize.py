import tkinter as tk
from tkinter import ttk, simpledialog
import os
import subprocess
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np

class PolygonVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("Polygon Clustering Visualizer")

        # Current data
        self.polygons = []
        self.clusters = []
        self.current_file = None
        self.executable_path = "./solve"
        self.tests_dir = "vis_tests"
        self.is_solved = False

        # Editor state
        self.edit_mode = False
        self.current_polygon_points = []
        self.created_polygons = []
        self.mouse_pos = (0, 0)
        self.zoom_level = 1.0
        self.pan_x, self.pan_y = 0, 0

        # Message system
        self.message_after_id = None

        self.setup_ui()
        self.load_test_files()
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.setup_editor_events()
        self.update_solve_button()

    def snap_to_grid(self, x, y):
        """Snap coordinates to nearest integer"""
        return (round(x), round(y))

    def setup_ui(self):
        # Left panel for file list
        left_frame = ttk.Frame(self.root)
        left_frame.pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=5)

        ttk.Label(left_frame, text="Test Files:").pack()

        self.file_listbox = tk.Listbox(left_frame, width=30)
        self.file_listbox.pack(fill=tk.BOTH, expand=True)
        self.file_listbox.bind('<<ListboxSelect>>', self.on_file_select)

        # Buttons
        self.solve_btn = ttk.Button(left_frame, text="Solve", command=self.solve_clustering)
        self.solve_btn.pack(pady=2)

        self.create_btn = ttk.Button(left_frame, text="Create test", command=self.toggle_edit_mode)
        self.create_btn.pack(pady=2)

        self.save_btn = ttk.Button(left_frame, text="Save", command=self.save_test, state='disabled')
        self.save_btn.pack(pady=2)

        # Clusters info
        self.clusters_label = ttk.Label(left_frame, text="Clusters: 0")
        self.clusters_label.pack()

        # Edit mode info
        self.edit_label = ttk.Label(left_frame, text="")
        self.edit_label.pack()

        # Right panel for visualization
        right_frame = ttk.Frame(self.root)
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Matplotlib canvas
        self.fig, self.ax = plt.subplots(figsize=(8, 6))
        self.canvas = FigureCanvasTkAgg(self.fig, right_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # Message label in bottom right
        self.message_label = ttk.Label(right_frame, text="", foreground="red")
        self.message_label.pack(side=tk.BOTTOM, anchor=tk.SE)

        self.draw_polygons()

    def setup_editor_events(self):
        self.canvas.mpl_connect('button_press_event', self.on_canvas_click)
        self.canvas.mpl_connect('motion_notify_event', self.on_mouse_move)
        self.canvas.mpl_connect('scroll_event', self.on_scroll)
        self.root.bind('<Key>', self.on_key_press)
        self.root.focus_set()

    def show_message(self, text, duration=3000):
        """Show temporary message in bottom right corner"""
        if self.message_after_id:
            self.root.after_cancel(self.message_after_id)

        self.message_label.config(text=text)
        self.message_after_id = self.root.after(duration, lambda: self.message_label.config(text=""))

    def update_solve_button(self):
        """Update solve button state based on current conditions"""
        if self.edit_mode or not self.current_file or self.is_solved:
            self.solve_btn.config(state='disabled')
        else:
            self.solve_btn.config(state='normal')

    def load_test_files(self):
        if os.path.exists(self.tests_dir):
            files = [f for f in os.listdir(self.tests_dir) if f.endswith('.txt')]
            for file in sorted(files):
                self.file_listbox.insert(tk.END, file)

    def on_file_select(self, event):
        if self.edit_mode:
            return
        selection = self.file_listbox.curselection()
        if selection:
            filename = self.file_listbox.get(selection[0])
            self.current_file = os.path.join(self.tests_dir, filename)
            self.load_polygons()
            self.clusters = []
            self.is_solved = False
            self.clusters_label.config(text="Clusters: 0")
            self.draw_polygons()
            self.update_solve_button()

    def load_polygons(self):
        self.polygons = []
        with open(self.current_file, 'r') as f:
            n_polygons = int(f.readline().strip())

            for _ in range(n_polygons):
                n_points = int(f.readline().strip())
                points = []
                for _ in range(n_points):
                    x, y = map(float, f.readline().strip().split())
                    points.append((x, y))
                self.polygons.append(points)

    def draw_grid(self):
        """Draw grid when zoom is large enough"""
        if self.zoom_level > 0.01:
            return

        xlim = self.ax.get_xlim()
        ylim = self.ax.get_ylim()

        x_start = int(xlim[0]) - 1
        x_end = int(xlim[1]) + 1
        y_start = int(ylim[0]) - 1
        y_end = int(ylim[1]) + 1

        for x in range(x_start, x_end + 1):
            self.ax.axvline(x, color='lightgray', alpha=0.3, linewidth=0.5)

        for y in range(y_start, y_end + 1):
            self.ax.axhline(y, color='lightgray', alpha=0.3, linewidth=0.5)

    def draw_polygons(self):
        self.ax.clear()

        if not self.edit_mode and not self.polygons:
            self.ax.text(0.5, 0.5, "Please, select a test file",
                        transform=self.ax.transAxes, ha='center', va='center', fontsize=14)
            self.ax.grid(False)
            self.ax.set_xticks([])
            self.ax.set_yticks([])
            self.canvas.draw()
            return

        if self.edit_mode:
            # Set view limits based on zoom and pan for editor
            xlim = (self.pan_x - 5000/self.zoom_level, self.pan_x + 5000/self.zoom_level)
            ylim = (self.pan_y - 5000/self.zoom_level, self.pan_y + 5000/self.zoom_level)
            self.ax.set_xlim(xlim)
            self.ax.set_ylim(ylim)

            # Draw grid if zoomed in enough
            self.draw_grid()

            # Draw created polygons
            for polygon in self.created_polygons:
                points = polygon + [polygon[0]]
                xs, ys = zip(*points)
                self.ax.plot(xs, ys, 'o-', color='black', markersize=4, linewidth=1)

            # Draw current polygon being created
            if self.current_polygon_points:
                if len(self.current_polygon_points) >= 1:
                    xs, ys = zip(*self.current_polygon_points)
                    self.ax.plot(xs, ys, 'o-', color='blue', markersize=4, linewidth=2)

                # Draw line to mouse
                if len(self.current_polygon_points) >= 1:
                    snapped_mouse = self.snap_to_grid(*self.mouse_pos)
                    color = 'red' if self.would_intersect(self.current_polygon_points[-1], snapped_mouse) else 'blue'
                    self.ax.plot([self.current_polygon_points[-1][0], snapped_mouse[0]],
                               [self.current_polygon_points[-1][1], snapped_mouse[1]],
                               color=color, linewidth=2)

                # Draw closing line (dashed)
                if len(self.current_polygon_points) >= 2:
                    snapped_mouse = self.snap_to_grid(*self.mouse_pos)
                    color = 'red' if self.would_intersect(self.current_polygon_points[0], snapped_mouse) else 'green'
                    self.ax.plot([self.current_polygon_points[0][0], snapped_mouse[0]],
                               [self.current_polygon_points[0][1], snapped_mouse[1]],
                               color=color, linewidth=2, linestyle='--')
        else:
            # Auto-scale for viewing test cases
            all_points = []
            for polygon in self.polygons:
                all_points.extend(polygon)

            if all_points:
                xs, ys = zip(*all_points)
                margin = max(max(xs) - min(xs), max(ys) - min(ys)) * 0.1
                self.ax.set_xlim(min(xs) - margin, max(xs) + margin)
                self.ax.set_ylim(min(ys) - margin, max(ys) + margin)

            # Draw loaded polygons
            colors = plt.cm.Set1(np.linspace(0, 1, max(len(self.clusters), 1))) if self.clusters else ['black']

            for i, polygon in enumerate(self.polygons):
                if self.clusters:
                    cluster_idx = next((j for j, cluster in enumerate(self.clusters) if i in cluster), 0)
                    color = colors[cluster_idx % len(colors)]
                else:
                    color = 'black'

                points = polygon + [polygon[0]]
                xs, ys = zip(*points)
                self.ax.plot(xs, ys, 'o-', color=color, markersize=4, linewidth=1)

        self.ax.set_aspect('equal')
        self.ax.grid(True, alpha=0.3)
        self.canvas.draw()

    def cross_product(self, o, a, b):
        """Calculate cross product of vectors OA and OB"""
        return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])

    def segments_intersect(self, p1, p2, p3, p4):
        """Check if line segments p1-p2 and p3-p4 intersect using cross products"""
        # Calculate cross products
        cp1 = self.cross_product(p1, p2, p3)
        cp2 = self.cross_product(p1, p2, p4)
        cp3 = self.cross_product(p3, p4, p1)
        cp4 = self.cross_product(p3, p4, p2)

        # Check if segments intersect
        return (cp1 * cp2 < 0) and (cp3 * cp4 < 0)

    def would_intersect(self, start, end):
        """Check if segment would intersect with existing polygons or current polygon"""
        # Check against completed polygons
        for polygon in self.created_polygons:
            for i in range(len(polygon)):
                p3 = polygon[i]
                p4 = polygon[(i+1) % len(polygon)]
                if self.segments_intersect(start, end, p3, p4):
                    return True

        # Check against current polygon segments (but not adjacent ones)
        for i in range(len(self.current_polygon_points) - 1):
            p3 = self.current_polygon_points[i]
            p4 = self.current_polygon_points[i + 1]
            # Skip adjacent segment check
            if i == len(self.current_polygon_points) - 2:
                continue
            if self.segments_intersect(start, end, p3, p4):
                return True

        return False

    def toggle_edit_mode(self):
        self.edit_mode = not self.edit_mode

        if self.edit_mode:
            self.create_btn.config(text="Exit edit")
            self.save_btn.config(state='normal')
            self.edit_label.config(text="Edit Mode: Click to add points")
            self.created_polygons = []
            self.current_polygon_points = []
            self.zoom_level = 1.0
            self.pan_x, self.pan_y = 0, 0
            self.mouse_pos = (0, 0)
        else:
            self.create_btn.config(text="Create test")
            self.save_btn.config(state='disabled')
            self.edit_label.config(text="")
            self.current_polygon_points = []

        self.update_solve_button()
        self.draw_polygons()

    def on_canvas_click(self, event):
        if not self.edit_mode or event.inaxes != self.ax:
            return

        x, y = event.xdata, event.ydata
        if x is None or y is None:
            return

        snapped_pos = self.snap_to_grid(x, y)
        self.mouse_pos = snapped_pos

        # Check if starting new polygon or continuing current one
        if not self.current_polygon_points:
            # Start new polygon
            self.current_polygon_points = [snapped_pos]
        else:
            # Add point to current polygon
            if not self.would_intersect(self.current_polygon_points[-1], snapped_pos):
                self.current_polygon_points.append(snapped_pos)
            else:
                self.show_message("Line would intersect with existing segments!")

        self.draw_polygons()

    def on_mouse_move(self, event):
        if not self.edit_mode or event.inaxes != self.ax:
            return

        if event.xdata is not None and event.ydata is not None:
            self.mouse_pos = self.snap_to_grid(event.xdata, event.ydata)
            self.draw_polygons()

    def on_scroll(self, event):
        if not self.edit_mode:
            return

        # Zoom
        scale_factor = 1.2 if event.button == 'up' else 1/1.2
        new_zoom = self.zoom_level * scale_factor

        # Limit zoom
        if 0.001 <= new_zoom <= 1000:
            self.zoom_level = new_zoom
            self.draw_polygons()

    def on_key_press(self, event):
        if not self.edit_mode:
            return

        if event.keysym == 'Return':
            # Finish current polygon
            if len(self.current_polygon_points) >= 3:
                snapped_mouse = self.snap_to_grid(*self.mouse_pos)
                if not self.would_intersect(self.current_polygon_points[0], self.current_polygon_points[-1]):
                    self.created_polygons.append(self.current_polygon_points[:])
                    self.current_polygon_points = []
                    self.edit_label.config(text=f"Edit Mode: {len(self.created_polygons)} polygons created")
                else:
                    self.show_message("Closing line would intersect with existing segments!")
            else:
                self.show_message("Polygon must have at least 3 vertices!")

        elif event.keysym == 'z' and event.state & 0x4:  # Ctrl+Z
            # Undo last polygon
            if self.created_polygons:
                self.created_polygons.pop()
                self.edit_label.config(text=f"Edit Mode: {len(self.created_polygons)} polygons created")
            elif self.current_polygon_points:
                self.current_polygon_points = []

        elif event.keysym in ['Up', 'Down', 'Left', 'Right']:
            # Pan
            move_distance = 100 / self.zoom_level
            if event.keysym == 'Up':
                self.pan_y += move_distance
            elif event.keysym == 'Down':
                self.pan_y -= move_distance
            elif event.keysym == 'Left':
                self.pan_x -= move_distance
            elif event.keysym == 'Right':
                self.pan_x += move_distance

        self.draw_polygons()

    def save_test(self):
        if not self.created_polygons:
            self.show_message("No polygons to save!")
            return

        filename = simpledialog.askstring("Save Test", "Enter filename (without .txt):")
        if not filename:
            return

        # Ensure tests directory exists
        os.makedirs(self.tests_dir, exist_ok=True)

        filepath = os.path.join(self.tests_dir, filename + '.txt')

        try:
            with open(filepath, 'w') as f:
                f.write(f"{len(self.created_polygons)}\n")
                for polygon in self.created_polygons:
                    f.write(f"{len(polygon)}\n")
                    for x, y in polygon:
                        f.write(f"{x} {y}\n")

            # Refresh file list
            self.file_listbox.delete(0, tk.END)
            self.load_test_files()

            self.show_message(f"Test saved as {filename}.txt", 2000)

        except Exception as e:
            self.show_message(f"Failed to save file: {e}")

    def solve_clustering(self):
        if not self.current_file or self.is_solved:
            return

        try:
            with open(self.current_file, 'r') as f:
                file_content = f.read()

            result = subprocess.run([self.executable_path],
                              input=file_content, capture_output=True, text=True, check=True)

            lines = result.stdout.strip().split('\n')
            n_clusters = int(lines[0])

            self.clusters = []
            for i in range(1, n_clusters + 1):
                indices = list(map(int, lines[i].split()))
                self.clusters.append(indices)

            self.clusters_label.config(text=f"Clusters: {n_clusters}")
            self.is_solved = True
            self.update_solve_button()
            self.draw_polygons()

        except subprocess.CalledProcessError as e:
            self.show_message(f"Error running program: {e}")
        except Exception as e:
            self.show_message(f"Error parsing output: {e}")

    def on_closing(self):
        plt.close('all')
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = PolygonVisualizer(root)
    root.mainloop()