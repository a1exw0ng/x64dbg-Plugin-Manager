// Copyright (c) 2019-2020 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "consoleoutput.h"
#include "../global.h"
#include "../utils.h"
#include "../createmoduleprocess.h"
#include "../getfilefromserverprocess.h"
#include "../installmoduleprocess.h"
#include "../removemoduleprocess.h"

void installModules(XPLUGINMANAGER::OPTIONS *pOptions,Utils::MODULES_DATA *pModulesData,ConsoleOutput *pConsoleOutput,QList<QString> *pListModuleNames)
{
    int nCount=pListModuleNames->count();

    for(int i=0;i<nCount;i++)
    {
        Utils::MDATA mdata=Utils::getMDataByName(&(pModulesData->listServerList),pListModuleNames->at(i));

        if(mdata.sName!="")
        {
            QString sFileName=Utils::getModuleFileName(pOptions,mdata.sName);

            if(!XBinary::isFileHashValid(XBinary::HASH_SHA1,sFileName,mdata.sSHA1))
            {
                Utils::WEB_RECORD record={};

                record.sFileName=sFileName;
                record.sLink=mdata.sSrc;

                GetFileFromServerProcess getFileFromServerProcess;
                QObject::connect(&getFileFromServerProcess,SIGNAL(infoMessage(QString)),pConsoleOutput,SLOT(infoMessage(QString)));
                QObject::connect(&getFileFromServerProcess,SIGNAL(errorMessage(QString)),pConsoleOutput,SLOT(errorMessage(QString)));
                getFileFromServerProcess.setData(QList<Utils::WEB_RECORD>()<<record);

                getFileFromServerProcess.process();
            }

            if(XBinary::isFileHashValid(XBinary::HASH_SHA1,sFileName,mdata.sSHA1))
            {
                InstallModuleProcess installModuleProcess;
                QObject::connect(&installModuleProcess,SIGNAL(infoMessage(QString)),pConsoleOutput,SLOT(infoMessage(QString)));
                QObject::connect(&installModuleProcess,SIGNAL(errorMessage(QString)),pConsoleOutput,SLOT(errorMessage(QString)));
                installModuleProcess.setData(pOptions,QList<QString>()<<sFileName);

                installModuleProcess.process();
            }
            else
            {
                pConsoleOutput->errorMessage(QString("Invalid SHA1: %1").arg(sFileName));
            }
        }
        else
        {
            pConsoleOutput->errorMessage(QString("Invalid name: %1").arg(pListModuleNames->at(i)));
        }
    }
}

void removeModules(XPLUGINMANAGER::OPTIONS *pOptions,Utils::MODULES_DATA *pModulesData,ConsoleOutput *pConsoleOutput,QList<QString> *pListModuleNames)
{
    int nCount=pListModuleNames->count();

    for(int i=0;i<nCount;i++)
    {
        Utils::MDATA mdata=Utils::getMDataByName(&(pModulesData->listInstalled),pListModuleNames->at(i));

        if(mdata.sName!="")
        {
            RemoveModuleProcess removeModuleProcess;
            QObject::connect(&removeModuleProcess,SIGNAL(infoMessage(QString)),pConsoleOutput,SLOT(infoMessage(QString)));
            QObject::connect(&removeModuleProcess,SIGNAL(errorMessage(QString)),pConsoleOutput,SLOT(errorMessage(QString)));
            removeModuleProcess.setData(pOptions,QList<QString>()<<mdata.sName);

            removeModuleProcess.process();
        }
        else
        {
            pConsoleOutput->errorMessage(QString("Invalid name: %1. This module is not installed.").arg(pListModuleNames->at(i)));
        }
    }
}

