#ifndef ASCIICHARSREF_H
#define ASCIICHARSREF_H

#include "core/modules/ReferenceBase.h"
#include <QWidget>

class AsciiCharsRef : public ReferenceBase
{
    Q_OBJECT
public:
    explicit AsciiCharsRef();
    QString name() override { return "ASCII / Unicode Characters"; }

private:
    void initWindow() override;
    void initWidgets() override;

signals:
};

#endif // ASCIICHARSREF_H
