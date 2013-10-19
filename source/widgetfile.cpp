#include "widgetfile.h"
#include "minisplitter.h"
#include "widgettextedit.h"
#include "widgetconsole.h"
#include "widgetsimpleoutput.h"
#include "widgetfindreplace.h"
#include "widgetpdfdocument.h"
#include "widgetpdfviewer.h"
#include "widgetlinenumber.h"
#include "syntaxhighlighter.h"
#include "file.h"
#include "filemanager.h"
#include "builder.h"
#include "configmanager.h"
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <QAction>
#include <QDebug>

WidgetFile::WidgetFile(QWidget *parent) :
    QWidget(parent)
{
    _widgetTextEdit = new WidgetTextEdit(this);
    _syntaxHighlighter = new SyntaxHighlighter(_widgetTextEdit->document());
    _widgetTextEdit->setSyntaxHighlighter(_syntaxHighlighter);
    _widgetPdfViewer = new WidgetPdfViewer(this);
    _widgetPdfViewer->widgetPdfDocument()->setWidgetTextEdit(_widgetTextEdit);
    _widgetFindReplace = new WidgetFindReplace(_widgetTextEdit);
     this->closeFindReplaceWidget();
    _widgetLineNumber = new WidgetLineNumber(this);
    _widgetLineNumber->setWidgetTextEdit(_widgetTextEdit);
    _widgetTextEdit->setWidgetLineNumber(_widgetLineNumber);
    _widgetConsole = new WidgetConsole();
    _widgetSimpleOutput = new WidgetSimpleOutput(this);
    _widgetSimpleOutput->setWidgetTextEdit(_widgetTextEdit);

    _horizontalSplitter = new MiniSplitter(Qt::Horizontal);
    _verticalSplitter = new MiniSplitter(Qt::Vertical);

    QGridLayout * layout = new QGridLayout();
    layout->addWidget(_widgetLineNumber,0,0);
    layout->addWidget(_horizontalSplitter,0,1);
    layout->setColumnMinimumWidth(0,40);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->setMargin(0);
    this->setLayout(layout);
    this->setContentsMargins(0,8,0,0);

    _horizontalSplitter->addWidget(_verticalSplitter);
    _horizontalSplitter->addWidget(_widgetPdfViewer);
    QList<int> sizes;
    sizes << width()/2 << width()/2;
    _horizontalSplitter->setSizes(sizes);

    _verticalSplitter->addWidget(this->_widgetTextEdit);
    _verticalSplitter->addWidget(this->_widgetFindReplace);
    _verticalSplitter->addWidget(this->_widgetSimpleOutput);
    _verticalSplitter->addWidget(this->_widgetConsole);

    _verticalSplitter->setCollapsible(3,true);
    _verticalSplitter->setCollapsible(2,true);



    connect(_widgetFindReplace->pushButtonClose(), SIGNAL(clicked()), this, SLOT(closeFindReplaceWidget()));
    connect(_widgetTextEdit,SIGNAL(textChanged()),_widgetLineNumber,SLOT(update()));
    connect(_widgetTextEdit->getCurrentFile()->getBuilder(), SIGNAL(pdfChanged()),_widgetPdfViewer->widgetPdfDocument(),SLOT(updatePdf()));
    connect(_widgetTextEdit->getCurrentFile()->getBuilder(), SIGNAL(error()),this,SLOT(openErrorTable()));
    connect(_widgetTextEdit->getCurrentFile()->getBuilder(), SIGNAL(success()),this,SLOT(closeErrorTable()));
    connect(_widgetConsole, SIGNAL(requestLine(int)), _widgetTextEdit, SLOT(goToLine(int)));


    _widgetTextEdit->getCurrentFile()->create();
    _widgetTextEdit->setText(" ");

    _widgetPdfViewer->widgetPdfDocument()->setFile(_widgetTextEdit->getCurrentFile());
    _widgetConsole->setBuilder(_widgetTextEdit->getCurrentFile()->getBuilder());
    _widgetConsole->setMaximumHeight(0);
    _widgetSimpleOutput->setBuilder(_widgetTextEdit->getCurrentFile()->getBuilder());
    _widgetSimpleOutput->setMaximumHeight(0);
    int pos = _widgetTextEdit->textCursor().position();
    _widgetTextEdit->selectAll();
    _widgetTextEdit->textCursor().setBlockCharFormat(ConfigManager::Instance.getTextCharFormats("normal"));
    QTextCursor cur(_widgetTextEdit->textCursor());
    cur.deletePreviousChar();
    _widgetTextEdit->setTextCursor(cur);
    _widgetTextEdit->getCurrentFile()->setModified(false);
}

