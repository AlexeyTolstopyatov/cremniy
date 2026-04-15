#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "core/modules/ReferenceBase.h"
#include "core/modules/TabBase.h"
#include "core/modules/WindowBase.h"

using CreatorTabModule = std::function<TabBase*()>;
using CreatorWindowModule = std::function<WindowBase*()>;
using CreatorReferenceModule = std::function<ReferenceBase*()>;

struct TabModuleDescription {
    CreatorTabModule creator;
    int position;
};

class ModuleManager {

public:
    static ModuleManager& instance();

    void registerTab(const QString& id, CreatorTabModule creator, const int& position);
    void registerWindow(const QString& id, CreatorWindowModule creator);
    void registerReference(const QString& id, CreatorReferenceModule creator);

    QList<QString> getTabGroups() const;
    QList<QString> getWindowGroups() const;
    QList<QString> getReferenceGroups() const;

    const QVector<TabModuleDescription>& getTabsByGroup(const QString& group) const;
    const QVector<CreatorWindowModule>& getWindowsByGroup(const QString& group) const;
    const QVector<CreatorReferenceModule>& getReferencesByGroup(const QString& group) const;

    TabBase* createTab(const QString& group, const int& index);
    WindowBase* createWindow(const QString& group, const int& index);
    ReferenceBase* createReference(const QString& group, const int& index);

private:

    QHash<QString, QVector<TabModuleDescription>> m_tabModuleCreators;
    QHash<QString, QVector<CreatorWindowModule>> m_windowModuleCreators;
    QHash<QString, QVector<CreatorReferenceModule>> m_referenceModuleCreators;

};

#endif // MODULEMANAGER_H
