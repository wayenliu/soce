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

#include <thread>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include "dbcore.h"
#include "dbcmd.h"
#include "proto/dynamic-getter.h"
#include "utils/hash.h"
#include "log4rel/logger.h"

using std::string;
using soce::utils::DispatchQueue;
using namespace soce::log4rel;

namespace soce{
namespace fadsdb{

    DbCore::DbCore()
    {
    }

    DbCore::~DbCore()
    {
        run_.store(false);
    }

    int DbCore::init(uint32_t threads)
    {
        req_queue_.reset(new DispatchQueue<SqlData>(threads, [](const SqlData& data, size_t consumers){
                    uint32_t slot = soce::utils::Hash::crc32(data.key_);
                    return slot % consumers;
                }));
        resp_queue_.reset(new DispatchQueue<RespData>());

        int rc = 0;
        for (uint32_t i=0; i<threads; ++i){
            workers_.emplace_back(std::shared_ptr<DbWorker>(new DbWorker(req_queue_,
                                                                         resp_queue_,
                                                                         i)));
            if (workers_.back()->start()){
                rc = -1;
                break;
            }
        }

        if (rc){
            for (auto& i : workers_){
                i->stop();
            }
        }

        return rc;
    }

    FadsDbRespStatus DbCore::do_sql(int64_t req_id, string&& data)
    {
        soce::proto::DynamicGetter dg;
        dg.init(const_cast<char*>(data.c_str()), data.size());
        int32_t cmd_type = -1;
        string key;
        if (dg.get_enum(cmd_type_index_, cmd_type)
            || dg.get_string(key_index_, key)){
            return kErrProto;
        }

        if (cmd_type == kCmdCreate){
            FadsDbRespStatus rc = create(key);
            resp_queue_->produce(req_id, rc, std::string());
        }
        else{
            req_queue_->produce(req_id, cmd_type, key, std::move(data));
        }

        return kOk;
    }

    FadsDbRespStatus DbCore::do_sql(int64_t req_id, const char* data, size_t len)
    {
        soce::proto::DynamicGetter dg;
        dg.init(const_cast<char*>(data), len);
        int32_t cmd_type = -1;
        string key;
        if (dg.get_enum(cmd_type_index_, cmd_type)
            || dg.get_string(key_index_, key)){
            return kErrProto;
        }

        if (cmd_type == kCmdCreate){
            FadsDbRespStatus rc = create(key);
            resp_queue_->produce(req_id, rc, std::string());
        }
        else{
            req_queue_->produce(req_id, cmd_type, key, data, len);
        }

        return kOk;
    }

    string DbCore::get_tables()
    {
        return std::move(table_creater_.get_tables());
    }

    string DbCore::get_custom_type()
    {
        return std::move(table_creater_.get_custom_type());
    }

    string DbCore::get_table_schema()
    {
        return std::move(table_creater_.get_table_schema());
    }

    string DbCore::get_table_schema(const string& name)
    {
        return std::move(table_creater_.get_table_schema(name));
    }

    string DbCore::get_custom_type_schema()
    {
        return std::move(table_creater_.get_custom_type_schema());
    }

    string DbCore::get_custom_type_schema(const string& name)
    {
        return std::move(table_creater_.get_custom_type_schema(name));
    }

    FadsDbRespStatus DbCore::create(const string& schema)
    {
        std::vector<std::shared_ptr<FadsTable>> tables;
        if (table_creater_.create(schema, tables)){
            return kErrCrt;
        }

        for (auto& i : workers_){
            i->add_tables(tables);
        }

        return kOk;
    }

    DbCore::DbWorker::DbWorker(std::shared_ptr<DispatchQueue<SqlData>> req_queue,
                               std::shared_ptr<DispatchQueue<RespData>> resp_queue,
                               size_t index)
        : req_queue_(req_queue), resp_queue_(resp_queue), index_(index)
    {
    }

    int DbCore::DbWorker::start()
    {
        run_ = true;
        thread_ = std::thread(&DbCore::DbWorker::thread_entry, this);
        return 0;
    }

    void DbCore::DbWorker::stop()
    {
        if (run_ && thread_.joinable()){
            run_ = false;
            thread_.join();
        }
    }

