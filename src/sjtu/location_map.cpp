#include "location_map.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

namespace rxy {
static constexpr int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
static constexpr int dy[] = {1, 0, -1, 0, 1, -1, 1, -1};

void LocationMap::compute_distance() const {

    const double sqrt2 = sqrt(x_step_ext * x_step_ext + y_step_ext * y_step_ext);
    const double dist[] = {y_step_ext, x_step_ext, y_step_ext, x_step_ext,
                           sqrt2,      sqrt2,      sqrt2,      sqrt2};

    using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, LocationPtr,
                                          boost::property<boost::edge_weight_t, Point::value_type>>;

    auto V = ext_set.size();
#ifdef DEBUG
    std::cout << "start build graph ...\n V = " << V << std::endl;
#endif

    graph_t g(V);
    std::unordered_map<LocationPtr, int> loc2vertex;
    loc2vertex.reserve(V);
    {
        int i = 0;
        for (auto&& loc : ext_set) {
            loc2vertex[loc] = i;
            g[i++] = loc;
        }
    }

#ifdef DEBUG
    std::cout << "vertices done." << std::endl;
#endif

    for (int x = 0; x < m_ext; ++x) {
        for (int y = 0; y < n_ext; ++y) {
            if (!ext_map[x][y]) continue;
            int u = loc2vertex[ext_map[x][y]];
            for (int k = 0; k < 8; ++k) {
                int nx = x + dx[k], ny = y + dy[k];
                if (nx < 0 || nx >= m_ext || ny < 0 || ny >= n_ext || !ext_map[nx][ny])
                    continue;
                int v = loc2vertex[ext_map[nx][ny]];
                boost::add_edge(u, v, dist[k], g);
            }
        }
    }

#ifdef DEBUG
    std::cout << "build graph DONE. \n start johnson algorithm..." << std::endl;
#endif

    std::vector<std::vector<double>> dist_matrix(
        V, std::vector<double>(V, std::numeric_limits<double>::infinity()));
    boost::johnson_all_pairs_shortest_paths(g, dist_matrix);
    for (size_t i = 0; i < V; ++i) {
        for (size_t j = 0; j < V; ++j) {
            if (dist_matrix[i][j] == std::numeric_limits<double>::infinity()) continue;
            auto loc1 = g[i], loc2 = g[j];
            dist_map[loc1][loc2] = dist_matrix[i][j];
        }
    }
#ifdef DEBUG
    std::cout << "johnson algorithm DONE." << std::endl;
#endif
}

}  // namespace rxy
