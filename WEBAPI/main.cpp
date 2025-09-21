#include "crow.h"
#include <sqlite3.h>
#include <iostream>

int main() {
    crow::SimpleApp app;
CROW_ROUTE(app, "/")([]() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::string html;
    std::string select;
    if (sqlite3_open("../DB/Cafeteria.db", &db) != SQLITE_OK) {
        return crow::response(500, "Failed to open database");
    }

    const char* sql = "SELECT Item, Name, Price FROM Menu;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return crow::response(500, "Failed to prepare SQL statement");
    }

    html = "<table border='1'><tr><th>Item</th><th>Name</th><th>Price</th></tr>";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int item        = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int price       = sqlite3_column_int(stmt, 2);

        html += "<tr><td>" + std::to_string(item) + "</td><td>" +
                std::string(name ? name : "") + "</td><td>" +
                std::to_string(price) + "</td></tr>";
    
        select += "<option value=" + std::to_string(item) + ">" + std::to_string(item) + "</option>";
            }

    html += "</table>";

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    auto page = crow::mustache::load("home.html");
    crow::mustache::context ctx({{"table", html}, {"options", select}});

    // wrap rendered template in crow::response
    return crow::response(page.render(ctx));
});
     CROW_ROUTE(app, "/order")([](const crow::request& req)
        {
            auto itemParam = req.url_params.get("Item");
            auto qtyParam  = req.url_params.get("qty");

            if (!itemParam || !qtyParam) {
                return crow::response(400, "Missing item or qty");
            }

            int item = std::stoi(itemParam);
            int qty  = std::stoi(qtyParam);

            sqlite3* db;
            sqlite3_stmt* stmt;
            int rc = sqlite3_open("../DB/Cafeteria.db", &db);
            if (rc) {
                return crow::response(500, "Cannot open database");
            }

            // Get price from Menu
            const char* sqlPrice = "SELECT Price FROM Menu WHERE Item = ?;";
            sqlite3_prepare_v2(db, sqlPrice, -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, item);

            int price = 0;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                price = sqlite3_column_int(stmt, 0);
            } else {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return crow::response(404, "Item not found");
            }
            sqlite3_finalize(stmt);

            int total = price * qty;

            // 2. Insert into Orders
            const char* sqlInsert = "INSERT INTO Orders (Item, Qty, TotalBill) VALUES (?, ?, ?);";
            sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, item);
            sqlite3_bind_int(stmt, 2, qty);
            sqlite3_bind_int(stmt, 3, total);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return crow::response(500, "Insert failed");
            }

            sqlite3_finalize(stmt);
            sqlite3_close(db);

            // 3. Response back
            return crow::response(
                "<!DOCTYPE html>"
                "<html>"
                "<head><title>Order Placed</title></head>"
                "<body>"
                "<script>"
                "alert('Order placed! Item " + std::to_string(item) +
                ", Qty " + std::to_string(qty) +
                ", Total = " + std::to_string(total) + "');"
                "window.location.href = '/';"
                "</script>"
                "</body>"
                "</html>"
            );
        });

        CROW_ROUTE(app, "/admin")([](){
            return crow::response(
            "<!DOCTYPE html>"
            "<html>"
            "<head><title>Order Placed</title></head>"
            "<body>"
            "<script>"
            "alert('Page is Underconstrunction.');"
            "window.location.href = '/';"
            "</script>"
            "</body>"
            "</html>");
        });

    app.port(80).multithreaded().run();
}
