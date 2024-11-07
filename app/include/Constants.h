#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

#include <QColor>

/* Utility Functions */
constexpr inline int CONVERT_TO_DEFAULT_STEP(double value, double min, double max, int steps) {
    const double step = (max - min) / steps;
    return static_cast<int>((value - min) / step);
}

namespace UI_CONSTANTS {
    /* Element Sizes */
    static constexpr int DEFAULT_SLIDER_SIZE = 200;
    static constexpr int DEFAULT_ICON_SIZE = 24;

    /* Colors */
    static constexpr Qt::GlobalColor DEFAULT_PLAIN_COLOR = Qt::red;
    static constexpr Qt::GlobalColor DEFAULT_BEZIER_POINT_COLOR = Qt::blue;
    static constexpr Qt::GlobalColor DEFAULT_TRIANGLE_LINE_COLOR = Qt::darkRed;
    static constexpr Qt::GlobalColor DEFAULT_BEZIER_LINE_COLOR = Qt::darkGreen;

    /* Timing */
    static constexpr int DEFAULT_TOAST_DURATION_MS = 3000;
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
    static constexpr float DEFAULT_ALPHA = 0.0f;
    static constexpr float DEFAULT_BETA = 0.0f;
}

namespace LIGHTING_CONSTANTS {
    static constexpr float DEFAULT_KS = 0.5f;
    static constexpr float DEFAULT_KD = 0.5f;
    static constexpr int DEFAULT_M = 50;
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
        static constexpr double MIN = 0.0;
        static constexpr double MAX = 1.0;
        static constexpr int STEPS = 100;
        static constexpr int DEFAULT_STEP = CONVERT_TO_DEFAULT_STEP(
                0.5,
                MIN,
                MAX,
                STEPS
        );
    }
}


namespace RESOURCE_CONSTANTS {
    static constexpr const char* DEFAULT_CONTROL_POINTS_PATH = ":/data/example1";
    static constexpr const char* DEFAULT_TEXTURE_PATH = ":/data/test_texture.png";
    static constexpr int TEXTURE_IMAGE_SIZE = 1000;
}

/* Enums */
enum class FillType {
    SIMPLE_COLOR,
    TEXTURE
};

#endif /* APP_CONSTANTS_H */