WidgetFile::~WidgetFile()
{
#ifdef DEBUG_DESTRUCTOR
    qDebug()<<"delete WidgetFile";
#endif
}

void WidgetFile::initTheme()
{
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, ConfigManager::Instance.getTextCharFormats("linenumber").background().color());
    this->setAutoFillBackground(true);
    this->setPalette(Pal);
    _widgetTextEdit->setStyleSheet(QString("QPlainTextEdit { border: 0px solid ")+
                                        ConfigManager::Instance.colorToString(ConfigManager::Instance.getTextCharFormats("textedit-border").foreground().color())+"; "+
                                        QString("color: ")+
                                        ConfigManager::Instance.colorToString(ConfigManager::Instance.getTextCharFormats("normal").foreground().color())+"; "+
                                        QString("background-color: ")+
                                        ConfigManager::Instance.colorToString(ConfigManager::Instance.getTextCharFormats("normal").background().color())+
                                "; }");
    _widgetTextEdit->setCurrentCharFormat(ConfigManager::Instance.getTextCharFormats("normal"));
    QTextCursor cur = _widgetTextEdit->textCursor();
    cur.setCharFormat(ConfigManager::Instance.getTextCharFormats("normal"));
    _widgetTextEdit->setTextCursor(cur);
    //this->widgetTextEdit->setCurrentFont(ConfigManager::Instance.getTextCharFormats("normal").font());


#ifdef OS_MAC
    if(ConfigManager::Instance.getTextCharFormats("normal").background().color().value()<100) // if it's a dark color
    {
        QPixmap whiteBeamPixmap("/Users/quentinbramas/Projects/texiteasy/texiteasy-repository/source/data/cursor/whiteBeam.png");
        QCursor whiteBeam(whiteBeamPixmap);
        _widgetTextEdit->viewport()->setCursor(whiteBeam);
    }
    else
    {
        _widgetTextEdit->viewport()->setCursor(Qt::IBeamCursor);
    }
#endif

    {
        QPalette Pal(palette());
        // set black background
        QTextCharFormat format = ConfigManager::Instance.getTextCharFormats("linenumber");
        QBrush brush = format.background();
        Pal.setColor(QPalette::Background, brush.color());
        _widgetLineNumber->setAutoFillBackground(true);
        _widgetLineNumber->setPalette(Pal);
    }
}


bool WidgetFile::isConsoleOpen()
{
    QList<int> sizes = this->verticalSplitter()->sizes();
    if(sizes[3] > 0)
    {
        return true;
    }
    return false;
}
bool WidgetFile::isErrorTableOpen()
{
    QList<int> sizes = this->verticalSplitter()->sizes();
    if(sizes[2] > 0)
    {
        return true;
    }
    return false;
}
void WidgetFile::openConsole()
{
    if(isConsoleOpen())
    {
        return;
    }
    if(isErrorTableOpen())
    {
        closeErrorTable();
    }
    QList<int> sizes = this->verticalSplitter()->sizes();
    sizes.replace(0, sizes[0] - 60 + sizes[3]);
    sizes.replace(2, 0);
    sizes.replace(3, 60);
    this->verticalSplitter()->widget(3)->setMaximumHeight(height()*2/3);
    this->verticalSplitter()->setSizes(sizes);
    emit verticalSplitterChanged();
}

void WidgetFile::openErrorTable()
{
    if(isErrorTableOpen())
    {
        return;
    }
    if(isConsoleOpen())
    {
        closeConsole();
    }
    QList<int> sizes = this->verticalSplitter()->sizes();
    sizes.replace(0, sizes[0] - 60 + sizes[3]);
    sizes.replace(2, 60);
    sizes.replace(3, 0);
    this->verticalSplitter()->widget(2)->setMaximumHeight(height()*2/3);
    this->verticalSplitter()->setSizes(sizes);
    emit verticalSplitterChanged();
}

