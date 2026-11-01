#pragma once

#include "../DashboardCard.h"

class DropdownCardImpl : public DashboardCard {
public:
    String value;
    String placeholder;
    std::vector<DropdownOption> options;
    DropdownCallback onChange;

    DropdownCardImpl(const String& id, const String& title, const String& placeholder = "Select...");

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void addOption(const String& val, const String& label);

    void setValue(const String& val);
};
