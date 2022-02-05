/**
 * @file        misaka_input.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2022 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include <libMisakaGuard/misaka_input.h>
#include <generate_api_docu.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

namespace Misaka
{

/**
 * @brief HanamiMessaging::initMisakaBlossoms
 * @return
 */
bool
initMisakaBlossoms()
{
    // init predefined blossoms
    Kitsunemimi::Sakura::SakuraLangInterface* interface =
            Kitsunemimi::Sakura::SakuraLangInterface::getInstance();
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    const std::string group = "-";

    if(interface->addBlossom(group, "get_api_documentation", new GenerateApiDocu()) == false) {
        return false;
    }
    // add new endpoints
    if(endpoints->addEndpoint("v1/documentation/api",
                              Kitsunemimi::Hanami::GET_TYPE,
                              Kitsunemimi::Hanami::BLOSSOM_TYPE,
                              group,
                              "get_api_documentation") == false)
    {
        return false;
    }

    return true;
}

}
