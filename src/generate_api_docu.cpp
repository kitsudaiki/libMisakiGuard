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

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>
#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiEndpoints/endpoint.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi;

namespace Misaka
{

GenerateApiDocu::GenerateApiDocu()
    : Sakura::Blossom("Generate a user-specific documentation for the API "
                                   "of the current component.")
{
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
 * @brief generate documenation for all fields
 *
 * @param docu reference to the complete document
 * @param defMap map with all field to ducument
 */
void
addFieldDocu(std::string &docu,
             const std::map<std::string, Sakura::FieldDef>* defMap)
{
    std::map<std::string, Sakura::FieldDef>::const_iterator it;
    for(it = defMap->begin();
        it != defMap->end();
        it++)
    {
        const std::string field = it->first;
        const Sakura::FieldType fieldType = it->second.fieldType;
        const std::string comment = it->second.comment;
        const bool isRequired = it->second.isRequired;
        const DataItem* defaultVal = it->second.defaultVal;
        const DataItem* matchVal = it->second.match;
        const std::string regexVal = it->second.regex;
        const long lowerBorder = it->second.lowerBorder;
        const long upperBorder = it->second.upperBorder;

        docu.append("\n");
        docu.append("``" + field + "``\n");

        // comment
        if(comment != "")
        {
            docu.append("    **Description:**\n");
            docu.append("        ``" + comment + "``\n");
        }

        // type
        docu.append("    **Type:**\n");
        if(fieldType == Sakura::SAKURA_MAP_TYPE) {
            docu.append("        ``Map``\n");
        } else if(fieldType == Sakura::SAKURA_ARRAY_TYPE) {
            docu.append("        ``Array``\n");
        } else if(fieldType == Sakura::SAKURA_BOOL_TYPE) {
            docu.append("        ``Bool``\n");
        } else if(fieldType == Sakura::SAKURA_INT_TYPE) {
            docu.append("        ``Int``\n");
        } else if(fieldType == Sakura::SAKURA_FLOAT_TYPE) {
            docu.append("        ``Float``\n");
        } else if(fieldType == Sakura::SAKURA_STRING_TYPE) {
            docu.append("        ``String``\n");
        }

        // required
        docu.append("    **Required:**\n");
        if(isRequired) {
            docu.append("        ``True``\n");
        } else {
            docu.append("        ``False``\n");
        }

        // default
        if(defaultVal != nullptr
                && isRequired == false)
        {
            docu.append("    **Default:**\n");
            docu.append("        ``" + defaultVal->toString() + "``\n");
        }

        // match
        if(matchVal != nullptr)
        {
            docu.append("    **Does have the value:**\n");
            docu.append("        ``" + matchVal->toString() + "``\n");
        }

        // match
        if(regexVal != "")
        {
            docu.append("    **Must match the regex:**\n");
            docu.append("        ``" + regexVal + "``\n");
        }

        // border
        if(lowerBorder != 0
                || upperBorder != 0)
        {
            if(fieldType == Sakura::SAKURA_INT_TYPE)
            {
                docu.append("    **Lower border of value:**\n");
                docu.append("        ``" + std::to_string(lowerBorder) + "``\n");
                docu.append("    **Upper border of value:**\n");
                docu.append("        ``" + std::to_string(upperBorder) + "``\n");
            }
            if(fieldType == Sakura::SAKURA_STRING_TYPE)
            {
                docu.append("    **Minimum string-length:**\n");
                docu.append("        ``" + std::to_string(lowerBorder) + "``\n");
                docu.append("    **Maximum string-length:**\n");
                docu.append("        ``" + std::to_string(upperBorder) + "``\n");
            }
        }
    }
}

/**
 * @brief generate documentation of a blossom-item
 *
 * @param docu reference to the complete document
 * @param langInterface pinter to the sakura-language-interface
 * @param groupName group of the blossom
 * @param itemName name of the blossom in group
 */
void
createBlossomDocu(std::string &docu,
                  Sakura::SakuraLangInterface* langInterface,
                  const std::string &groupName,
                  const std::string &itemName)
{
    Sakura::Blossom* blossom = langInterface->getBlossom(groupName, itemName);

    if(blossom == nullptr) {
        // TODO: handle error
        return;
    }

    // add comment/describtion
    docu.append(blossom->comment + "\n");

    // add input-fields
    docu.append("\n");
    docu.append("Request-Parameter\n");
    docu.append("~~~~~~~~~~~~~~~~~\n");
    addFieldDocu(docu, blossom->getInputValidationMap());

    // add output-fields
    docu.append("\n");
    docu.append("Response-Parameter\n");
    docu.append("~~~~~~~~~~~~~~~~~~\n");
    addFieldDocu(docu, blossom->getOutputValidationMap());
}

/**
 * @brief generate documentation of a tree-item
 *
 * @param docu reference to the complete document
 * @param langInterface pinter to the sakura-language-interface
 * @param treeId id of the tree to document
 */
void
createTreeDocu(std::string &docu,
               Sakura::SakuraLangInterface* langInterface,
               const std::string &treeId)
{
    // request necessary infos from the tree-item
    std::map<std::string, Sakura::FieldDef> validationMap;
    if(langInterface->getTreeValidMap(validationMap, treeId) == false) {
        // TODO: handle error
        return;
    }

    // add comment/describtion
    std::string comment;
    langInterface->getTreeComment(comment, treeId);
    docu.append(comment + "\n");

    // add input-fields
    docu.append("\n");
    docu.append("Request-Parameter\n");
    docu.append("~~~~~~~~~~~~~~~~~\n");
    addFieldDocu(docu, &validationMap);

    // add output-fields
    docu.append("\n");
    docu.append("Response-Parameter\n");
    docu.append("~~~~~~~~~~~~~~~~~~\n");
    addFieldDocu(docu, &validationMap);
}

/**
 * @brief generate documentation for the endpoints
 *
 * @param docu reference to the complete document
 */
void
generateEndpointDocu(std::string &docu)
{
    Hanami::Endpoint* endpoints = Hanami::Endpoint::getInstance();
    Sakura::SakuraLangInterface* langInterface =
            Sakura::SakuraLangInterface::getInstance();
    docu.append("\n");

    std::map<std::string, std::map<Hanami::HttpRequestType, Hanami::EndpointEntry>>::iterator it;
    for(it = endpoints->endpointRules.begin();
        it != endpoints->endpointRules.end();
        it++)
    {
        const std::string endpoint = it->first;

        // add endpoint
        docu.append(endpoint);
        docu.append("\n");
        docu.append(std::string(endpoint.size(), '-'));
        docu.append("\n");

        std::map<Hanami::HttpRequestType, Hanami::EndpointEntry>::const_iterator ruleIt;
        for(ruleIt = it->second.begin();
            ruleIt != it->second.end();
            ruleIt++)
        {
            docu.append("\n");

            // add http-type
            if(ruleIt->first == Hanami::GET_TYPE) {
                docu.append("GET\n^^^\n\n");
            } else if(ruleIt->first == Hanami::POST_TYPE) {
                docu.append("POST\n^^^^\n\n");
            } else if(ruleIt->first == Hanami::DELETE_TYPE) {
                docu.append("DELETE\n^^^^^^\n\n");
            } else if(ruleIt->first == Hanami::PUT_TYPE) {
                docu.append("PUT\n^^^\n\n");
            }

            if(ruleIt->second.type == Hanami::BLOSSOM_TYPE) {
                createBlossomDocu(docu, langInterface, ruleIt->second.group, ruleIt->second.name);
            } else {
                createTreeDocu(docu, langInterface, ruleIt->second.name);
            }
        }
    }
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
    const std::string localComponent =
            Hanami::SupportedComponents::getInstance()->localComponent;

    std::string documentsion = "";
    documentsion.append(localComponent);
    toUpperCase(documentsion);
    documentsion.append("\n");
    documentsion.append(localComponent.size(), '=');
    documentsion.append("\n");

    generateEndpointDocu(documentsion);
    std::string base64Docu;
    Crypto::encodeBase64(base64Docu, documentsion.c_str(), documentsion.size());

    blossomLeaf.output.insert("documentation", base64Docu);

    return true;
}

}  // namespace Misaka
