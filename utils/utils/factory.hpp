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

#ifndef _SOCE_FACTORY_wyuS5dlwmN_H_
#define _SOCE_FACTORY_wyuS5dlwmN_H_

#include <memory>
#include <new>

namespace soce{
namespace utils{

    template <typename T>
    class SoceFactory
    {
    public:
        template<typename... Args>
        static std::shared_ptr<T> create(Args&&... args){
            return std::shared_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
        }
    };

}} // namespace

#endif
