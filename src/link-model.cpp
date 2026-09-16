#include "link-model.hpp"

#include <utility>

namespace dibu {

bool LinkModel::link(std::string masterScene, std::string canvasScene)
{
  if (masterScene.empty() || canvasScene.empty())
    return false;

  links_.insert_or_assign(std::move(masterScene), std::move(canvasScene));
  return true;
}

bool LinkModel::unlink(const std::string &masterScene)
{
  return links_.erase(masterScene) != 0;
}

void LinkModel::clear()
{
  links_.clear();
}

std::optional<std::string> LinkModel::resolve(const std::string &masterScene) const
{
  const auto found = links_.find(masterScene);
  if (found == links_.end())
    return std::nullopt;
  return found->second;
}

std::vector<std::pair<std::string, std::string>> LinkModel::links() const
{
  return {links_.begin(), links_.end()};
}

std::size_t LinkModel::size() const noexcept
{
  return links_.size();
}

} // namespace dibu
