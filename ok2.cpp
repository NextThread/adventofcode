#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <exception>
#include <utility>
#include <deque>
#include <set>
#include <algorithm>

enum class Direction
{
    NORTH,
    EAST,
    SOUTH,
    WEST
};

// todo: can probably infer this start_replace, though manually is easier
constexpr char start_replace = 'J';
constexpr char file_path[] = "/home/xdavidliu/Documents/temp/data.txt";
static bool left_handed = false;

// constexpr char start_replace = '7';
// constexpr char file_path[] = "/home/xdavidliu/Documents/temp/example.txt";
// static bool left_handed = true;

constexpr Direction initial_direction = Direction::SOUTH; // assumes vertical

using Point = std::pair<std::size_t, std::size_t>;

Point shift(const Point &pt, const Direction dir)
{
    switch (dir)
    {
    case Direction::NORTH:
        return {pt.first - 1, pt.second};
    case Direction::SOUTH:
        return {pt.first + 1, pt.second};
    case Direction::EAST:
        return {pt.first, pt.second + 1};
    case Direction::WEST:
        return {pt.first, pt.second - 1};
    }
}

Direction difference(const Point &to, const Point &from)
{
    if (to.first == from.first)
    {
        return to.second < from.second ? Direction::WEST : Direction::EAST;
    }
    else
    {
        return to.first < from.first ? Direction::NORTH : Direction::SOUTH;
    }
}

Direction interior_direction(const Direction move_dir)
{
    switch (move_dir)
    {
    case Direction::NORTH:
        return left_handed ? Direction::WEST : Direction::EAST;
    case Direction::SOUTH:
        return left_handed ? Direction::EAST : Direction::WEST;
    case Direction::EAST:
        return left_handed ? Direction::NORTH : Direction::SOUTH;
    case Direction::WEST:
        return left_handed ? Direction::SOUTH : Direction::NORTH;
    }
}

std::pair<std::size_t, std::size_t> find_start(const std::vector<std::string> &grid)
{
    for (std::size_t r = 0; r < grid.size(); ++r)
    {
        const auto &row = grid[r];
        for (std::size_t c = 0; c < row.size(); ++c)
        {
            if (row[c] == 'S')
            {
                return {r, c};
            }
        }
    }
    throw std::exception();
}

char replace(const char grid_ch)
{
    return grid_ch == 'S' ? start_replace : grid_ch;
}

std::vector<Point> two_neighbors(
    const std::vector<std::string> &grid, const Point &point)
{
    switch (replace(grid[point.first][point.second]))
    {
    case '-':
        return {shift(point, Direction::EAST), shift(point, Direction::WEST)};
    case '|':
        return {shift(point, Direction::NORTH), shift(point, Direction::SOUTH)};
    case 'L':
        return {shift(point, Direction::NORTH), shift(point, Direction::EAST)};
    case 'J':
        return {shift(point, Direction::NORTH), shift(point, Direction::WEST)};
    case '7':
        return {shift(point, Direction::SOUTH), shift(point, Direction::WEST)};
    case 'F':
        return {shift(point, Direction::SOUTH), shift(point, Direction::EAST)};
    default:
        throw std::exception();
    }
}

std::vector<Point> compute_path_order(const std::set<Point> &path_seen, const std::vector<std::string> &grid)
{
    const auto is_vertical = [&grid](const Point &pt)
    {
        return grid[pt.first][pt.second] == '|';
    };
    const auto found = std::find_if(path_seen.cbegin(), path_seen.cend(), is_vertical);
    if (found == path_seen.cend())
    {
        std::cout << "loop without verticals; don't know what to do\n";
        throw std::exception();
    }
    const auto initial_vertical = *found;
    std::vector<Point> path_order;
    path_order.push_back(initial_vertical);
    Point next = shift(initial_vertical, initial_direction);
    while (next != initial_vertical)
    { // terminates when loops back
        const auto prev = path_order.back();
        path_order.push_back(next);
        const auto nbs = two_neighbors(grid, next);
        next = nbs[0] == prev ? nbs[1] : nbs[0];
    }
    path_order.push_back(initial_vertical); // just to close the loop
    return path_order;
}

