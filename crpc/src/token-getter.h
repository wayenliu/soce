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

#ifndef _SOCE_TOKEN_GETTER_bOikC8FewB_H_
#define _SOCE_TOKEN_GETTER_bOikC8FewB_H_

#include <string>

namespace soce{
namespace crpc{

    class TokenGetter
    {
    public:
        void set_data(const char* data, size_t len);
        std::string get_token();
        bool end();

    private:
        void skip_space();

    private:
        const char* data_ = NULL;
        size_t len_ = 0;
        size_t pos_ = 0;
    };

} // namespace crpc
} // namespace soce


#endif
