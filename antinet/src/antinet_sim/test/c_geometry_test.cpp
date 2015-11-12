#include "gtest/gtest.h"
#include "c_geometry.hpp"

TEST(c_geometry_test, simple_vertical) {
    t_geo_r x2 = 0,
            y2 = 4;
    t_geo_point A(0,0);
    t_geo_point B(x2,y2);
    t_geo_point C = c_geometry::point_on_line_between_part(A,B,1);
    EXPECT_EQ(C.x, 0);
    EXPECT_EQ(C.y, 4);
}

TEST(c_geometry_test, middle_between_points) {
    t_geo_r x2 = 3,
            y2 = 4;
    t_geo_point A(0,0);
    t_geo_point B(x2,y2);
    t_geo_point C = c_geometry::point_on_line_between_part(A,B,0.5);
    EXPECT_EQ(C.x, 1.5);
    EXPECT_EQ(C.y, 2);
}

TEST(c_geometry_test, distance_between_points) {
    t_geo_r x2,y2;
    for (int i = 0; i < 100; ++i) {
        x2 = y2 = i;
        t_geo_point A(0,0);
        t_geo_point B(x2,y2);
        t_geo_r dist = c_geometry::distance(A,B);
        // off-topic -- intresting std compraring
        //(std::abs(x-y) < std::numeric_limits<T>::epsilon())
        EXPECT_DOUBLE_EQ(dist, static_cast<t_geo_r>(i*sqrt(2)));
    }
}
