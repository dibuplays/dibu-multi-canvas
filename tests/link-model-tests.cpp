#include "link-model.hpp"

#include <cassert>
#include <iostream>

int main()
{
  dibu::LinkModel links;
  assert(links.size() == 0);
  assert(!links.link("", "Vertical"));
  assert(!links.link("Gameplay", ""));
  assert(links.link("Gameplay", "Vertical - Gameplay"));
  assert(links.resolve("Gameplay") == "Vertical - Gameplay");
  assert(links.size() == 1);

  assert(links.link("Gameplay", "Vertical - Boss"));
  assert(links.resolve("Gameplay") == "Vertical - Boss");
  assert(links.size() == 1);

  assert(links.link("Chatting", "Vertical - Chatting"));
  assert(links.links().size() == 2);
  assert(links.unlink("Gameplay"));
  assert(!links.resolve("Gameplay"));
  assert(!links.unlink("Missing"));

  links.clear();
  assert(links.size() == 0);
  std::cout << "Dibu Multi-Canvas core tests passed\n";
  return 0;
}
