/***************************************************************************
 *   copyright       : (C) 2013 by Quentin BRAMAS                          *
 *   http://texiteasy.com                                                  *
 *                                                                         *
 *   This file is part of texiteasy.                                          *
 *                                                                         *
 *   texiteasy is free software: you can redistribute it and/or modify        *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   texiteasy is distributed in the hope that it will be useful,             *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with texiteasy.  If not, see <http://www.gnu.org/licenses/>.       *                         *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H


#include <QObject>
#include <QMap>
#include <QString>
#include <QTextCharFormat>
#include <QSettings>
#include <QDebug>
#include <QMutex>
#include <QDesktopServices>
#include <QUrl>

class QWidget;

class ConfigManager : public QObject
{
    Q_OBJECT

public:

    static ConfigManager Instance;

    QTextCharFormat getTextCharFormats(QString key = QString("normal"))
    {
        //_charFormatMutex.lock();
        QTextCharFormat format(this->textCharFormats->value(key,this->textCharFormats->value("normal")));
        //_charFormatMutex.unlock();
        return format;
    }

    ~ConfigManager();

    int tabWidth() { QSettings settings; return settings.value("tabWidth", 4).toInt(); }
    void setTabWidth(int tabW) { QSettings settings; settings.setValue("tabWidth", tabW); emit tabWidthChanged(); }

    bool isUsingSpaceIndentation() { QSettings settings; return settings.value("spaceIndentation", true).toBool(); }
    void setUsingSpaceIndentation(bool use) { QSettings settings; settings.setValue("spaceIndentation", use); }
    QString tabToString()
    {
        QSettings settings;
        if(settings.value("spaceIndentation", true).toBool())
        {
            return QString().fill(' ',settings.value("tabWidth", 4).toInt());
        }
        return QString("\t");
    }

    bool darkTheme() { return 120 > this->getTextCharFormats().background().color().value(); }

    void changePointSizeBy(int delta);
    void setPointSize(int size);
    void setReplaceDefaultFont(bool replace);
    bool isDefaultFontReplaced() { QSettings settings; return settings.value("theme/replaceDefaultFont").toBool(); }
    int pointSize() { QSettings settings; return settings.value("theme/pointSize").toInt(); }
    void setFontFamily(QString family);

    void setMainWindow(QWidget * mainWindow);
    void save(void);
    bool load(QString theme = QString());
    QString colorToString(const QColor & color) { return "rgb("+QString::number(color.red())+","+QString::number(color.green())+","+QString::number(color.blue())+")";}

    QString textCharFormatToString(QTextCharFormat charFormat, QTextCharFormat defaultFormat = QTextCharFormat());
    QTextCharFormat stringToTextCharFormat(QString string, QTextCharFormat defaultFormat = QTextCharFormat());

    QStringList themesList();
    const QString& theme() { return _theme; }

    QStringList languagesList();
    QString language() { QSettings settings; return settings.value("language").toString(); }
    void setLanguage(QString language) { QSettings settings; settings.setValue("language", language); applyTranslation(); }
    void applyTranslation();

    void checkRevision();

    QString bibtexCommand(bool fullPath = false) { QSettings settings; return (fullPath ? settings.value("builder/latexPath").toString() : QString(""))+settings.value("builder/bibtex").toString(); }
    QString pdflatexCommand(bool fullPath = false) { QSettings settings; return (fullPath ? settings.value("builder/latexPath").toString() : QString(""))+settings.value("builder/pdflatex").toString(); }
    QString latexCommand(bool fullPath = false) { QSettings settings; return (fullPath ? settings.value("builder/latexPath").toString() : QString(""))+settings.value("builder/latex").toString(); }
    QString latexPath() { QSettings settings; return settings.value("builder/latexPath").toString(); }

    QString commandDatabaseFilename() { QSettings settings; return settings.value("commandDatabaseFilename").toString(); }

    void setBibtexCommand(QString command) { QSettings settings; settings.setValue("builder/bibtex", command); }
    void setPdflatexCommand(QString command) { QSettings settings; settings.setValue("builder/pdflatex", command); }
    void setLatexPath(QString path) { QSettings settings; settings.setValue("builder/latexPath", path); }

    bool isPdfSynchronized() { QSettings settings; return settings.value("pdfSynchronized", true).toBool(); }

    bool isThisVersionHaveToBeReminded(QString version);
    void dontRemindMeThisVersion(QString version);

    QString lastFolder() { QSettings settings; return settings.value("lastFolder").toString(); }

    static QString Extensions;

    void init();
public slots:
    /**
     * @brief signalVersionIsOutdated
     * call this function to emit the versionIsOutdated() signal
     */
    void signalVersionIsOutdated() { emit versionIsOutdated(); }

    void setPdfSynchronized(bool pdfSynchronized) { QSettings settings; settings.setValue("pdfSynchronized", pdfSynchronized); }
    void openThemeFolder();
    void openUpdateWebsite() { QString link = TEXITEASY_UPDATE_WEBSITE;
                               QDesktopServices::openUrl(QUrl(link)); }

signals:

    /**
     * @brief versionIsOutdated [signal]
     * emitted when we detect that the version is outdated
     */
    void versionIsOutdated();
    void tabWidthChanged();
private:
    void replaceDefaultFont();
    ConfigManager();

    QApplication * _application;
    QMutex _charFormatMutex;
    QWidget * mainWindow;
    QMap<QString,QTextCharFormat> * textCharFormats;
    QString _theme;
    QString _pdflatexExe;
};



#endif // CONFIGMANAGER_H
