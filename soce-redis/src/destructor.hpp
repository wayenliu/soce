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

#ifndef _SOCE_DESTRUCTOR_fbz3HNvNeJ_H_
#define _SOCE_DESTRUCTOR_fbz3HNvNeJ_H_

#include <hiredis/hiredis.h>

namespace soce{
namespace redis{

    void free_redis_contex(redisContext* ctx)
    {
        if (ctx){
            redisFree(ctx);
        }
    }

    void free_redis_reply(redisReply* reply)
    {
        if (reply){
            freeReplyObject(reply);
        }
    }

}} // end namespace
#endif
