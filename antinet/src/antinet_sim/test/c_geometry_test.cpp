#include "gtest/gtest.h"
#include "c_geometry.hpp"

TEST(c_geometry_test, Test) {

    //t_geo_r x1,y1;
    //t_geo_r x2,y2;

    t_geo_point A(100,100);
    t_geo_point B(200,500);
    t_geo_point C = c_geometry::point_on_line_between_distance(A,B,0.5);
    EXPECT_TRUE(1);
}
