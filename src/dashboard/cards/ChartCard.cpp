#include "ChartCard.h"

ChartCard::ChartCard(const String& id, const String& title, ChartType type, int maxPoints)
    : DashboardCard(id, CardType::CHART, title), chartType(type), maxDataPoints(maxPoints) {}

void ChartCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["chartType"] = chartTypeToString(chartType);
    config["color"] = variantToString(variant);
    writeSize(config);

    // Multi-series data
    if (series.size() > 0) {
        JsonArray seriesArr = config["series"].to<JsonArray>();
        for (const ChartSeries& s : series) {
            JsonObject seriesObj = seriesArr.add<JsonObject>();
            seriesObj["name"] = s.name;
            seriesObj["color"] = s.color;
            JsonArray dataArr = seriesObj["data"].to<JsonArray>();
            for (float val : s.data) {
                dataArr.add(val);
            }
        }
    } else {
        // Legacy single-series format
        JsonArray dataArr = config["data"].to<JsonArray>();
        for (float val : data) {
            dataArr.add(val);
        }
    }
}

int ChartCard::addSeries(const String& name, const String& color) {
    series.push_back(ChartSeries(name, color));
    return series.size() - 1;
}

void ChartCard::addDataPoint(int seriesIndex, float val) {
    if (seriesIndex < 0 || seriesIndex >= (int)series.size())
        return;
    series[seriesIndex].data.push_back(val);
    if (series[seriesIndex].data.size() > maxDataPoints) {
        series[seriesIndex].data.erase(series[seriesIndex].data.begin());
    }
}

void ChartCard::addDataPoint(float val) {
    data.push_back(val);
    if (data.size() > maxDataPoints) {
        data.erase(data.begin());
    }
}

void ChartCard::clearSeries(int seriesIndex) {
    if (seriesIndex >= 0 && seriesIndex < (int)series.size()) {
        series[seriesIndex].data.clear();
    }
}

void ChartCard::clearAllSeries() {
    for (auto& s : series) {
        s.data.clear();
    }
}

void ChartCard::setChartType(ChartType t) {
    chartType = t;
}

void ChartCard::clearData() {
    data.clear();
}