void bfs_bulk(
    const std::set<Point> &path_seen, const std::vector<std::string> &grid,
    std::set<Point> &bulk_seen, const Point &possible_bulk_point)
{
    constexpr Direction bulk_neighbors[] = {
        Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST};
    if (bulk_seen.count(possible_bulk_point) || path_seen.count(possible_bulk_point))
    {
        return;
    }
    std::deque<Point> que;
    que.push_back(possible_bulk_point);
    bulk_seen.insert(possible_bulk_point);
    while (!que.empty())
    {
        const auto point = que.front();
        que.pop_front();
        for (const auto dir : bulk_neighbors)
        {
            const auto nb = shift(point, dir);
            if (nb.first < 0 || nb.first >= grid.size() || nb.second < 0 || nb.second >= grid.front().size())
            {
                std::cout << "out of bounds; use opposite bool for left_handed\n";
                throw std::exception();
            }
            if (0 == path_seen.count(nb) && 0 == bulk_seen.count(nb))
            {
                que.push_back(nb);
                bulk_seen.insert(nb);
            }
        }
    }
}

void solve2(const std::set<Point> &path_seen, const std::vector<std::string> &grid)
{
    const auto path_order = compute_path_order(path_seen, grid);
    std::set<Point> bulk_seen;
    auto prev_move_dir = Direction::SOUTH; // garbage, will be overwritten in first iter;
    for (
        auto iter = path_order.cbegin();
        // need to omit final one because of move_dir calculation
        // final one assumed to be initial_vertical
        iter != path_order.cend() && iter + 1 != path_order.cend();
        ++iter)
    {
        const auto next_move_dir = difference(*(iter + 1), *iter);
        if (iter != path_order.cbegin() && prev_move_dir != next_move_dir)
        {
            // because otherwise tight u-turn looks down but it doesn't look left on before turning
            bfs_bulk(path_seen, grid, bulk_seen, shift(*iter, interior_direction(prev_move_dir)));
            ;
        }
        bfs_bulk(path_seen, grid, bulk_seen, shift(*iter, interior_direction(next_move_dir)));
        prev_move_dir = next_move_dir;
    }
    std::cout << "part 2 = " << bulk_seen.size() << '\n'; // 589
    // original sketch: go clockwise or counterclockwise around path
    // have an "interior direction" (but how is it maintained around corners?)
    // do bfs from each pipe; count total number seen
    // if reach outside grid, you know you're on wrong side
    // for 7, change from down to left and from up to right
    // note when going around corner may need to bfs BOTH dirs of a corner
}

void solve(const std::vector<std::string> &grid)
{
    std::size_t row, col;
    for (std::size_t r = 0; r < grid.size(); ++r)
    {
        const auto &roww = grid[r];
        for (std::size_t c = 0; c < roww.size(); ++c)
        {
            if (roww[c] == 'S')
            {
                row = r;
                col = c;
            }
        }
    }
    std::deque<std::pair<Point, std::size_t>> que;
    std::set<Point> seen;
    que.push_back({{row, col}, 0});
    seen.insert(Point{row, col});
    std::size_t best = 0;
    while (!que.empty())
    {
        // https://stackoverflow.com/q/25035691/2990344
        // interesting: you cannot pop without copying
        const auto pt = que.front();
        que.pop_front();
        const auto nbs = two_neighbors(grid, pt.first);
        for (const auto &nb_pt : nbs)
        {
            const Point coord = {nb_pt.first, nb_pt.second};
            if (0 == seen.count(coord))
            {
                const auto nb_dist = pt.second + 1;
                que.emplace_back(nb_pt, nb_dist);
                seen.insert(coord);
                best = std::max(best, nb_dist);
            }
        }
    }
    std::cout << "part 1 = " << best << '\n'; // 7063
    solve2(seen, grid);
}

int main()
{
    left_handed = left_handed; // hack to get rid of compiler warnings
    if (auto fs = std::ifstream(file_path))
    {
        std::string line;
        std::vector<std::string> grid;
        while (std::getline(fs, line))
        {
            grid.push_back(line);
        }
        solve(grid);
    }
}