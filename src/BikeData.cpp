#include "BikeData.h"

BikeData::BikeData(std::unique_ptr<BikeDataSource> dataSource)
    : dataSource(std::move(dataSource))
{
}

void BikeData::update()
{
    dataSource->update();
}