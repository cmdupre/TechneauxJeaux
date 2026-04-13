#include "main.h"
#include "environment.h"
#include "logger/Logger.hpp"
#include "bt/BLEHelper.hpp"
#include "jutta_bt_proto/CoffeeMaker.hpp"

int main()
{
    Main mainLoop;
    mainLoop.Geaux();
}

void Main::Geaux()
{
    while (true)
    {
        SPDLOG_INFO("Scanning...");

        bool cancelled = false;

        std::shared_ptr<bt::ScanArgs> result = 
            bt::scan_for_device("TT214H BlueFrog", &cancelled);

        if (!result)
        {
            SPDLOG_INFO("bt::scan_for_device() failed, retry in 2 seconds...");
            std::this_thread::sleep_for(std::chrono::seconds{2});
            continue;
        }

        jutta_bt_proto::CoffeeMaker coffeeMaker(std::string{result->name}, std::string{result->addr});

        coffeeMaker.joeChangedEventHandler.append(JoeChanged);

        SPDLOG_INFO("Connecting...");

        if (!coffeeMaker.connect())
        {
            SPDLOG_INFO("connect() failed, retry in 2 seconds...");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        std::unique_ptr<sql::Connection> conn = GetDbConnection();
        std::unique_ptr<sql::Statement> stmnt(conn->createStatement());

        SPDLOG_INFO("Entering application loop.");

        while (coffeeMaker.get_state() == jutta_bt_proto::CONNECTED)
        {
            SPDLOG_INFO("Requesting statistics...");
            coffeeMaker.request_statistics(jutta_bt_proto::StatParseMode::MAINTENANCE_COUNTER);
            coffeeMaker.request_statistics(jutta_bt_proto::StatParseMode::MAINTENANCE_PERCENT);
            coffeeMaker.request_statistics(jutta_bt_proto::StatParseMode::PRODUCT_COUNTERS_DAILY);

            unsigned short i = 60;
            while (i-- > 0)
            {
                std::unique_ptr<sql::ResultSet> res(stmnt->executeQuery("select * from orders"));
                if (res->next()) // only want the first record
                {
                    uint8_t strength = static_cast<uint8_t>(res->getUInt(1));
                    char strength_str[3];
                    sprintf(strength_str, "%02X", strength);

                    uint8_t amount = static_cast<uint8_t>(res->getUInt(2));
                    char amount_str[4];
                    sprintf(amount_str, "%d", amount);

                    if (strength > 0 && amount > 0)
                    {
                        jutta_bt_proto::Product coffee = coffeeMaker.get_joe()->products[2];

                        jutta_bt_proto::Product custom_coffee(
                            std::move(coffee.name), 
                            std::move(coffee.code),
                            std::make_optional<jutta_bt_proto::ItemsOption>(std::move(coffee.strength->argument), std::move(strength_str), std::move(coffee.strength->items)),
                            std::move(coffee.temperature),
                            std::make_optional<jutta_bt_proto::MinMaxOption>(std::move(coffee.waterAmount->argument), amount, coffee.waterAmount->min, coffee.waterAmount->max, coffee.waterAmount->step),
                            std::move(coffee.milkFoamAmount));

                        SPDLOG_INFO("Requesting coffee...");
                        SPDLOG_INFO(strength_str);
                        SPDLOG_INFO(amount_str);
                        coffeeMaker.request_coffee(custom_coffee);
                    }

                    SPDLOG_INFO("Truncating orders table.");
                    stmnt->executeQuery("truncate table orders");
                    break;
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        SPDLOG_INFO("Disconnected.");
    }
}

void Main::JoeChanged(const std::shared_ptr<jutta_bt_proto::Joe>& joe)
{
    joe->alertsChangedEventHandler.append(AlertsChanged);
    joe->productStatisticCountersChangedEventHandler.append(ProductStatisticCountersChanged);
}

void Main::AlertsChanged(const std::vector<const jutta_bt_proto::Alert*>& alerts)
{
    std::unique_ptr<sql::Connection> conn = GetDbConnection();

    for (const jutta_bt_proto::Alert* alert : alerts)
    {
        SPDLOG_INFO("Writing new alert to database: name '{}' with type '{}'.", alert->name, alert->type);
        std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO alerts (timestamp, name, type) VALUES (CURRENT_TIMESTAMP(3), ?, ?)"));
        stmnt->setString(1, alert->name);
        stmnt->setString(2, alert->type);
        stmnt->executeQuery();
    }
}

void Main::ProductStatisticCountersChanged(const std::shared_ptr<jutta_bt_proto::Joe>& joe)
{
    std::unique_ptr<sql::Connection> conn = GetDbConnection();

    SPDLOG_INFO("Writing maintenance percentages to database.");
    for (const jutta_bt_proto::MaintenancePercentage& mp : joe->maintenancePercentages)
    {
        std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement(
            "INSERT INTO maintenancePercentages (name, percentage, timestamp) VALUES (?, ?, CURRENT_TIMESTAMP(3)) ON DUPLICATE KEY UPDATE name=VALUES(name), percentage=VALUES(percentage), timestamp=CURRENT_TIMESTAMP(3)"));

        stmnt->setString(1, mp.name);
        stmnt->setUInt(2, mp.percent);
        stmnt->executeQuery();
    }

    SPDLOG_INFO("Writing maintenance counters to database.");
    for (const jutta_bt_proto::MaintenanceCounter& mc : joe->maintenanceCounters)
    {
        std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement(
            "INSERT INTO maintenanceCounters (name, count, timestamp) VALUES (?, ?, CURRENT_TIMESTAMP(3)) ON DUPLICATE KEY UPDATE name=VALUES(name), count=VALUES(count), timestamp=CURRENT_TIMESTAMP(3)"));

        stmnt->setString(1, mc.name);
        stmnt->setUInt(2, mc.count);
        stmnt->executeQuery();
    }

    SPDLOG_INFO("Writing product counters to database.");
    for (const jutta_bt_proto::Product& p : joe->products)
    {
        std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement(
            "INSERT INTO productCounters (name, code, count, timestamp) VALUES (?, ?, ?, CURRENT_TIMESTAMP(3)) ON DUPLICATE KEY UPDATE name=VALUES(name), code=VALUES(code), count=VALUES(count), timestamp=CURRENT_TIMESTAMP(3)"));

        stmnt->setString(1, p.name);
        stmnt->setString(2, p.code);
        stmnt->setUInt(3, p.statCounter);
        stmnt->executeQuery();
    }
}

std::unique_ptr<sql::Connection> Main::GetDbConnection()
{
    sql::SQLString url("jdbc:mariadb://localhost:3306/jeaux");
    sql::Properties properties({{"user", ENVIRONMENT_DB_USER}, {"password", ENVIRONMENT_DB_PASS}});
    sql::Driver* driver = sql::mariadb::get_driver_instance();
    return std::unique_ptr<sql::Connection>(driver->connect(url, properties));
}