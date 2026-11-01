#pragma once

#include "../DashboardCard.h"

class ChartCard : public DashboardCard {
public:
    std::vector<ChartSeries> series;
    ChartType chartType;
    int maxDataPoints;

    // Legacy single-series data (for backwards compatibility)
    std::vector<float> data;

    ChartCard(const String& id, const String& title, ChartType type = ChartType::LINE, int maxPoints = 20);

    void toJson(JsonObject& card) override;

    // Add a new series to the chart
    int addSeries(const String& name, const String& color = "primary");

    // Add data point to a specific series
    void addDataPoint(int seriesIndex, float val);

    // Legacy: Add data point to single-series (backwards compatible)
    void addDataPoint(float val);

    // Clear a specific series
    void clearSeries(int seriesIndex);

    // Clear all series
    void clearAllSeries();

    void setChartType(ChartType t);

    void clearData();
};
