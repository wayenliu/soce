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

#ifndef _SOCE_FADS_DB_pq3BSrT1Br_H_
#define _SOCE_FADS_DB_pq3BSrT1Br_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include "status.h"

namespace soce{
namespace fadsdb{

    class FadsTable;

    class FadsDb
    {
    public:
        /* int alert(const std::string& schema); */
        void add_tables(const std::vector<std::shared_ptr<FadsTable>>& tables);
        Status insert(const std::string& table,
                      const std::string& key,
                      const char* data,
                      size_t len);
        Status remove(const std::string& table, const std::string& key);
        Status update(const std::string& table,
                      const std::string& key,
                      const std::string& cmd,
                      const std::string& filters);
        Status select(const std::string& table,
                      const std::string& key,
                      const std::string& fields,
                      const std::string& filters,
                      std::string& out);
        Status selup(const std::string& table,
                     const std::string& key,
                     const std::string& fields,
                     const std::string& uncond_cmds,
                     const std::string& precmds,
                     const std::string& subcmds,
                     const std::string& filters,
                     std::string& out);

    private:
        std::unordered_map<std::string, std::shared_ptr<FadsTable>> tables_;
    };

} // namespace fadsdb
} // namespace soce

#endif