    void DbCore::DbWorker::thread_entry()
    {
        while (run_){
            soce::utils::FQVector<SqlData> reqs;
            req_queue_->try_consume_for(index_, reqs, timeout_);
            do_sql(reqs);
        }

        soce::utils::FQVector<SqlData> reqs;
        req_queue_->try_consume_for(index_, reqs, timeout_);
        do_sql(reqs);
    }

    void DbCore::DbWorker::add_tables(std::vector<std::shared_ptr<FadsTable>> tables)
    {
        std::lock_guard<std::mutex> lck(mtx_);
        db_.add_tables(tables);
    }

    void DbCore::DbWorker::do_sql(soce::utils::FQVector<SqlData>& reqs)
    {
        std::lock_guard<std::mutex> lck(mtx_);

        for (auto& i : reqs){
            FadsDbRespStatus rc = kOk;
            string out;

            switch (i.cmd_type_){
            case kCmdInsert:
                rc = do_insert(i);
                break;

            case kCmdRemove:
                rc = do_remove(i);
                break;

            case kCmdUpdate:
                rc = do_update(i);
                break;

            case kCmdSelect:
                rc = do_select(i, out);
                break;

            case kCmdSelUp:
                rc = do_selup(i, out);
                break;

            default:
                break;
            }

            resp_queue_->produce(i.req_id_, rc, std::move(out));
        }
    }

    FadsDbRespStatus DbCore::DbWorker::do_insert(const SqlData& req)
    {
        soce::proto::BinaryProto bp;
        bp.init(const_cast<char*>(req.data_.c_str()), req.data_.size());

        FadsDbInsert cmd;
        if (cmd.deserialize(&bp) == 0){
            return kErrProto;
        }

        return db_.insert(cmd.get_table(),
                          cmd.get_key(),
                          cmd.get_data().c_str(),
                          cmd.get_data().size());
    }

    FadsDbRespStatus DbCore::DbWorker::do_remove(const SqlData& req)
    {
        soce::proto::BinaryProto bp;
        bp.init(const_cast<char*>(req.data_.c_str()), req.data_.size());

        FadsDbRemove cmd;
        if (cmd.deserialize(&bp) == 0){
            return kErrProto;
        }

        return db_.remove(cmd.get_table(), cmd.get_key());
    }

    FadsDbRespStatus DbCore::DbWorker::do_update(const SqlData& req)
    {
        soce::proto::BinaryProto bp;
        bp.init(const_cast<char*>(req.data_.c_str()), req.data_.size());

        FadsDbUpdate cmd;
        if (cmd.deserialize(&bp) == 0){
            return kErrProto;
        }

        return db_.update(cmd.get_table(),
                          cmd.get_key(),
                          cmd.get_cmd(),
                          (cmd.has_filters() ? cmd.get_filters() : ""));
    }

    FadsDbRespStatus DbCore::DbWorker::do_select(const SqlData& req, string& out)
    {
        soce::proto::BinaryProto bp;
        bp.init(const_cast<char*>(req.data_.c_str()), req.data_.size());

        FadsDbSelect cmd;
        if (cmd.deserialize(&bp) == 0){
            return kErrProto;
        }

        return db_.select(cmd.get_table(),
                          cmd.get_key(),
                          cmd.get_fields(),
                          (cmd.has_filters() ? cmd.get_filters() : ""),
                          out);
    }

    FadsDbRespStatus DbCore::DbWorker::do_selup(const SqlData& req, string& out)
    {
        soce::proto::BinaryProto bp;
        bp.init(const_cast<char*>(req.data_.c_str()), req.data_.size());

        FadsDbSelUp cmd;
        if (cmd.deserialize(&bp) == 0){
            return kErrProto;
        }

        return db_.selup(cmd.get_table(),
                         cmd.get_key(),
                         cmd.get_fields(),
                         (cmd.has_uncond_cmds() ? cmd.get_uncond_cmds() : ""),
                         (cmd.has_precmds() ? cmd.get_precmds() : ""),
                         (cmd.has_subcmds() ? cmd.get_subcmds() : ""),
                         (cmd.has_filters() ? cmd.get_filters() : ""),
                         out);
    }

} // namespace fadsdb
} // namespace soce
