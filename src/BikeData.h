#pragma once
#include "BikeDataSource.h"
#include <Arduino.h>
#include <memory>

class BikeData
{
public:
    BikeData(std::unique_ptr<BikeDataSource> dataSource);
    void update();
    BikeDataSource::Data getData() const { return dataSource->getData(); }

private:
    std::unique_ptr<BikeDataSource> dataSource;
};