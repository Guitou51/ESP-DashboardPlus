#include "DropdownCard.h"

DropdownCardImpl::DropdownCardImpl(const String& id, const String& title, const String& placeholder)
    : DashboardCard(id, CardType::DROPDOWN, title), placeholder(placeholder) {}

void DropdownCardImpl::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    config["placeholder"] = placeholder;
    JsonArray optionsArr = config["options"].to<JsonArray>();
    for (const DropdownOption& opt : options) {
        JsonObject optObj = optionsArr.add<JsonObject>();
        optObj["value"] = opt.value;
        optObj["label"] = opt.label;
    }
    writeSize(config);
}

void DropdownCardImpl::handleAction(const String& action, JsonObject& data) {
    if (action == "change" && !data["value"].isNull()) {
        value = data["value"].as<String>();
        if (onChange) {
            onChange(value);
        }
    }
}

void DropdownCardImpl::addOption(const String& val, const String& label) {
    options.push_back({val, label});
}

void DropdownCardImpl::setValue(const String& val) {
    value = val;
}
