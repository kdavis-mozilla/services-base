/******************************************************************************
 * Copyright (c) 2013-2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#ifdef NEED_DATA_STORE
#include "AboutDataStore.h"
#include <alljoyn/config/AboutDataStoreInterface.h>
#include <alljoyn/AboutData.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

using namespace ajn;

AboutDataStore::AboutDataStore(const char* factoryConfigFile, const char* configFile) :
    AboutDataStoreInterface(factoryConfigFile, configFile), m_IsInitialized(false)
{
    std::cout << "AboutDataStore::AboutDataStore" << std::endl;
    m_configFileName.assign(configFile);
    m_factoryConfigFileName.assign(factoryConfigFile);
}

void AboutDataStore::Initialize()
{
    std::cout << "AboutDataStore::Initialize " << m_factoryConfigFileName << std::endl;
    std::ifstream factoryConfigFile(m_factoryConfigFileName.c_str(), std::ios::binary);
    if (factoryConfigFile) {
        std::string str((std::istreambuf_iterator<char>(factoryConfigFile)),
                        std::istreambuf_iterator<char>());
        std::cout << "Contains:" << str << std::endl;
        QStatus status;
        status = CreateFromXml(qcc::String(str.c_str()));

        if (status != ER_OK) {
            std::cout << "AboutDataStore::Initialize ERROR" << std::endl;
            return;
        }
        size_t numFields = GetFields();
        std::cout << "AboutDataStore::Initialize() numFields=" << numFields << std::endl;

    }

    m_IsInitialized = true;
    std::cout << "AboutDataStore::Initialize End" << std::endl;
}

void AboutDataStore::FactoryReset()
{
    std::cout << "AboutDataStore::FactoryReset" << std::endl;

    std::ifstream factoryConfigFile(m_factoryConfigFileName.c_str(), std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(factoryConfigFile)),
                    std::istreambuf_iterator<char>());
    factoryConfigFile.close();

    std::ofstream configFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
    configFileWrite.write(str.c_str(), str.length());
    configFileWrite.close();


    if (factoryConfigFile) {
        AboutData factoryAboutData;
        QStatus status = factoryAboutData.CreateFromXml(qcc::String(str.c_str()));
        if (status != ER_OK) {
            std::cout << "AboutDataStore::FactoryReset CreateFromXml ERROR" << std::endl;
            return;
        }
        size_t numFields = factoryAboutData.GetFields();

        std::cout << "AboutDataStore::FactoryReset() numFields=" << numFields << std::endl;
        if (0 == numFields) {
            return;
        }
        const char* fieldNames[512];
        factoryAboutData.GetFields(fieldNames, numFields);
        char* defaultLanguage;
        status = factoryAboutData.GetDefaultLanguage(&defaultLanguage);
        if (ER_OK != status) {
            return;
        }
        size_t numLangs = factoryAboutData.GetSupportedLanguages();
        std::cout << "numLangs=" << numLangs << std::endl;
        const char** langs = new const char*[numLangs];
        factoryAboutData.GetSupportedLanguages(langs, numLangs);
        for (size_t i = 0; i < numFields; i++) {
            ajn::MsgArg* arg;
            factoryAboutData.GetField(fieldNames[i], arg);
            if (arg->Signature() != "s") {
                continue;
            }
            SetField(fieldNames[i], *arg);

            if (!factoryAboutData.IsFieldLocalized(fieldNames[i])) {
                continue;
            }

            for (size_t j = 0; j < numLangs; j++) {
                if (langs[j] == defaultLanguage) {
                    continue;
                }
                factoryAboutData.GetField(fieldNames[i], arg, langs[j]);
                SetField(fieldNames[i], *arg, langs[j]);
            }
        }
        delete [] langs;
    }

}

AboutDataStore::~AboutDataStore()
{
    std::cout << "AboutDataStore::~AboutDataStore" << std::endl;
}

QStatus AboutDataStore::ReadAll(const char* languageTag, DataPermission::Filter filter, ajn::MsgArg& all)
{
    std::cout << "AboutDataStore::ReadAll" << std::endl;
    return GetAboutData(&all, languageTag);
}

QStatus AboutDataStore::Update(const char* name, const char* languageTag, const ajn::MsgArg* value)
{
    std::cout << "AboutDataStore::Update" << " name:" << name << " languageTag:" <<  languageTag << " value:" << value << std::endl;

    QStatus status = ER_OK;
    if (strcmp(name, AboutData::APP_ID) == 0) {
        uint8_t* appId = NULL;
        size_t* num = NULL;
        status = value->Get("ay", num, &appId);
        status = SetAppId(appId, *num);
    } else if (strcmp(name, AboutData::DEFAULT_LANGUAGE) == 0) {
        char* defaultLanguage;
        status = value->Get("s", &defaultLanguage);
        status = SetDefaultLanguage(defaultLanguage);
    } else if (strcmp(name, AboutData::DEVICE_NAME) == 0) {
        std::cout << "Got device name" << std::endl;
        char* deviceName = NULL;
        status = value->Get("s", &deviceName);
        status = SetDeviceName(deviceName, languageTag);
    } else if (strcmp(name, AboutData::DEVICE_ID) == 0) {
        char* deviceId = NULL;
        status = value->Get("s", &deviceId);
        status = SetDeviceId(deviceId);
    } else if (strcmp(name, AboutData::APP_NAME) == 0) {
        char* appName = NULL;
        status = value->Get("s", &appName);
        status = SetAppName(appName, languageTag);
    } else if (strcmp(name, AboutData::MANUFACTURER) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        status = SetManufacturer(chval);
    } else if (strcmp(name, AboutData::MODEL_NUMBER) == 0) {
        char* chval = NULL;
        status = value->Get("s", chval);
        status = SetModelNumber(chval);
    } else if (strcmp(name, AboutData::SUPPORTED_LANGUAGES) == 0) {
        //Added automatically when adding value
        std::cout << "AboutDataStore::Update - supported languages will be added automatically when adding value" << std::endl;
    } else if (strcmp(name, AboutData::DESCRIPTION) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        status = SetDescription(chval);
    } else if (strcmp(name, AboutData::DATE_OF_MANUFACTURE) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        status = SetDateOfManufacture(chval);
    } else if (strcmp(name, AboutData::SOFTWARE_VERSION) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        status = SetSoftwareVersion(chval);
    } else if (strcmp(name, AboutData::HARDWARE_VERSION) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        status = SetHardwareVersion(chval);
    } else if (strcmp(name, AboutData::SUPPORT_URL) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        status = SetSupportUrl(chval);
    }

    //Generate xml
    qcc::String str = ToXml();
    //write to config file
    std::ofstream iniFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
    //write to config file
    iniFileWrite.write(str.c_str(), str.length());
    iniFileWrite.close();

    return status;
}

QStatus AboutDataStore::Delete(const char* name, const char* languageTag)
{
    std::cout << "AboutDataStore::Delete(" << name << ")" << std::endl;
    QStatus status = ER_OK;
    char emptyCharValue = '\0';
    if (strcmp(name, AboutData::APP_ID) == 0) {
        uint8_t appId = 0;
        size_t num = 0;
        status = SetAppId(&appId, num);
    } else if (strcmp(name, AboutData::DEFAULT_LANGUAGE) == 0) {
        status = SetDefaultLanguage(&emptyCharValue);
    } else if (strcmp(name, AboutData::DEVICE_NAME) == 0) {
        status = SetDeviceName(&emptyCharValue, languageTag);
        char* deviceName = NULL;
        GetDeviceName(&deviceName, languageTag);
        std::cout <<  "==== deviceName:" << deviceName << std::endl;
    } else if (strcmp(name, AboutData::DEVICE_ID) == 0) {
        status = SetDeviceId(&emptyCharValue);
    } else if (strcmp(name, AboutData::APP_NAME) == 0) {
        status = SetAppName(&emptyCharValue, languageTag);
    } else if (strcmp(name, AboutData::MANUFACTURER) == 0) {
        status = SetManufacturer(&emptyCharValue);
    } else if (strcmp(name, AboutData::MODEL_NUMBER) == 0) {
        status = SetModelNumber(&emptyCharValue);
    } else if (strcmp(name, AboutData::SUPPORTED_LANGUAGES) == 0) {
        //Added automatically when adding value
    } else if (strcmp(name, AboutData::DESCRIPTION) == 0) {
        status = SetDescription(&emptyCharValue);
    } else if (strcmp(name, AboutData::DATE_OF_MANUFACTURE) == 0) {
        status = SetDateOfManufacture(&emptyCharValue);
    } else if (strcmp(name, AboutData::SOFTWARE_VERSION) == 0) {
        status = SetSoftwareVersion(&emptyCharValue);
    } else if (strcmp(name, AboutData::HARDWARE_VERSION) == 0) {
        status = SetHardwareVersion(&emptyCharValue);
    } else if (strcmp(name, AboutData::SUPPORT_URL) == 0) {
        status = SetSupportUrl(&emptyCharValue);
    }

    //Generate xml
    qcc::String str = ToXml();
    //write to config file
    std::ofstream iniFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
    //write to config file
    iniFileWrite.write(str.c_str(), str.length());
    iniFileWrite.close();
    return status;
}

const qcc::String& AboutDataStore::GetConfigFileName()
{
    std::cout << "AboutDataStore::GetConfigFileName" << std::endl;
    return m_configFileName;
}

qcc::String AboutDataStore::ToXml()
{
    std::cout << "AboutDataStore::ToXml" << std::endl;
    QStatus status = ER_OK;
    size_t numFields = GetFields();
    if (0 == numFields) {
        return "";
    }
    const char* fieldNames[512];
    GetFields(fieldNames, numFields);
    char* defaultLanguage;
    status = GetDefaultLanguage(&defaultLanguage);
    if (ER_OK != status) {
        return "";
    }
    size_t numLangs = GetSupportedLanguages();
    const char** langs = new const char*[numLangs];
    GetSupportedLanguages(langs, numLangs);
    qcc::String res;
    res += "<AboutData>\n";
    for (size_t i = 0; i < numFields; i++) {
        ajn::MsgArg* arg;
        char* val;
        GetField(fieldNames[i], arg);
        if (arg->Signature() != "s") {
            continue;
        }
        arg->Get("s", &val);
        res += "  <" + qcc::String(fieldNames[i]) + ">";
        res += val;
        res += "</" + qcc::String(fieldNames[i]) + ">\n";
        if (!IsFieldLocalized(fieldNames[i])) {
            continue;
        }

        for (size_t j = 0; j < numLangs; j++) {
            if (langs[j] == defaultLanguage) {
                continue;
            }

            res += "  <" + qcc::String(fieldNames[i]) + " lang=\"" + langs[j] + "\">";
            GetField(fieldNames[i], arg, langs[j]);
            arg->Get("s", &val);
            res += val;
            res += "</" + qcc::String(fieldNames[i]) + ">\n";
        }
    }
    res += "</AboutData>";

    delete [] langs;
    std::cout << "end" << std::endl;
    return res;
}

#endif //NEED_DATA_STORE
