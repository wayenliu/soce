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

#ifndef _SOCE_COND_EXPR_rESIIwGFMx_H_
#define _SOCE_COND_EXPR_rESIIwGFMx_H_

#include <memory>
#include <functional>

namespace soce{
namespace utils{

    using ConcreteExpr = std::function<bool(const std::string&,
                                            const std::string&,
                                            const std::string&)>;


    class ExprIf;

    /*
     * Condition expression like "a > 1 && ( c != 2 || d < 4)".
     * Token "a > 1" will be split to symbols "a", ">" and "1"
     * which will be passed to ConcreteExpr.
     */
    class CondExpr
    {
    public:
        /*
         * Parse the condition expression
         *
         * @param data The data of the condition expression
         * @param len  The length of the condition expression
         *
         * @return 0 success
         *         -1 failed
         */
        int parse(const char* data, size_t len);

        /*
         * calculate the condition expression
         *
         * @parse expr The expression
         *
         * @return true or false
         */
        bool calc(ConcreteExpr expr);

        void show();

    protected:
        std::shared_ptr<ExprIf> expr_;
    };

} // namespace utils
} // namespace soce

#endif
