#include "location_map.hpp"
#include "hmm/location.hpp"
#include <limits>
#include <mutex>
#include <queue>

namespace rxy {
static constexpr int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
static constexpr int dy[] = {1, 0, -1, 0, 1, -1, 1, -1};

static std::mutex mtx;

void LocationMap::compute_distance() const {
    if (computed) return;
    std::lock_guard<std::mutex> lk(mtx);
    if (computed) return;

    std::cout << "compute distance start" << std::endl;
    
    const double sqrt2 =
        sqrt(x_step_ext * x_step_ext + y_step_ext * y_step_ext);
    const double edge[] = {y_step_ext, x_step_ext, y_step_ext, x_step_ext,
                           sqrt2,      sqrt2,      sqrt2,      sqrt2};

    std::vector<std::vector<std::vector<std::vector<double>>>> dist(
        m_ext,
        std::vector<std::vector<std::vector<double>>>(
            n_ext,
            std::vector<std::vector<double>>(
                m_ext, std::vector<double>(
                           n_ext, std::numeric_limits<double>::infinity()))));

    auto dd = GetConfig().d0;
    {
        std::vector<std::vector<bool>> computed(
            m_ext, std::vector<bool>(n_ext, false));
        std::vector<std::vector<bool>> vis(m_ext,
                                           std::vector<bool>(n_ext, false));
        std::queue<std::pair<int, int>> q;
        for (int i = 0; i < m_ext; ++i) {
            for (int j = 0; j < n_ext; ++j) {
                if (!ext_map[i][j])
                    continue;
                for (auto &&tt : vis)
                    std::fill(tt.begin(), tt.end(), false);
                vis[i][j] = true;
                q.emplace(i, j);
                auto &d = dist[i][j];
                d[i][j] = 0;
                while (!q.empty()) {
                    auto [a, b] = q.front();
                    q.pop();
                    auto d0 = d[a][b];
                    if (d0 > dd) {
                        continue;
                    }
                    for (int k = 0; k < 8; ++k) {
                        int x = a + dx[k], y = b + dy[k];
                        if (x >= 0 && x < m_ext && y >= 0 && y < n_ext &&
                            ext_map[x][y] && !vis[x][y] && !computed[x][y]) {
                            vis[x][y] = true;
                            d[x][y] = dist[x][y][i][j] = d0 + edge[k];
                            q.emplace(x, y);
                        }
                    }
                }
                computed[i][j] = true;
            }
        }
    }

    for (size_t i = 0; i < m_ext; ++i) {
        for (size_t j = 0; j < n_ext; ++j) {
            // std::cout << i << ',' << j << '\n';
            auto x = ext_map[i][j];
            if (!x)
                continue;
            auto &dmx = dist_map[x];
            auto const &d = dist[i][j];
            for (size_t a = 0; a < m_ext; ++a) {
                for (size_t b = 0; b < n_ext; ++b) {
                    auto y = ext_map[a][b];
                    if (!y || d[a][b] > dd)
                        continue;
                    dmx[y] = d[a][b];
                }
            }
        }
    }
    std::cout << "compute_distance()" << std::endl;
    computed = true;
}

} // namespace rxy
