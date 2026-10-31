#include "CardRegistry.h"
#include <algorithm>

namespace espdashboardplus {
namespace detail {

CardRegistry::~CardRegistry() {
    for (auto& entry : _cards)
        delete entry.second;
}

DashboardCard* CardRegistry::add(DashboardCard* card) {
    _cards[card->id] = card;
    return card;
}

DashboardCard* CardRegistry::get(const String& id) const {
    const auto found = _cards.find(id);
    return found == _cards.end() ? nullptr : found->second;
}

bool CardRegistry::remove(const String& id) {
    auto found = _cards.find(id);
    if (found == _cards.end())
        return false;
    delete found->second;
    _cards.erase(found);
    return true;
}

void CardRegistry::addGroup(const String& id, const String& title) {
    _groups.emplace_back(id, title);
}

void CardRegistry::addGroup(const String& id, const String& title, std::initializer_list<String> cards) {
    CardGroup group(id, title);
    group.cardIds.assign(cards.begin(), cards.end());
    _groups.push_back(group);
}

void CardRegistry::addCardToGroup(const String& groupId, const String& cardId) {
    for (auto& group : _groups) {
        if (group.id == groupId) {
            group.cardIds.push_back(cardId);
            return;
        }
    }
}

void CardRegistry::removeCardFromGroup(const String& groupId, const String& cardId) {
    for (auto& group : _groups) {
        if (group.id == groupId) {
            group.cardIds.erase(std::remove(group.cardIds.begin(), group.cardIds.end(), cardId), group.cardIds.end());
            return;
        }
    }
}

void CardRegistry::removeGroup(const String& id) {
    _groups.erase(
        std::remove_if(_groups.begin(), _groups.end(), [&id](const CardGroup& group) { return group.id == id; }),
        _groups.end());
}

const std::map<String, DashboardCard*>& CardRegistry::cards() const {
    return _cards;
}
const std::vector<CardGroup>& CardRegistry::groups() const {
    return _groups;
}

} // namespace detail
} // namespace espdashboardplus