void showModules(ConsoleOutput *pConsoleOutput,QList<Utils::MDATA> *pList)
{
    int nCount=pList->count();

    for(int i=0;i<nCount;i++)
    {
        QString sString=QString("%1 [%2]").arg(pList->at(i).sName).arg(pList->at(i).sVersion);

        pConsoleOutput->infoMessage(sString);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(X_ORGANIZATIONNAME);
    QCoreApplication::setOrganizationDomain(X_ORGANIZATIONDOMAIN);
    QCoreApplication::setApplicationName(X_APPLICATIONNAME);
    QCoreApplication::setApplicationVersion(X_APPLICATIONVERSION);

    QCoreApplication app(argc, argv);

    XPLUGINMANAGER::OPTIONS options={};

    Utils::loadOptions(&options);

    ConsoleOutput consoleOutput;
    QCommandLineParser parser;
    QString sDescription;
    sDescription.append(QString("%1 v%2\n").arg(X_APPLICATIONNAME).arg(X_APPLICATIONVERSION));
    sDescription.append(QString("%1\n").arg("Copyright(C) 2019 hors<horsicq@gmail.com> Web: http://ntinfo.biz"));
    parser.setApplicationDescription(sDescription);
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("plugin","The plugin to open.");

    QCommandLineOption clSetGlobalRootPath          (QStringList()<<"G"<<"setglobalrootpath",   "Set a global root path<path>.",                "path");
    QCommandLineOption clSetGlobalDataPath          (QStringList()<<"D"<<"setglobaldatapath",   "Set a global data path<path>.",                "path");
    QCommandLineOption clSetGlobalJSONLink          (QStringList()<<"J"<<"setglobaljsonlink",   "Set a global JSON link<link>.",                "link");
    QCommandLineOption clCreatePlugin               (QStringList()<<"c"<<"createplugin",        "Create a plugin<name>.",                       "name");
    QCommandLineOption clCreateServerList           (QStringList()<<"l"<<"createserverlist",    "Create a serverlist<name>.",                   "name");
    QCommandLineOption clInstallPlugin              (QStringList()<<"i"<<"installplugin",       "Install plugin(s)."                            );
    QCommandLineOption clUpdatePlugin               (QStringList()<<"u"<<"updateplugin",        "Update plugin(s)."                             );
    QCommandLineOption clRemovePlugin               (QStringList()<<"m"<<"removeplugin",        "Remove plugin(s)."                             );
    QCommandLineOption clUpdateServerList           (QStringList()<<"U"<<"updateserverlist",    "Update server list."                           );
    QCommandLineOption clUpdateAllInstalledPlugins  (QStringList()<<"A"<<"updateall",           "Update all installed plugins."                 );
    QCommandLineOption clShowServerList             (QStringList()<<"S"<<"showserverlist",      "Show server list."                             );
    QCommandLineOption clShowInstalled              (QStringList()<<"N"<<"showinstalled",       "Show installed."                               );
    QCommandLineOption clShowUpdates                (QStringList()<<"P"<<"showupdates",         "Show updates."                                 );
    QCommandLineOption clSetRootPath                (QStringList()<<"r"<<"setrootpath",         "Set a root path<path>.",                       "path");
    QCommandLineOption clSetName                    (QStringList()<<"n"<<"setname",             "Set a name of plugin<name>.",                  "name");
    QCommandLineOption clSetVersion                 (QStringList()<<"V"<<"setversion",          "Set a version of plugin<version>.",            "version");
    QCommandLineOption clSetDate                    (QStringList()<<"d"<<"setdate",             "Set a date of plugin<date>.",                  "date");
    QCommandLineOption clSetAuthor                  (QStringList()<<"a"<<"setauthor",           "Set an author of plugin<author>.",             "author");
    QCommandLineOption clSetBugreport               (QStringList()<<"b"<<"setbugreport",        "Set a bugreport of plugin<bugreport>.",        "bugreport");
    QCommandLineOption clSetInfo                    (QStringList()<<"I"<<"setinfo",             "Set an info of plugin<info>.",                 "info");
    QCommandLineOption clSetWebPrefix               (QStringList()<<"p"<<"setwebprefix",        "Set a webprefix<prefix>.",                     "prefix");

    parser.addOption(clSetGlobalRootPath);
    parser.addOption(clSetGlobalDataPath);
    parser.addOption(clSetGlobalJSONLink);
    parser.addOption(clCreatePlugin);
    parser.addOption(clCreateServerList);
    parser.addOption(clInstallPlugin);
    parser.addOption(clUpdatePlugin);
    parser.addOption(clRemovePlugin);
    parser.addOption(clUpdateServerList);
    parser.addOption(clUpdateAllInstalledPlugins);
    parser.addOption(clShowServerList);
    parser.addOption(clShowInstalled);
    parser.addOption(clShowUpdates);
    parser.addOption(clSetRootPath);
    parser.addOption(clSetName);
    parser.addOption(clSetVersion);
    parser.addOption(clSetDate);
    parser.addOption(clSetAuthor);
    parser.addOption(clSetBugreport);
    parser.addOption(clSetInfo);
    parser.addOption(clSetWebPrefix);

    parser.process(app);

    bool bProcess=false;

    bool bIsSetGlobalRootPath=parser.isSet(clSetGlobalRootPath);
    bool bIsSetGlobalDataPath=parser.isSet(clSetGlobalDataPath);
    bool bIsSetGlobalJSONLink=parser.isSet(clSetGlobalJSONLink);

    if(bIsSetGlobalRootPath||bIsSetGlobalDataPath||bIsSetGlobalJSONLink)
    {
        bProcess=true;

        if(bIsSetGlobalRootPath)
        {
            options.sRootPath=parser.value(clSetGlobalRootPath);

            if(options.sRootPath!="")
            {
                XBinary::createDirectory(XBinary::convertPathName(options.sRootPath));
            }

            if(XBinary::isDirectoryExists(XBinary::convertPathName(options.sRootPath)))
            {
                consoleOutput.infoMessage(QString("Set a global root path: %1").arg(options.sRootPath));
            }
            else
            {
                options.sRootPath="";
                consoleOutput.errorMessage(QString("Invalid root path: %1").arg(options.sRootPath));
            }
        }
        if(bIsSetGlobalDataPath)
        {
            options.sDataPath=parser.value(clSetGlobalDataPath);

            if(options.sDataPath!="")
            {
                XBinary::createDirectory(XBinary::convertPathName(options.sDataPath));
            }

            if(XBinary::isDirectoryExists(XBinary::convertPathName(options.sDataPath)))
            {
                consoleOutput.infoMessage(QString("Set a global data path: %1").arg(options.sDataPath));
            }
            else
            {
                options.sDataPath="";
                consoleOutput.errorMessage(QString("Invalid data path: %1").arg(options.sDataPath));
            }
        }
        if(bIsSetGlobalJSONLink)
        {
            options.sJSONLink=parser.value(clSetGlobalJSONLink);
            consoleOutput.infoMessage(QString("Set a global JSON link: %1").arg(options.sJSONLink));
        }

        Utils::saveOptions(&options);
    }

    if(parser.isSet(clSetRootPath))
    {
        options.sRootPath=parser.value(clSetRootPath);
    }

    bool bRootPathPresent=false;
    bool bDataPathPresent=false;

    if(options.sRootPath!="")
    {
        XBinary::createDirectory(XBinary::convertPathName(options.sRootPath));
        bRootPathPresent=XBinary::isDirectoryExists(XBinary::convertPathName(options.sRootPath));
    }

    if(options.sDataPath!="")
    {
        XBinary::createDirectory(XBinary::convertPathName(options.sDataPath));
        bRootPathPresent=XBinary::isDirectoryExists(XBinary::convertPathName(options.sDataPath));
    }

    if(!bRootPathPresent)
    {
        consoleOutput.errorMessage(QString("Invalid root path: %1").arg(options.sRootPath));
    }

    if(!bDataPathPresent)
    {
        consoleOutput.errorMessage(QString("Invalid data path: %1").arg(options.sDataPath));
    }

    if(parser.isSet(clCreatePlugin))
    {
        bProcess=true;

        Utils::MDATA mdata={};

        mdata.sBundleFileName=parser.value(clCreatePlugin);
        consoleOutput.infoMessage(QString("Create a plugin: %1").arg(mdata.sBundleFileName));

        if(mdata.sBundleFileName!="")
        {
            mdata.sBundleFileName+=".x64dbg.zip";
        }

        mdata.sRoot=parser.value(clSetRootPath);
        mdata.sName=parser.value(clSetName);
        mdata.sVersion=parser.value(clSetVersion);
        mdata.sDate=parser.value(clSetDate);
        mdata.sAuthor=parser.value(clSetAuthor);
        mdata.sBugreport=parser.value(clSetBugreport);
        mdata.sInfo=parser.value(clSetInfo);

        QString sErrorString;

        if(Utils::checkMData(&mdata,&sErrorString))
        {
            CreateModuleProcess createModuleProcess;
            QObject::connect(&createModuleProcess,SIGNAL(infoMessage(QString)),&consoleOutput,SLOT(infoMessage(QString)));
            QObject::connect(&createModuleProcess,SIGNAL(errorMessage(QString)),&consoleOutput,SLOT(errorMessage(QString)));
            createModuleProcess.setData(&mdata);
            createModuleProcess.process();
        }
        else
        {
            consoleOutput.errorMessage(sErrorString);
        }
    }
    else if(parser.isSet(clCreateServerList))
    {
        bProcess=true;

        QString sListName=parser.value(clCreateServerList);
        consoleOutput.infoMessage(QString("Create a serverlist: %1").arg(sListName));
        QString sWebPrefix=parser.value(clSetWebPrefix);
        QString sDate=parser.value(clSetDate);

        if(sDate=="")
        {
            sDate=QDate::currentDate().toString("yyyy-MM-dd");
        }

        if(sListName!="")
        {
            QList<QString> listFiles=parser.positionalArguments();

            if(listFiles.count())
            {
                if(!Utils::createServerList(sListName,&listFiles,sWebPrefix,sDate))
                {
                    consoleOutput.errorMessage("Cannot create serverlist.");
                }
            }
            else
            {
                consoleOutput.errorMessage("No input files.");
            }
        }
        else
        {
            consoleOutput.errorMessage("List name is empty.");
        }
    }
    else if(parser.isSet(clUpdateServerList))
    {
        bProcess=true;

        consoleOutput.infoMessage(QString("Update server list."));

        Utils::WEB_RECORD record={};

        record.sFileName=Utils::getServerListFileName(&options);
        record.sLink=options.sJSONLink;

        GetFileFromServerProcess getFileFromServerProcess;
        QObject::connect(&getFileFromServerProcess,SIGNAL(infoMessage(QString)),&consoleOutput,SLOT(infoMessage(QString)));
        QObject::connect(&getFileFromServerProcess,SIGNAL(errorMessage(QString)),&consoleOutput,SLOT(errorMessage(QString)));
        getFileFromServerProcess.setData(QList<Utils::WEB_RECORD>()<<record);

        getFileFromServerProcess.process();
    }
    else if(parser.isSet(clInstallPlugin)||
            parser.isSet(clUpdatePlugin)||
            parser.isSet(clRemovePlugin)||
            parser.isSet(clUpdateAllInstalledPlugins)||
            parser.isSet(clShowServerList)||
            parser.isSet(clShowInstalled)||
            parser.isSet(clShowUpdates))
    {
        bProcess=true;

        if(bRootPathPresent&&bDataPathPresent)
        {
            Utils::MODULES_DATA modulesData=Utils::getModulesData(&options);

            if(parser.isSet(clShowServerList))
            {
                if(modulesData.listServerList.count())
                {
                    consoleOutput.infoMessage(QString("Show server list."));

                    showModules(&consoleOutput,&modulesData.listServerList);
                }
                else
                {
                    consoleOutput.infoMessage(QString("Server list is empty. Please update server list('-U' or '--updateserverlist')"));
                }
            }
            else if(parser.isSet(clShowInstalled))
            {
                if(modulesData.listInstalled.count())
                {
                    consoleOutput.infoMessage(QString("Show installed."));

                    showModules(&consoleOutput,&modulesData.listInstalled);
                }
                else
                {
                    consoleOutput.infoMessage(QString("No plugins installed"));
                }
            }
            else if(parser.isSet(clShowUpdates))
            {
                if(modulesData.listUpdates.count())
                {
                    consoleOutput.infoMessage(QString("Show updates."));

                    int nCount=modulesData.listUpdates.count();

                    for(int i=0;i<nCount;i++)
                    {
                        consoleOutput.infoMessage(modulesData.listUpdates.at(i).sName);
                    }
                }
                else
                {
                    consoleOutput.infoMessage(QString("No updates available."));
                }
            }
            else if(parser.isSet(clInstallPlugin))
            {
                consoleOutput.infoMessage(QString("Install plugin(s)."));

                QList<QString> listModules=parser.positionalArguments();

                installModules(&options,&modulesData,&consoleOutput,&listModules);
            }
            else if(parser.isSet(clUpdatePlugin))
            {
                consoleOutput.infoMessage(QString("Update plugin(s)."));

                QList<QString> listModules=parser.positionalArguments();
                QList<QString> _listModules;

                int nCount=listModules.count();

                for(int i=0;i<nCount;i++)
                {
                    Utils::WEB_RECORD webRecord=Utils::getWebRecordByName(&(modulesData.listUpdates),listModules.at(i));

                    if(webRecord.sName!="")
                    {
                        _listModules.append(webRecord.sName);
                    }
                    else
                    {
                         consoleOutput.errorMessage(QString("No update available for module: %1").arg(listModules.at(i)));
                    }
                }

                installModules(&options,&modulesData,&consoleOutput,&_listModules);
            }
            else if(parser.isSet(clRemovePlugin))
            {
                consoleOutput.infoMessage(QString("Remove plugin(s)."));

                QList<QString> listModules=parser.positionalArguments();

                removeModules(&options,&modulesData,&consoleOutput,&listModules);
            }
            else if(parser.isSet(clUpdateAllInstalledPlugins))
            {
                if(modulesData.listUpdates.count())
                {
                    consoleOutput.infoMessage(QString("Update all installed plugins"));

                    QList<QString> listModules=Utils::getNamesFromWebRecords(&(modulesData.listUpdates));

                    installModules(&options,&modulesData,&consoleOutput,&listModules);
                }
                else
                {
                    consoleOutput.infoMessage(QString("No updates available."));
                }
            }
        }
    }

    if(!bProcess)
    {
        parser.showHelp();
        Q_UNREACHABLE();
    }

    return 0;
}