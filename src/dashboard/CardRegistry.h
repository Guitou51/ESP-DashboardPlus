#pragma once

#include "DashboardCard.h"
#include <initializer_list>
#include <map>

namespace espdashboardplus {
namespace detail {

// Owns cards and their group membership; it has no network responsibilities.
class CardRegistry {
public:
    ~CardRegistry();
    DashboardCard* add(DashboardCard* card);
    DashboardCard* get(const String& id) const;
    bool remove(const String& id);
    void addGroup(const String& id, const String& title);
    void addGroup(const String& id, const String& title, std::initializer_list<String> cards);
    void addCardToGroup(const String& groupId, const String& cardId);
    void removeCardFromGroup(const String& groupId, const String& cardId);
    void removeGroup(const String& id);
    const std::map<String, DashboardCard*>& cards() const;
    const std::vector<CardGroup>& groups() const;

private:
    std::map<String, DashboardCard*> _cards;
    std::vector<CardGroup> _groups;
};

} // namespace detail
} // namespace espdashboardplus
