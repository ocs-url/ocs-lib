#include "confighandler.h"

#include <QStringList>
#include <QStandardPaths>

#include "qtil_dir.h"

ConfigHandler::ConfigHandler(QObject *parent)
    : QObject(parent)
{
    appConfig_ = Qtil::Config(":/configs");
    importAppConfigApplication();
    importAppConfigInstallTypes();
    usrConfig_ = Qtil::Config(Qtil::Dir::genericConfigPath() + "/" + getAppConfigApplication()["id"].toString());
}

QJsonObject ConfigHandler::getAppConfigApplication() const
{
    return appConfigApplication_;
}

QJsonObject ConfigHandler::getAppConfigInstallTypes() const
{
    return appConfigInstallTypes_;
}

QJsonObject ConfigHandler::getUsrConfigApplication() const
{
    return usrConfig_.get("application");
}

bool ConfigHandler::setUsrConfigApplication(const QJsonObject &object) const
{
    /* object format
    {
        "update_checked_at": 1483658977219
    }
    */
    return usrConfig_.set("application", object);
}

QJsonObject ConfigHandler::getUsrConfigProviders() const
{
    return usrConfig_.get("providers");
}

bool ConfigHandler::setUsrConfigProviders(const QJsonObject &object) const
{
    return usrConfig_.set("providers", object);
}

QJsonObject ConfigHandler::getUsrConfigCategories() const
{
    return usrConfig_.get("categories");
}

bool ConfigHandler::setUsrConfigCategories(const QJsonObject &object) const
{
    return usrConfig_.set("categories", object);
}

QJsonObject ConfigHandler::getUsrConfigInstalledItems() const
{
    return usrConfig_.get("installed_items");
}

bool ConfigHandler::setUsrConfigInstalledItems(const QJsonObject &object) const
{
    return usrConfig_.set("installed_items", object);
}

QJsonObject ConfigHandler::getUsrConfigUpdateAvailableItems() const
{
    return usrConfig_.get("update_available_items");
}

bool ConfigHandler::setUsrConfigUpdateAvailableItems(const QJsonObject &object) const
{
    return usrConfig_.set("update_available_items", object);
}

bool ConfigHandler::setUsrConfigProvidersProvider(const QString &providerKey, const QJsonObject &object) const
{
    /* object format
    {
        "id": "example",
        "location": "https://example.com/ocs/v1/",
        "name": "Example",
        "icon": "https://example.com/icon.png",
        "termsofuse": "https://example.com/termsofuse",
        "register": "https://example.com/register",
        "_providerfile": "https://example.com/ocs/providers.xml"
    }
    */
    auto providers = getUsrConfigProviders();
    providers[providerKey] = object;
    return setUsrConfigProviders(providers);
}

bool ConfigHandler::removeUsrConfigProvidersProvider(const QString &providerKey) const
{
    auto providers = getUsrConfigProviders();
    providers.remove(providerKey);
    return setUsrConfigProviders(providers);
}

bool ConfigHandler::setUsrConfigCategoriesProvider(const QString &providerKey, const QJsonObject &object) const
{
    /* object format
    {
        "1": {
            "id": "1",
            "name": "Desktop Icons",
            "install_type": "icons"
        },
        "2": {
            "id": "2",
            "name": "Desktop Themes",
            "install_type": "themes"
        }
    }
    */
    auto categories = getUsrConfigCategories();
    categories[providerKey] = object;
    return setUsrConfigCategories(categories);
}

bool ConfigHandler::removeUsrConfigCategoriesProvider(const QString &providerKey) const
{
    auto categories = getUsrConfigCategories();
    categories.remove(providerKey);
    return setUsrConfigCategories(categories);
}

bool ConfigHandler::setUsrConfigCategoriesInstallType(const QString &providerKey, const QString &categoryKey, const QString &installType) const
{
    auto categories = getUsrConfigCategories();
    QJsonObject providerCategories;
    if (categories.contains(providerKey)) {
        providerCategories = categories[providerKey].toObject();
    }
    QJsonObject providerCategory;
    if (providerCategories.contains(categoryKey)) {
        providerCategory = providerCategories[categoryKey].toObject();
    }
    providerCategory["install_type"] = installType;
    providerCategories[categoryKey] = providerCategory;
    categories[providerKey] = providerCategories;
    return setUsrConfigCategories(categories);
}

