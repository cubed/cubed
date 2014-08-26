#pragma once

#include "geo.h"

Point::Point(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Point::Point(int x, int y, int z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Point::Point(const rapidjson::Value &v) {
    x = v[rapidjson::SizeType(0)].GetInt();
    y = v[rapidjson::SizeType(1)].GetInt();
    z = v[rapidjson::SizeType(2)].GetInt();
}