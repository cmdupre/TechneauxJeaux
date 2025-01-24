#include "main.h"
#include "environment.h"
#include <iostream>
#include <string>
#include <thread>
#include <mariadb/conncpp.hpp>
#include "logger/Logger.hpp"
#include "bt/BLEHelper.hpp"
#include "jutta_bt_proto/CoffeeMaker.hpp"
#include "jutta_bt_proto/CoffeeMakerLoader.hpp"

int
main()
{
    while (true)
    {
        SPDLOG_INFO("Scanning...");

        bool cancelled = false;

        std::shared_ptr<bt::ScanArgs> result =
        bt::scan_for_device("TT214H BlueFrog", &cancelled);

        if (!result)
        {
            SPDLOG_INFO("No coffee maker found. Retry in 2 seconds...");
            std::this_thread::sleep_for(std::chrono::seconds{2});
            continue;
        }

        jutta_bt_proto::CoffeeMaker
        coffeeMaker(std::string{result->name}, std::string{result->addr});

        coffeeMaker.joeChangedEventHandler.append
        ([](const std::shared_ptr<jutta_bt_proto::Joe>& joe)
        {
            joe->alertsChangedEventHandler.append
            ([](const std::vector<const jutta_bt_proto::Alert*>& alerts)
            {
                for (const jutta_bt_proto::Alert* alert : alerts)
                    SPDLOG_INFO("New alert '{}' with type '{}'.", alert->name, alert->type);
            });
        });

        coffeeMaker.joeChangedEventHandler.append
        ([](const std::shared_ptr<jutta_bt_proto::Joe>& joe)
        {
            joe->productStatisticCountersChangedEventHandler.append
            ([](const std::shared_ptr<jutta_bt_proto::Joe>& joe)
            {
                for (const jutta_bt_proto::MaintenancePercentage& mp : joe->maintenancePercentages)
                    SPDLOG_INFO("Name: {}, Percentage: {}", mp.name, mp.percent);

                for (const jutta_bt_proto::MaintenanceCounter& mc : joe->maintenanceCounters)
                    SPDLOG_INFO("Name: {}, Count: {}", mc.name, mc.count);

                for (const jutta_bt_proto::Product& p : joe->products)
                    SPDLOG_INFO("Product Name: {}, Code: {}, StatCounter: {}", p.name, p.code, p.statCounter);

                /*
                sql::Driver* driver = sql::mariadb::get_driver_instance();
                sql::SQLString url("jdbc:mariadb://localhost:3306/jeaux");
                sql::Properties properties({{"user", ENVIRONMENT_DB_USER}, {"password", ENVIRONMENT_DB_PASS}});
                std::unique_ptr<sql::Connection> conn(driver->connect(url, properties));

                for (const jutta_bt_proto::Product& p : joe->products)
                {
                    std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO statistics (timestamp, product_name, counter) VALUES (CURRENT_TIMESTAMP, ?, ?)"));
                    stmnt->setString(1, p.name);
                    stmnt->setInt(2, p.statCounter);
                    stmnt->executeQuery();
                }
                */
            });
        });

        if (coffeeMaker.connect())
        {
            while (coffeeMaker.get_state() == jutta_bt_proto::CONNECTED)
            {
                coffeeMaker.request_statistics(jutta_bt_proto::StatParseMode::MAINTENANCE_COUNTER);
                coffeeMaker.request_statistics(jutta_bt_proto::StatParseMode::MAINTENANCE_PERCENT);
                coffeeMaker.request_statistics(jutta_bt_proto::StatParseMode::PRODUCT_COUNTERS_DAILY);

                SPDLOG_INFO("Waiting 5 seconds.");
                std::this_thread::sleep_for(std::chrono::seconds{5});
            }
        }

        coffeeMaker.disconnect();
        SPDLOG_INFO("Disconnected. Retry in 5 seconds...");
        std::this_thread::sleep_for(std::chrono::seconds{5});
    }
}