#include "InputCard.h"

InputCard::InputCard(const String& id, const String& title, const String& placeholder)
    : DashboardCard(id, CardType::DATA_INPUT, title), placeholder(placeholder), inputType("text"), min(0), max(100),
      step(1) {}

void InputCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    config["placeholder"] = placeholder;
    config["inputType"] = inputType;
    config["unit"] = unit;
    if (inputType == "number") {
        config["min"] = min;
        config["max"] = max;
        config["step"] = step;
    }
    writeSize(config);
}

void InputCard::handleAction(const String& action, JsonObject& data) {
    if (!data["value"].isNull()) {
        value = data["value"].as<String>();
        if (action == "change" && onChange) {
            onChange(value);
        } else if (action == "submit" && onSubmit) {
            onSubmit(value);
        }
    }
}

void InputCard::setNumberInput(float minVal, float maxVal, float stepVal, const String& unitStr) {
    inputType = "number";
    min = minVal;
    max = maxVal;
    step = stepVal;
    unit = unitStr;
}

void InputCard::setValue(const String& val) {
    value = val;
}
