#include <bits/stdc++.h>
using namespace std;
std::pair<std::size_t, std::size_t> find_start(const std::vector<std::string> &grid) {
    for (std::size_t r = 0; r < grid.size(); ++r) {
        const auto &row = grid[r];
        for (std::size_t c = 0; c < row.size(); ++c) {
            if (row[c] == 'S') {
                return {r, c};
            }
        }
    }
    throw std::exception();
}

struct Point {
    std::size_t row, col, dist;
};

char replace(const char grid_ch, const char start_replace) {
    return grid_ch == 'S' ? start_replace : grid_ch;
}

std::vector<Point> two_neighbors(
        const std::vector<std::string> &grid, const Point &point, const char start_replace) {
    const Point north = {point.row - 1, point.col, point.dist + 1};
    const Point south = {point.row + 1, point.col, point.dist + 1};
    const Point east = {point.row, point.col + 1, point.dist + 1};
    const Point west = {point.row, point.col - 1, point.dist + 1};
    switch (replace(grid[point.row][point.col], start_replace)) {
        case '-':
            return {east, west};
        case '|':
            return {north, south};
        case 'L':
            return {north, east};
        case 'J':
            return {north, west};
        case '7':
            return {south, west};
        case 'F':
            return {south, east};
        default:
            throw std::exception();
    }
}

void solve(const std::vector<std::string> &grid, const char start_replace) {
    std::size_t row, col; 
    for (std::size_t r = 0; r < grid.size(); ++r) {
        const auto &roww = grid[r];
        for (std::size_t c = 0; c < roww.size(); ++c) {
            if (roww[c] == 'S') {
                row = r;
                col = c;
            }
        }
    }
    std::deque<Point> que;
    std::set<std::pair<std::size_t, std::size_t>> seen;
    que.push_back({row, col, 0});
    seen.insert({row, col});
    std::size_t best = 0;
    while (!que.empty()) {
        const auto pt = que.front();
        que.pop_front();
        const auto nbs = two_neighbors(grid, pt, start_replace);
        for (const auto &nb_pt : nbs) {
            const auto coord = std::make_pair(nb_pt.row, nb_pt.col);
            if (0 == seen.count(coord)) {
                que.push_back(nb_pt);
                seen.insert(coord);
                best = std::max(best, nb_pt.dist);
            }
        }
    }
    std::cout << best << '\n'; 
}


int main() {
    constexpr char start_replace = 'J';
    constexpr char path[] = "day_10.in";
    if (auto fs = std::ifstream(path)) {
        std::string line;
        std::vector<std::string> grid;
        while (std::getline(fs, line)) {
            grid.push_back(line);
        }
        solve(grid, start_replace);
    }
}