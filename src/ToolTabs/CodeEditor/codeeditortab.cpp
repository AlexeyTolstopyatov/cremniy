#include "codeeditortab.h"
#include "QCodeEditor.hpp"
#include <qboxlayout.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstackedlayout.h>
#include <QToolTip>
#include <QHelpEvent>
#include <QTextCursor>
#include "filemanager.h"
#include "utils.h"
#include "utils/instructionhelpservice.h"

#include "core/ToolTabFactory.h"

static bool registered = [](){
    ToolTabFactory::instance().registerTab("1", [](){
        return new CodeEditorTab();
    });
    return true;
}();

CodeEditorTab::CodeEditorTab(QWidget *parent)
    : ToolTab{parent}
{

    // - - Create "Code Editor" Page - -

    m_codeEditorWidget = new QCodeEditor(this);
    m_codeEditorWidget->viewport()->setMouseTracking(true);
    m_codeEditorWidget->viewport()->installEventFilter(this);
    connect(m_codeEditorWidget, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
        if (!m_codeEditorWidget || !m_codeEditorWidget->hasFocus())
            return;
        const QPoint p = m_codeEditorWidget->cursorRect().bottomRight();
        showInstructionHelpAt(p, true);
    });

    QTextOption opt = m_codeEditorWidget->document()->defaultTextOption();
    opt.setTabStopDistance(20);
    m_codeEditorWidget->document()->setDefaultTextOption(opt);

    m_codeEditorWidget->document()->markContentsDirty(0, m_codeEditorWidget->document()->characterCount());
    m_codeEditorWidget->viewport()->update();

    // - - Create "Binary File Detected" Page - -

    m_overlayWidget = new QWidget(this);

    auto overlayLayout = new QVBoxLayout(m_overlayWidget);
    overlayLayout->setAlignment(Qt::AlignCenter);

    QLabel* title = new QLabel("Binary file detected");
    title->setStyleSheet("color: white; font-size: 20px;");
    title->setAlignment(Qt::AlignCenter);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    overlayLayout->addWidget(title);
    overlayLayout->addSpacing(15);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setAlignment(Qt::AlignCenter);

    QPushButton* anywayOpenBtn = new QPushButton("Open anyway");

    btnLayout->addWidget(anywayOpenBtn);
    overlayLayout->addLayout(btnLayout);

    // - - Create and Init Stacked Layout Widget - -

    auto stack = new QStackedLayout;
    stack->setStackingMode(QStackedLayout::StackAll);
    stack->addWidget(m_codeEditorWidget);
    stack->addWidget(m_overlayWidget);

    m_overlayWidget->hide();

    this->setLayout(stack);

    // - - Connects - -

    // Trigger: Menu Bar: View->wordWrap - setWordWrapMode
    // connect(GlobalWidgetsManager::instance().get_IDEWindow_menuBar_view_wordWrap(),
    //         &QAction::changed,
    //         this, [this]{
    //             if (GlobalWidgetsManager::instance().get_IDEWindow_menuBar_view_wordWrap()->isChecked())
    //                 m_codeEditorWidget->setWordWrapMode(QTextOption::WordWrap);
    //             else
    //                 m_codeEditorWidget->setWordWrapMode(QTextOption::NoWrap);
    //         });

    // Clicked: Open File Anyway Button
    connect(anywayOpenBtn, &QPushButton::clicked, this, [this]{
        forceSetData = true;
        this->setTabData();
    });

    // modificationChanged: signal modifyData
    connect(m_codeEditorWidget->document(),
            &QTextDocument::modificationChanged,
            this,
            [this](bool modified){
                if (!m_codeEditorWidget->m_ignoreModification)
                    setModifyIndicator(true);
                    emit modifyData();
            });

    // ContentsChanged: if new hash == old hash: dataEqual, else: signal modifyData
    connect(m_codeEditorWidget->document(),
            &QTextDocument::contentsChanged,
            this,
            [this](){
                QByteArray data = m_codeEditorWidget->getBData();
                uint newDataHash = qHash(data, 0);
                if (m_dataHash == newDataHash) {
                    setModifyIndicator(false);
                    emit dataEqual();
                }
                else{
                    if (!m_codeEditorWidget->m_ignoreModification)
                        setModifyIndicator(true);
                        emit modifyData();
                }
            });

}

bool CodeEditorTab::eventFilter(QObject *watched, QEvent *event)
{
    if (m_codeEditorWidget && watched == m_codeEditorWidget->viewport() && event) {
        if (event->type() == QEvent::ToolTip) {
            auto *helpEvent = static_cast<QHelpEvent *>(event);
            showInstructionHelpAt(helpEvent->pos(), false);
            return true;
        }
    }
    return ToolTab::eventFilter(watched, event);
}

void CodeEditorTab::showInstructionHelpAt(const QPoint &pos, bool forceByCursor)
{
    if (!m_codeEditorWidget)
        return;

    QTextCursor c = forceByCursor ? m_codeEditorWidget->textCursor() : m_codeEditorWidget->cursorForPosition(pos);
    c.select(QTextCursor::WordUnderCursor);
    const QString token = c.selectedText().trimmed();
    const QString line = c.block().text();

    QString tip = InstructionHelpService::instance().tooltipForToken(token, line);
    if (tip.isEmpty())
        tip = InstructionHelpService::instance().tooltipForLine(line);

    if (tip.isEmpty()) {
        QToolTip::hideText();
        return;
    }

    const QPoint globalPos = m_codeEditorWidget->viewport()->mapToGlobal(
        forceByCursor ? m_codeEditorWidget->cursorRect().bottomRight() : pos);
    QToolTip::showText(globalPos, tip, m_codeEditorWidget->viewport());
}

// - - override functions - -

// - public slots -

void CodeEditorTab::setFile(QString filepath){
    m_fileContext = new FileContext(filepath);
    QFileInfo fileInfo(filepath);
    QString ext = (fileInfo.suffix()).toLower();
    m_codeEditorWidget->setFileExt(ext);
}

void CodeEditorTab::setTabData(){

    qDebug() << "CodeEditorTab: setTabData";

    QByteArray data = FileManager::openFile(m_fileContext);

    if (isBinary(data) && !forceSetData){
        m_codeEditorWidget->hide();
        m_overlayWidget->show();
    }
    else{
        m_dataHash = qHash(data, 0);
        m_codeEditorWidget->show();
        m_overlayWidget->hide();
        m_codeEditorWidget->setBData(data);
        forceSetData = false;
    }

    setModifyIndicator(false);
    emit dataEqual();
}

void CodeEditorTab::saveTabData() {
    qDebug() << "CodeEditorTab: saveTabData";

    QByteArray data = m_codeEditorWidget->getBData();
    uint newDataHash = qHash(data, 0);
    if (newDataHash == m_dataHash) return;
    m_dataHash = newDataHash;

    FileManager::saveFile(m_fileContext, &data);

    m_codeEditorWidget->document()->setModified(false);

    setModifyIndicator(false);
    emit dataEqual();
    emit refreshDataAllTabsSignal();
}