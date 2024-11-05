//
// Created by Jlisowskyy on 11/04/24.
//

#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

/* external includes */
#include <QColor>

// UI Element Sizes
static constexpr int DEFAULT_SLIDER_SIZE = 200;
static constexpr int DEFAULT_ICON_SIZE = 24;

// Colors
static constexpr Qt::GlobalColor DEFAULT_PLAIN_COLOR = Qt::red;
static constexpr Qt::GlobalColor DEFAULT_BEZIER_POINT_COLOR = Qt::blue;

// Bezier Configuration
static constexpr size_t CONTROL_POINTS_COUNT = 16;
static constexpr int DEFAULT_BEZIER_POINT_RADIUS = 5;

// UI Timing
static constexpr int DEFAULT_TOAST_DURATION_MS = 3000;

// Camera/View Settings
static constexpr double DEFAULT_OBSERVER_DISTANCE = 10.0;

// Slider Ranges and Defaults
static constexpr double TRIANGULATION_SLIDER_MIN = 1.0;
static constexpr double TRIANGULATION_SLIDER_MAX = 30.0;
static constexpr int TRIANGULATION_SLIDER_STEPS = 29;
static constexpr double TRIANGULATION_SLIDER_DEFAULT = 5.0;

static constexpr double OBSERVER_DISTANCE_MIN = 5.0;
static constexpr double OBSERVER_DISTANCE_MAX = 100.0;
static constexpr int OBSERVER_DISTANCE_STEPS = 95;

static constexpr double ALPHA_ANGLE_MIN = -45.0;
static constexpr double ALPHA_ANGLE_MAX = 45.0;
static constexpr int ALPHA_ANGLE_STEPS = 900;
static constexpr double ALPHA_ANGLE_DEFAULT = 450.0;

static constexpr double BETA_ANGLE_MIN = 0.0;
static constexpr double BETA_ANGLE_MAX = 10.0;
static constexpr int BETA_ANGLE_STEPS = 100;
static constexpr double BETA_ANGLE_DEFAULT = 0.0;

// Lightning Constants
static constexpr double LIGHTING_COEFFICIENT_MIN = 0.0;
static constexpr double LIGHTING_COEFFICIENT_MAX = 1.0;
static constexpr int LIGHTING_COEFFICIENT_STEPS = 100;
static constexpr double LIGHTING_COEFFICIENT_DEFAULT = 50.0;

static constexpr double M_COEFFICIENT_MIN = 1.0;
static constexpr double M_COEFFICIENT_MAX = 100.0;
static constexpr int M_COEFFICIENT_STEPS = 99;
static constexpr double M_COEFFICIENT_DEFAULT = 50.0;

#endif //APP_CONSTANTS_H