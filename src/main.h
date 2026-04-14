#ifndef _MAIN_H
#define _MAIN_H

#include "jutta_bt_proto/CoffeeMakerLoader.hpp"
#include "mariadb/conncpp.hpp"

class Main
{
public:
    void Geaux();
    static void JoeChanged(const std::shared_ptr<jutta_bt_proto::Joe>& joe);
    static void AlertsChanged(const std::vector<const jutta_bt_proto::Alert*>& alerts);
    static void ProductStatisticCountersChanged(const std::shared_ptr<jutta_bt_proto::Joe>& joe);
    static std::unique_ptr<sql::Connection> GetDbConnection();
    static void SetConnectionStatus(bool status);
    static void TruncateOrders(void);
};

#endif//_MAIN_H