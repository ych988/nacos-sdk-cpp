#include "NacosConfigService.h"
#include "src/security/SecurityManager.h"
#include "src/log/Logger.h"
#include "ConfigProxy.h"
#include "src/utils/ParamUtils.h"

using namespace std;

namespace nacos{
NacosConfigService::NacosConfigService(ObjectConfigData *objectConfigData) NACOS_THROW(NacosException) {
    _objectConfigData = objectConfigData;
    if (_objectConfigData->_appConfigManager->nacosAuthEnabled()) {
        _objectConfigData->_securityManager->login();
        _objectConfigData->_securityManager->start();
    }
}

NacosConfigService::~NacosConfigService() {
    log_debug("[NacosConfigService]:~NacosConfigService()\n");
    delete _objectConfigData;
}

NacosString NacosConfigService::getConfig
        (
                const NacosString &dataId,
                const NacosString &group,
                long timeoutMs
        ) NACOS_THROW(NacosException) {
    return getConfigInner(getNamespace(), dataId, group, timeoutMs);
}

bool NacosConfigService::publishConfig
        (
                const NacosString &dataId,
                const NacosString &group,
                const NacosString &content
        ) NACOS_THROW(NacosException) {
    return publishConfigInner(getNamespace(), dataId, group, NULLSTR, NULLSTR, NULLSTR, content);
}

bool NacosConfigService::removeConfig
        (
                const NacosString &dataId,
                const NacosString &group
        ) NACOS_THROW(NacosException) {
    return removeConfigInner(getNamespace(), dataId, group, NULLSTR);
}

NacosString NacosConfigService::getConfigInner
        (
                const NacosString &tenant,
                const NacosString &dataId,
                const NacosString &group,
                long timeoutMs
        ) NACOS_THROW(NacosException) {
    NacosString result = NULLSTR;

    AppConfigManager *_appConfigManager = _objectConfigData->_appConfigManager;
    LocalSnapshotManager *_localSnapshotManager = _objectConfigData->_localSnapshotManager;

    NacosString clientName = _appConfigManager->get(PropertyKeyConst::CLIENT_NAME);
    result = _localSnapshotManager->getFailover(clientName.c_str(), dataId, group, tenant);
    if (!NacosStringOps::isNullStr(result)) {
        log_warn("[NacosConfigService]-getConfig:[clientName=%s] get failover ok, dataId=%s, group=%s, tenant=%s, config=%s",
                 clientName.c_str(),
                 dataId.c_str(),
                 group.c_str(),
                 tenant.c_str(),
                 result.c_str());
        return result;
    }

    try {
        result = _objectConfigData->_clientWorker->getServerConfig(tenant, dataId, group, timeoutMs);
    } catch (NacosException &e) {
        if (e.errorcode() == NacosException::NO_RIGHT) {
            log_error("Invalid credential, e: %d = %s\n", e.errorcode(), e.what());
        }
        const NacosString &clientName = _appConfigManager->get(PropertyKeyConst::CLIENT_NAME);
        result = _localSnapshotManager->getSnapshot(clientName, dataId, group, tenant);
        if (e.errorcode() == NacosException::NO_RIGHT && NacosStringOps::isNullStr(result)) {
            //permission denied and no failback, let user decide what to do
            throw e;
        }
    }
    return result;
}

bool NacosConfigService::removeConfigInner
        (
                const NacosString &tenant,
                const NacosString &dataId,
                const NacosString &group,
                const NacosString &tag
        ) NACOS_THROW(NacosException) {
    std::list <NacosString> headers;
    std::list <NacosString> paramValues;
    //Get the request url
    NacosString path = _objectConfigData->_appConfigManager->getContextPath() + ConfigConstant::CONFIG_CONTROLLER_PATH;

    HttpResult res;

    paramValues.push_back("dataId");
    paramValues.push_back(dataId);

    NacosString parmGroupid = ParamUtils::null2defaultGroup(group);
    paramValues.push_back("group");
    paramValues.push_back(parmGroupid);

    if (!isNull_nacos(tenant)) {
        paramValues.push_back("tenant");
        paramValues.push_back(tenant);
    }

    NacosString serverAddr = _objectConfigData->_serverListManager->getCurrentServerAddr();
    NacosString url = serverAddr + "/" + path;
    log_debug("[NacosConfigService]-removeConfigInner: Assembled URL:%s\n", url.c_str());

    ConfigProxy *_configProxy = _objectConfigData->_configProxy;
    try {
        res = _configProxy->reqAPI(IHttpCli::DELETE_COMPATIBLE, url, headers, paramValues, _objectConfigData->encoding, POST_TIMEOUT);
    }
    catch (NetworkException &e) {
        log_warn("[NacosConfigService]-removeConfigInner: error, %s, %s, %s, msg: %s\n", dataId.c_str(), group.c_str(), tenant.c_str(), e.what());
        return false;
    }

    //If the server returns true, then this call succeeds
    if (res.content.compare("true") == 0) {
        return true;
    } else {
        return false;
    }
}

bool NacosConfigService::publishConfigInner
        (
                const NacosString &tenant,
                const NacosString &dataId,
                const NacosString &group,
                const NacosString &tag,
                const NacosString &appName,
                const NacosString &betaIps,
                const NacosString &content
        ) NACOS_THROW(NacosException) {
    //TODO:More stringent check, need to improve checkParam() function
    ParamUtils::checkParam(dataId, group, content);

    std::list <NacosString> headers;
    std::list <NacosString> paramValues;
    NacosString parmGroupid;
    //Get the request url
    NacosString path = _objectConfigData->_appConfigManager->getContextPath() + ConfigConstant::CONFIG_CONTROLLER_PATH;

    HttpResult res;

    parmGroupid = ParamUtils::null2defaultGroup(group);
    ParamUtils::addKV(paramValues, "group", parmGroupid);

    ParamUtils::addKV(paramValues, "dataId", dataId);

    ParamUtils::addKV(paramValues, "content", content);

    if (!isNull_nacos(tenant)) {
        ParamUtils::addKV(paramValues, "tenant", tenant);
    }

    if (!isNull_nacos(appName)) {
        ParamUtils::addKV(paramValues, "appName", appName);
    }

    if (!isNull_nacos(tag)) {
        ParamUtils::addKV(paramValues, "tag", tag);
    }

    if (!isNull_nacos(betaIps)) {
        ParamUtils::addKV(paramValues, "betaIps", betaIps);
    }

    NacosString serverAddr = _objectConfigData->_serverListManager->getCurrentServerAddr();
    NacosString url = serverAddr + "/" + path;
    log_debug("[NacosConfigService]-publishConfigInner:httpPost Assembled URL:%s\n", url.c_str());

    ConfigProxy *_configProxy = _objectConfigData->_configProxy;
    try {
        res = _configProxy->reqAPI(IHttpCli::POST, url, headers, paramValues, _objectConfigData->encoding, POST_TIMEOUT);
    }
    catch (NetworkException &e) {
        //
        log_warn("[NacosConfigService]-publishConfigInner: exception, dataId=%s, group=%s, msg=%s\n", dataId.c_str(), group.c_str(),
                 tenant.c_str(), e.what());
        return false;
    }

    //If the server returns true, then this call succeeds
    if (res.content.compare("true") == 0) {
        return true;
    } else {
        return false;
    }
}

void NacosConfigService::addListener
        (
                const NacosString &dataId,
                const NacosString &group,
                Listener *listener
        ) NACOS_THROW(NacosException) {
    NacosString parmgroup = ConfigConstant::DEFAULT_GROUP;
    if (!isNull_nacos(group)) {
        parmgroup = group;
    }

    //TODO:give a constant to this hard-coded number
    NacosString cfgcontent;
    try {
        cfgcontent = getConfig(dataId, group, 3000);
    } catch (NacosException &e) {
        cfgcontent = "";
    }

    _objectConfigData->_clientWorker->addListener(dataId, parmgroup, getNamespace(), cfgcontent, listener);
    _objectConfigData->_clientWorker->startListening();
}

void NacosConfigService::removeListener
        (
                const NacosString &dataId,
                const NacosString &group,
                Listener *listener
        ) {
    NacosString parmgroup = ConfigConstant::DEFAULT_GROUP;
    if (!isNull_nacos(group)) {
        parmgroup = group;
    }
    log_debug("[NacosConfigService]-removeListener: calling client worker\n");
    _objectConfigData->_clientWorker->removeListener(dataId, parmgroup, getNamespace(), listener);
}

}//namespace nacos
