#include "core/modules/ModuleManager.h"
#include <qobject.h>

ModuleManager& ModuleManager::instance() {
    static ModuleManager inst;
    return inst;
}

void ModuleManager::registerTab(const QString& group, CreatorTabModule creator, const int& position) {
    TabModuleDescription tabDesc;
    tabDesc.creator = creator;
    tabDesc.position = position;
    m_tabModuleCreators[group].append(tabDesc);
}

void ModuleManager::registerWindow(const QString& group, CreatorWindowModule creator) {
    m_windowModuleCreators[group].append(creator);
}

void ModuleManager::registerReference(const QString& group, CreatorReferenceModule creator) {
    m_referenceModuleCreators[group].append(creator);
}

QList<QString> ModuleManager::getTabGroups() const{
    return m_tabModuleCreators.keys();
}

QList<QString> ModuleManager::getWindowGroups() const{
    return m_windowModuleCreators.keys();
}

QList<QString> ModuleManager::getReferenceGroups() const{
    return m_referenceModuleCreators.keys();
}


const QVector<TabModuleDescription>& ModuleManager::getTabsByGroup(const QString& group) const {
    static const QVector<TabModuleDescription> empty;
    auto it = m_tabModuleCreators.find(group);
    if (it == m_tabModuleCreators.end()) return empty;
    return it.value();
}

const QVector<CreatorWindowModule>& ModuleManager::getWindowsByGroup(const QString& group) const {
    static const QVector<CreatorWindowModule> empty;
    auto it = m_windowModuleCreators.find(group);
    if (it == m_windowModuleCreators.end()) return empty;
    return it.value();
}

const QVector<CreatorReferenceModule>& ModuleManager::getReferencesByGroup(const QString& group) const {
    static const QVector<CreatorReferenceModule> empty;
    auto it = m_referenceModuleCreators.find(group);
    if (it == m_referenceModuleCreators.end()) return empty;
    return it.value();
}

TabBase* ModuleManager::createTab(const QString& group, const int& index) {
    if (m_tabModuleCreators.contains(group)) {
        const QVector<TabModuleDescription>& tabDescs = m_tabModuleCreators[group];
        if (index >= 0 && index < tabDescs.size()) {
            return tabDescs[index].creator();
        }
    }
    return nullptr;
}

WindowBase* ModuleManager::createWindow(const QString& group, const int& index) {
    if (m_windowModuleCreators.contains(group)) {
        const QVector<CreatorWindowModule>& windowCreators = m_windowModuleCreators[group];
        if (index >= 0 && index < windowCreators.size()) {
            return windowCreators[index]();
        }
    }
    return nullptr;
}

ReferenceBase* ModuleManager::createReference(const QString& group, const int& index) {
    if (m_referenceModuleCreators.contains(group)) {
        const QVector<CreatorReferenceModule>& referenceCreators = m_referenceModuleCreators[group];
        if (index >= 0 && index < referenceCreators.size()) {
            return referenceCreators[index]();
        }
    }
    return nullptr;
}