bool ConfigHandler::setUsrConfigInstalledItemsItem(const QString &itemKey, const QJsonObject &object) const
{
    /* object format
    {
        "url": "http://example.com/downloads/123-1.tar.gz",
        "filename": "123-1.tar.gz",
        "install_type": "icons",
        "provider": "http://example.com/ocs/v1/",
        "content_id": "123",
        "files": [
            "iconset-light",
            "iconset-dark"
        ],
        "installed_at": 1483658977219
    }
    */
    auto installedItems = getUsrConfigInstalledItems();
    installedItems[itemKey] = object;
    return setUsrConfigInstalledItems(installedItems);
}

bool ConfigHandler::removeUsrConfigInstalledItemsItem(const QString &itemKey) const
{
    auto installedItems = getUsrConfigInstalledItems();
    installedItems.remove(itemKey);
    return setUsrConfigInstalledItems(installedItems);
}

bool ConfigHandler::setUsrConfigUpdateAvailableItemsItem(const QString &itemKey, const QJsonObject &object) const
{
    /* object format
    {
        "installed_item": "http://example.com/downloads/example.AppImage",
        "update_method": "appimageupdate"
    }
    */
    auto updateAvailableItems = getUsrConfigUpdateAvailableItems();
    updateAvailableItems[itemKey] = object;
    return setUsrConfigUpdateAvailableItems(updateAvailableItems);
}

bool ConfigHandler::removeUsrConfigUpdateAvailableItemsItem(const QString &itemKey) const
{
    auto updateAvailableItems = getUsrConfigUpdateAvailableItems();
    updateAvailableItems.remove(itemKey);
    return setUsrConfigUpdateAvailableItems(updateAvailableItems);
}

void ConfigHandler::importAppConfigApplication()
{
    appConfigApplication_ = appConfig_.get("application");
}

void ConfigHandler::importAppConfigInstallTypes()
{
    auto installTypes = appConfig_.get("install_types");
    for (const auto &key : installTypes.keys()) {
        auto installtype = installTypes[key].toObject();
        installtype["destination"] = convertPathString(installtype["destination"].toString());
        installtype["generic_destination"] = convertPathString(installtype["generic_destination"].toString());
        installTypes[key] = installtype;
    }
    auto installTypesAlias = appConfig_.get("install_types_alias");
    for (const auto &key : installTypesAlias.keys()) {
        auto installTypeAlias = installTypesAlias[key].toObject();
        auto baseKey = installTypeAlias["base"].toString();
        if (installTypes.contains(baseKey)) {
            auto installType = installTypes[baseKey].toObject();
            installType["base"] = baseKey;
            installType["name"] = installTypeAlias["name"].toString();
            installTypes[key] = installType;
        }
    }
    appConfigInstallTypes_ = installTypes;
}

QString ConfigHandler::convertPathString(const QString &path) const
{
    auto newPath = path;
    if (newPath.contains("$HOME")) {
        newPath.replace("$HOME", Qtil::Dir::homePath());
    }
    else if (newPath.contains("$XDG_DOCUMENTS_DIR")) {
        newPath.replace("$XDG_DOCUMENTS_DIR", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    }
    else if (newPath.contains("$XDG_DOWNLOAD_DIR")) {
        newPath.replace("$XDG_DOWNLOAD_DIR", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    }
    else if (newPath.contains("$XDG_PICTURES_DIR")) {
        newPath.replace("$XDG_PICTURES_DIR", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    }
    else if (newPath.contains("$XDG_MUSIC_DIR")) {
        newPath.replace("$XDG_MUSIC_DIR", QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    }
    else if (newPath.contains("$XDG_VIDEOS_DIR")) {
        newPath.replace("$XDG_VIDEOS_DIR", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    }
    else if (newPath.contains("$XDG_DATA_HOME")) {
        newPath.replace("$XDG_DATA_HOME", Qtil::Dir::genericDataPath());
    }
    else if (newPath.contains("$KDEHOME")) {
        newPath.replace("$KDEHOME", Qtil::Dir::kdehomePath());
    }
    else if (newPath.contains("$APP_DATA")) {
        newPath.replace("$APP_DATA", Qtil::Dir::genericDataPath() + "/" + getAppConfigApplication()["id"].toString());
    }
    return newPath;
}
