#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dibu {

class LinkModel {
public:
  bool link(std::string masterScene, std::string canvasScene);
  bool unlink(const std::string &masterScene);
  void clear();

  [[nodiscard]] std::optional<std::string> resolve(const std::string &masterScene) const;
  [[nodiscard]] std::vector<std::pair<std::string, std::string>> links() const;
  [[nodiscard]] std::size_t size() const noexcept;

private:
  std::map<std::string, std::string> links_;
};

} // namespace dibu
