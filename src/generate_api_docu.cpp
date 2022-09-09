/**
 * @file        generate_api_docu.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2021 Tobias Anker
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

#include "generate_api_docu.h"

#include <rst_docu_generation.h>

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi;
using Kitsunemimi::Hanami::SupportedComponents;

namespace Misaki
{

GenerateApiDocu::GenerateApiDocu()
    : Sakura::Blossom("Generate a user-specific documentation for the API "
                                   "of the current component.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("type",
                       Sakura::SAKURA_STRING_TYPE,
                       false,
                       "Output-type of the document (pdf, rst, md).");
    assert(addFieldDefault("type", new Kitsunemimi::DataValue("pdf")));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("documentation",
                        Sakura::SAKURA_STRING_TYPE,
                        "API-documentation as base64 converted string.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
GenerateApiDocu::runTask(Sakura::BlossomLeaf &blossomLeaf,
                         const DataMap &,
                         Sakura::BlossomStatus &,
                         ErrorContainer &)
{
    const std::string localComponent = SupportedComponents::getInstance()->localComponent;
    const std::string type = blossomLeaf.input.get("type").getString();

    std::string documentsion = "";

    if(type == "rst"
            || type == "pdf")
    {
        createRstDocumentation(documentsion, localComponent);
    }

    std::string base64Docu;
    Crypto::encodeBase64(base64Docu, documentsion.c_str(), documentsion.size());

    blossomLeaf.output.insert("documentation", base64Docu);

    return true;
}

}  // namespace Misaki
