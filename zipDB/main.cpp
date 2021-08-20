﻿#include <string>
#include <iostream>

#include <pqxx/pqxx>
#include <pqxx/except.hxx>
#include <zlib.h>

#include "CConnection.h"
#include "tools.h"
#include "compress.h"

//что-то что возвращает worker и сохраняет connection // почти сделано
//исключение не должно завершать программу // нет
//вынести цикл // есть
//сделать что-то, что обрабатывает одну запись // есть
//обрабатывать по 10000 //есть

void execute(pqxx::work& worker, pqxx::work& zipWorker)
{
    //счетчик 
    zipWorker.conn().prepare("insert", "INSERT INTO t_event (type, subjects, timestamp, zip_event, ts_vector) VALUES($1, $2, $3, $4, $5);");
    int id = 0;
    while (true)
    {
        //сколько было
        //сколько стало
        
        std::string query = "SELECT * FROM t_event ORDER BY timestamp DESC LIMIT 10000 OFFSET " + std::to_string(id);
        pqxx::result res = worker.exec(query);

        if (res.empty())
        {
            break;
        }

        for (const auto& row : res)
        {
            try
            {
                executeOneNote(row, zipWorker);
            }
            catch (const std::exception&)
            {
                std::cerr << "";//
            }
        }
        zipWorker.commit();
        id += 10000;
    }
}

int main()
{
    std::string host = "localhost";
    std::string port = "5432";
    std::string dbname = "ngp";
    std::string user = "ngp";
    std::string zipDbName = "ngpNew";
    std::string password = "123456";
    try
    {
        CConnection conn(host, port, dbname, user, password);
        pqxx::work worker = conn.getWorker();

        CConnection zipConn(host, port, zipDbName, user, password);
        pqxx::work zipWorker = zipConn.getWorker();

        execute(worker, zipWorker);
        zipWorker.commit();
    }
    catch (const std::exception& exc)
    {
        std::cerr << exc.what();
    }
    return 0;
}
