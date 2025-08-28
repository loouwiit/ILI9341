#pragma once

#include "spi.hpp"

bool mountSd(SPI& spi, GPIO cs);
void unmountSd();
