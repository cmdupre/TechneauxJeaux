#ifndef _MAIN_H
#define _MAIN_H

#include "jutta_bt_proto/CoffeeMakerLoader.hpp"

class Main
{
public:
    void Geaux();
    static void JoeChanged(const std::shared_ptr<jutta_bt_proto::Joe>& joe);
    static void AlertsChanged(const std::vector<const jutta_bt_proto::Alert*>& alerts);
    static void ProductStatisticCountersChanged(const std::shared_ptr<jutta_bt_proto::Joe>& joe);
};

#endif//_MAIN_H