#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

#include <QColor>

/* Utility Functions */
constexpr inline int CONVERT_TO_DEFAULT_STEP(const double value, const double min, const double max, const int steps) {
    const double step = (max - min) / steps;
    return static_cast<int>((value - min) / step);
}

namespace UI_CONSTANTS {
    /* Element Sizes */
    static constexpr int DEFAULT_SLIDER_SIZE = 200;
    static constexpr int DEFAULT_ICON_SIZE = 24;
    static constexpr int DEFAULT_LIGHT_MOVE_RADIUS = 600;
    static constexpr int DEFAULT_LIGHT_SOURCE_RADIUS = 15;

    /* Colors */
    static constexpr Qt::GlobalColor DEFAULT_PLAIN_COLOR = Qt::red;
    static constexpr Qt::GlobalColor DEFAULT_BEZIER_POINT_COLOR = Qt::blue;
    static constexpr Qt::GlobalColor DEFAULT_TRIANGLE_LINE_COLOR = Qt::darkRed;
    static constexpr Qt::GlobalColor DEFAULT_BEZIER_LINE_COLOR = Qt::darkGreen;
    static constexpr Qt::GlobalColor LIGHT_SOURCE_COLOR = Qt::yellow;

    /* Timing */
    static constexpr int DEFAULT_TOAST_DURATION_MS = 3000;

    /* others */
    static constexpr bool DEFAULT_USE_TEXTURE = false;
    static constexpr bool DEFAULT_DRAW_NET = true;
    static constexpr bool DEFAULT_PLAY_ANIMATION = false;
}

namespace BEZIER_CONSTANTS {
    static constexpr size_t CONTROL_POINTS_COUNT = 16;
    static constexpr size_t BERNSTEIN_TABLE_SIZE = 4;
    static constexpr size_t CONTROL_POINTS_MATRIX_SIZE = 4;
    static constexpr double DEFAULT_POINT_RADIUS = 15.0;
}

namespace VIEW_SETTINGS {
    static constexpr int DEFAULT_TRIANGLE_ACCURACY = 6;
    static constexpr int DEFAULT_OBSERVER_DISTANCE = 1500;
    static constexpr int DEFAULT_LIGHT_Z = 1000;
    static constexpr float DEFAULT_ALPHA = 0.0f;
    static constexpr float DEFAULT_BETA = 0.0f;
}

namespace LIGHTING_CONSTANTS {
    static constexpr float DEFAULT_KS = 0.5f;
    static constexpr float DEFAULT_KD = 0.5f;
    static constexpr int DEFAULT_M = 50;
    static constexpr int ANIMATION_TIME_STEP_MS = 50;
    static constexpr float LIGHT_SPEED = 0.1f;
    static constexpr float LIGHT_MOVEMENT_STEP = (ANIMATION_TIME_STEP_MS / 1000.0) * LIGHT_SPEED;
    static constexpr Qt::GlobalColor DEFAULT_LIGHT_COLOR = Qt::white;
    static constexpr float NUMBER_OF_SPIRALS = 5.0f;
}

namespace SLIDER_CONSTANTS {
    namespace TRIANGULATION {
        static constexpr double MIN = 2.0;
        static constexpr double MAX = 90.0;
        static constexpr int STEPS = 88;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            VIEW_SETTINGS::DEFAULT_TRIANGLE_ACCURACY,
            MIN,
            MAX,
            STEPS
        );
    }

    namespace OBSERVER {
        static constexpr double MIN = 1000.0;
        static constexpr double MAX = 5000.0;
        static constexpr int STEPS = 400;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            VIEW_SETTINGS::DEFAULT_OBSERVER_DISTANCE,
            MIN,
            MAX,
            STEPS
        );
    }

    namespace ALPHA {
        static constexpr double MIN = -45.0;
        static constexpr double MAX = 45.0;
        static constexpr int STEPS = 900;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            VIEW_SETTINGS::DEFAULT_ALPHA,
            MIN,
            MAX,
            STEPS
        );
    }

    namespace BETA {
        static constexpr double MIN = 0.0;
        static constexpr double MAX = 10.0;
        static constexpr int STEPS = 100;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            VIEW_SETTINGS::DEFAULT_BETA,
            MIN,
            MAX,
            STEPS
        );
    }

    namespace KS {
        static constexpr double MIN = 0.0;
        static constexpr double MAX = 1.0;
        static constexpr int STEPS = 100;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            LIGHTING_CONSTANTS::DEFAULT_KS,
            MIN,
            MAX,
            STEPS
        );
    }

    namespace KD {
        static constexpr double MIN = 0.0;
        static constexpr double MAX = 1.0;
        static constexpr int STEPS = 100;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            LIGHTING_CONSTANTS::DEFAULT_KD,
            MIN,
            MAX,
            STEPS
        );
    }

    namespace M {
        static constexpr double MIN = 1.0;
        static constexpr double MAX = 100.0;
        static constexpr int STEPS = 99;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            LIGHTING_CONSTANTS::DEFAULT_M,
            MIN,
            MAX,
            STEPS
        );
    }

    namespace LIGHT_POSITION {
        static constexpr double MIN = 100.0;
        static constexpr double MAX = 10000.0;
        static constexpr int STEPS = static_cast<int>((MAX - MIN) / 10.0);
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
            VIEW_SETTINGS::DEFAULT_LIGHT_Z,
            MIN,
            MAX,
            STEPS
        );
    }
}


namespace RESOURCE_CONSTANTS {
    static constexpr const char *DEFAULT_CONTROL_POINTS_PATH = ":/data/example1";
    static constexpr const char *DEFAULT_TEXTURE_PATH = ":/data/test_texture.png";
    static constexpr int TEXTURE_IMAGE_SIZE = 1000;
}

/* Enums */
enum class FillType {
    SIMPLE_COLOR,
    TEXTURE
};

#endif /* APP_CONSTANTS_H */
