#include "WinSock2.h"
#include "Windows.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <shellapi.h>
#include "crow.h"
#include "crow/middlewares/cors.h"
#include "html.h"
#include <synchapi.h>
#include "json.hpp"

using namespace std;

std::string key;
bool start = false;
using json = nlohmann::json;
bool example_config = false;

// Auto login attempt
bool login() {
   /* Your login system*/
    return false;
}

void verify(const crow::json::rvalue& body) {
    if (body.has("license")) {
        key = body["license"].s();
    }
}

int main() {
    bool loginresult = login();
    crow::App<crow::CORSHandler> app;
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("http://localhost")
        .allow_credentials()
        .headers(
            "Accept",
            "Origin",
            "Content-Type",
            "Authorization",
            "Refresh",
            "X-Requested-With"
        )
        .methods(
            crow::HTTPMethod::Get,
            crow::HTTPMethod::Post,
            crow::HTTPMethod::Options,
            crow::HTTPMethod::Head,
            crow::HTTPMethod::Put,
            crow::HTTPMethod::Delete
        );

    CROW_ROUTE(app, "/")([](const crow::request&, crow::response& res) {
        res.write(html__main);
        res.end();
        });

    CROW_ROUTE(app, "/loginpage")([](const crow::request&, crow::response& res) {
        res.write(html__main_login);  // Sua página HTML inline
        res.end();
        });

    CROW_ROUTE(app, "/get_config").methods(crow::HTTPMethod::Get)([](const crow::request&, crow::response& res) {
        json settingsJson = {
            {"example_config", true},
        };

        res.write(settingsJson.dump());
        res.end();
        });

    CROW_ROUTE(app, "/save_config").methods(crow::HTTPMethod::Post)([](const crow::request& req, crow::response& res) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                res.code = 400;
                res.write("{\"success\": false, \"message\": \"Invalid JSON\"}");
                res.end();
                return;
            }

            example_config = body["example_config"].b();
           // config.saveconfig();
            res.write("{\"success\": true}");
            res.end();
        }
        catch (const std::exception& e) {
            res.code = 500;
            res.write("{\"success\": false, \"message\": \"Internal Server Error\"}");
            res.end();
        }
        });

    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::Post)([&](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                return crow::response(400, "Invalid JSON");
            }
            verify(body);
            start = true;
            return crow::response(200, "{\"success\": true}");
        }
        catch (const std::exception& e) {
            return crow::response(500, "Internal Server Error");
        }
        });
    crow::logger::setLogLevel(crow::LogLevel::Critical); // disable useless messages
    auto _a = app.port(80).bindaddr("127.0.0.1").run_async();

    if (!loginresult) {
        ShellExecute(0, 0, L"http:/localhost/loginpage", 0, 0, SW_SHOW);
        while (!start) {
            std::this_thread::sleep_for(chrono::seconds(5));
        }
        //try logging here
        
        /*if (!sucess)
        {
            app.stop();
            exit(0);
        }*/
    }
    cout << "logged in!" << endl;
    ShellExecute(0, 0, L"http:/localhost/", 0, 0, SW_SHOW);
    while (true) {
        cout << example_config << endl;
        Sleep(1000);
    }
    app.stop(); // its a MUST
    (exit)(0);
    return 0;

}