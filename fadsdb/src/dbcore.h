/*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements. See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership. The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied. See the License for the
* specific language governing permissions and limitations
* under the License.
*/

#ifndef _SOCE_DBCORE_MhSyTqSivP_H_
#define _SOCE_DBCORE_MhSyTqSivP_H_

#include <cstdint>
#include <thread>
#include <mutex>
#include <atomic>
#include "fads-db.h"
#include "table-creater.h"
#include "status.h"
#include "utils/dispatch-queue.hpp"
#include "log4rel/logger.h"
#include "log4rel/thread-sink.h"

namespace soce{
namespace fadsdb{

    class DbCore
    {
    public:
        using SqlData = struct SqlData{
        SqlData(int64_t req_id, int32_t cmd_type, const std::string& key, std::string&& data)
        : req_id_(req_id),
                cmd_type_(cmd_type),
                key_(key),
                data_(std::move(data))
                {
                }

        SqlData(int64_t req_id, int32_t cmd_type, const std::string& key, const char* data, size_t len)
        : req_id_(req_id),
                cmd_type_(cmd_type),
                key_(key),
                data_(data, len)
                {
                }

            int64_t req_id_ = 0;
            int32_t cmd_type_ = 0;
            std::string key_;
            std::string data_;
        };

        using RespData = struct RespData{
        RespData(int64_t req_id, Status result, std::string&& data)
        : req_id_(req_id),
                result_(result),
                data_(std::move(data))
                {
                }

            int64_t req_id_ = 0;
            Status result_ = kOk;
            std::string data_;
        };

    public:
        class DbWorker;
        using RespHandler = std::function<void(const RespData& resp)>;

    public:
        DbCore();
        ~DbCore();

        /*
         * Each request will be handled by the thread with id 'hash(key) % threads'
         *
         * @param threads The number of thread
         *
         * @return 0  success
         *         -1 failed
         */
        int init(uint32_t threads);

        /*
         * Set RespHandler to handle the response.
         * Just print by default.
         */
        inline void set_resp_handler(RespHandler handler){
            resp_handler_ = handler;
        }

        /*
         * Do the sql request.
         * The request should be serialized according to the format defined in "internal-idl/dbcmd.fads"
         *
         * @param req_id The request id
         * @param data   The request data
         * @param len    The length of the request data
         *
         * @return 0  success
         *         -1 failed
         */
        Status do_sql(int64_t req_id, std::string&& data);
        Status do_sql(int64_t req_id, const char* data, size_t len);

        std::string get_tables();
        std::string get_custom_type();
        std::string get_table_schema();
        std::string get_table_schema(const std::string& name);
        std::string get_custom_type_schema();
        std::string get_custom_type_schema(const std::string& name);

    private:
        void resp_thread_entry();
        Status create(const std::string& schema);
        void DefaultRespHandler(const RespData& resp);

    private:
        TableCreater table_creater_;
        std::vector<std::shared_ptr<DbWorker>> workers_;
        std::shared_ptr<soce::utils::DispatchQueue_1n<SqlData>> req_queue_;
        std::shared_ptr<soce::utils::DispatchQueue_n1<RespData>> resp_queue_;
        RespHandler resp_handler_;
        std::thread thread_;
        std::atomic<bool> run_{true};
        uint32_t timeout_ = 1000;
        soce::log4rel::MultiThreadSink sink_;
        std::vector<uint32_t> cmd_type_index_{0, 0};
        std::vector<uint32_t> key_index_{1};
    };

    class DbCore::DbWorker
    {
    public:
        DbWorker(std::shared_ptr<soce::utils::DispatchQueue_1n<SqlData>> req_queue,
                 std::shared_ptr<soce::utils::DispatchQueue_n1<RespData>> resp_queue,
                 size_t index);
        int start();
        void stop();
        void thread_entry();
        void add_tables(std::vector<std::shared_ptr<FadsTable>> tables);

    private:
        void do_sql(soce::utils::DQVector<SqlData>& reqs);
        Status do_insert(const SqlData& req);
        Status do_remove(const SqlData& req);
        Status do_update(const SqlData& req);
        Status do_select(const SqlData& req, std::string& out);
        Status do_selup(const SqlData& req, std::string& out);

    private:
        std::thread thread_;
        std::mutex mtx_;
        FadsDb db_;
        size_t timeout_ = 1000;
        bool run_ = false;
        std::shared_ptr<soce::utils::DispatchQueue_1n<SqlData>> req_queue_;
        std::shared_ptr<soce::utils::DispatchQueue_n1<RespData>> resp_queue_;
        size_t index_ = 0;
    };

} // namespace fadsdb
} // namespace soce

#endif