void WidgetFile::closeConsole()
{
    if(!isConsoleOpen())
    {
        return;
    }
    QList<int> sizes = this->verticalSplitter()->sizes();
    sizes.replace(0, sizes[0] + sizes[3]);
    sizes.replace(3, 0);
    this->verticalSplitter()->widget(3)->setMaximumHeight(0);
    this->verticalSplitter()->setSizes(sizes);
    emit verticalSplitterChanged();
}

void WidgetFile::closeErrorTable()
{
    if(!isErrorTableOpen())
    {
        return;
    }
    QList<int> sizes = this->verticalSplitter()->sizes();
    sizes.replace(0, sizes[0] + sizes[2]);
    sizes.replace(2, 0);
    this->verticalSplitter()->widget(2)->setMaximumHeight(0);
    this->verticalSplitter()->setSizes(sizes);
    emit verticalSplitterChanged();
}

void WidgetFile::openFindReplaceWidget()
{
    this->_widgetFindReplace->setMaximumHeight(110);
    this->_widgetFindReplace->setMinimumHeight(110);
    this->_widgetFindReplace->open();
}

void WidgetFile::closeFindReplaceWidget()
{
    this->_widgetFindReplace->setMaximumHeight(0);
    this->_widgetFindReplace->setMinimumHeight(0);
}

void WidgetFile::toggleConsole()
{
    if(isConsoleOpen())
    {
        closeConsole();
    }
    else
    {
        openConsole();
    }
}
void WidgetFile::toggleErrorTable()
{
    if(isErrorTableOpen())
    {
        closeErrorTable();
    }
    else
    {
        openErrorTable();
    }
}
void WidgetFile::pdflatex()
{
    this->save();
    _widgetTextEdit->getCurrentFile()->getBuilder()->pdflatex();
}
void WidgetFile::bibtex()
{
    this->save();
    _widgetTextEdit->getCurrentFile()->getBuilder()->bibtex();
}
bool WidgetFile::isEmpty()
{
    return !_widgetTextEdit->getCurrentFile()->isModified() && _widgetTextEdit->toPlainText().isEmpty();
}

void WidgetFile::save()
{
    if(_widgetTextEdit->getCurrentFile()->getFilename().isEmpty())
    {
        return this->saveAs();
    }
    _widgetTextEdit->getCurrentFile()->setData(_widgetTextEdit->toPlainText());
    _widgetTextEdit->getCurrentFile()->save();
    //this->statusBar()->showMessage(tr(QString::fromUtf8("Sauvegardé").toLatin1()),2000);
}

void WidgetFile::saveAs()
{
    //this->widgetTextEdit->getCurrentFile()->setData("sdfsdfg");
    //return;
    QString filename = QFileDialog::getSaveFileName(this,tr("Enregistrer un fichier"));
    if(filename.isEmpty())
    {
        return;
    }
    QString oldFilename = _widgetTextEdit->getCurrentFile()->getFilename();
    _widgetTextEdit->getCurrentFile()->setData(_widgetTextEdit->toPlainText());
    _widgetTextEdit->getCurrentFile()->save(filename);

    if(oldFilename.compare(filename))
    {
        emit FileManager::Instance.sendFilenameChanged(this, filename);
    }
}

void WidgetFile::open(QString filename)
{
    _widgetTextEdit->getCurrentFile()->open(filename);
    _widgetPdfViewer->widgetPdfDocument()->setFile(_widgetTextEdit->getCurrentFile());
    _widgetConsole->setBuilder(_widgetTextEdit->getCurrentFile()->getBuilder());
    foreach(AssociatedFile associatedfile,  _widgetTextEdit->getCurrentFile()->associatedFiles())
    {
        QString basename = associatedfile.filename;
        basename.replace(QRegExp("^.*[\\\\\\//]([^\\\\\\//]+)$"), "\\1");
        QAction * a = new QAction(trUtf8("Ouvrir ") + basename, this);
        a->setProperty("filename", associatedfile.filename);
        connect(a, SIGNAL(triggered()), &FileManager::Instance, SLOT(openAssociatedFile()));
        this->addAction(a);
    }
}
