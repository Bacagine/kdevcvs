/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsstatusjob.h"
#include "debug.h"

#include <QUrl>
#include <QDir>
#include <QStringList>

#include <vcs/vcsstatusinfo.h>


CvsStatusJob::CvsStatusJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : CvsJob(parent, verbosity)
{
}

CvsStatusJob::~CvsStatusJob()
{
}

QVariant CvsStatusJob::fetchResults()
{
    // Convert job's output into KDevelop::VcsStatusInfo
    QList<QVariant> infos;
    parseOutput(output(), infos);

    return infos;
}

void CvsStatusJob::addInfoToList(QList<QVariant>& infos,
        const QString& currentDir, const QString& filename,
        const QString& statusString)
{
    KDevelop::VcsStatusInfo::State cvsState = String2EnumState( statusString );

    QString correctedFilename = filename;
    if (cvsState == KDevelop::VcsStatusInfo::ItemDeleted) {
        // cvs status writes "no file" in front of the filename
        // in case the file was locally removed
        correctedFilename.remove(QStringLiteral("no file "));
    }

    // join the current directory (if any) and the found filename ...
    // note: current directy is always relative to the directory where the
    //       cvs command was executed
    QString file = currentDir;
    if (file.length() > 0) {
        file += QDir::separator();
    }
    file += correctedFilename;

    // ... and create a VcsFileInfo entry
    KDevelop::VcsStatusInfo info;
    info.setUrl(QUrl::fromLocalFile(QString(getDirectory() + QDir::separator() + file)));
    info.setState(cvsState);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qCDebug(PLUGIN_CVS) << "Added status of: " << info.url() << endl;
    infos << qVariantFromValue( info );
#else
    qCDebug(PLUGIN_CVS) << "Added status of: " << info.url() << Qt::endl;
    infos << QVariant::fromValue( info );
#endif
}

void CvsStatusJob::parseOutput(const QString& jobOutput, QList<QVariant>& infos)
{
    QString filename;
    QString status;
    QString reporev;
    QString workrev;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    static QRegExp re_start(QStringLiteral("^=+$"));
    static QRegExp re_file(QStringLiteral("File:\\s+(.*)\\s+Status:\\s+(.*)"));
    static QRegExp re_workrev(QStringLiteral("\\s+Working revision:\\s+([\\d\\.]*).*"));
    static QRegExp re_reporev(QStringLiteral("\\s+Repository revision:\\s+([\\d\\.]*).*"));
    static QRegExp re_dirchange(QStringLiteral("cvs status: Examining\\s+(.*)"));
#else
    static QRegularExpression re_start(QStringLiteral("^=+$"));
    static QRegularExpression re_file(QStringLiteral("File:\\s+(.*)\\s+Status:\\s+(.*)"));
    static QRegularExpression re_workrev(QStringLiteral("\\s+Working revision:\\s+([\\d\\.]*).*"));
    static QRegularExpression re_reporev(QStringLiteral("\\s+Repository revision:\\s+([\\d\\.]*).*"));
    static QRegularExpression re_dirchange(QStringLiteral("cvs status: Examining\\s+(.*)"));
#endif

    QString currentDir;

    QStringList lines = jobOutput.split(QLatin1Char('\n'));
    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];

        if (s.isEmpty())
            continue;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto match_start = re_start.match(s);
        auto match_file = re_file.match(s);
        auto match_work = re_workrev.match(s);
        auto match_repo = re_reporev.match(s);
        auto match_dir = re_dirchange.match(s);

        if ( match_start.hasMatch() ) {
#else
        if ( re_start.exactMatch(s) ) {
#endif
            if ( !filename.isEmpty() ) {
//                qCDebug(PLUGIN_CVS) << "File:" << filename << "Status:" << status
//                    << "working:" << workrev << "repo:" << reporev << endl;

                addInfoToList( infos, currentDir, filename, status );
            }
            filename.clear();
            status.clear();
            reporev.clear();
            workrev.clear();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        } else if ( re_file.exactMatch(s) ) {
            filename = re_file.cap(1).trimmed();
            status = re_file.cap(2).trimmed();
        } else if ( re_workrev.exactMatch(s) ) {
            workrev = re_workrev.cap(1);
        } else if ( re_reporev.exactMatch(s) ) {
            reporev = re_reporev.cap(1);
        } else if ( re_dirchange.exactMatch(s) ) {
            currentDir = re_dirchange.cap(1);
#else
        } else if ( match_file.hasMatch() ) {
            filename = match_file.captured(1).trimmed();
            status = match_file.captured(2).trimmed();
        } else if ( match_work.hasMatch() ) {
            workrev = match_work.captured(1);
        } else if ( match_repo.hasMatch() ) {
            reporev = match_repo.captured(1);
        } else if ( match_dir.hasMatch() ) {
            currentDir = match_dir.captured(1);
#endif
            if (currentDir == QLatin1String("."))
                currentDir.clear();
        }
    }

    if ( !filename.isEmpty() ) {
//        qCDebug(PLUGIN_CVS) << "File:" << filename << "Status:" << status
//            << "working:" << workrev << "repo:" << reporev << endl;

        addInfoToList( infos, currentDir, filename, status );
    }

}

KDevelop::VcsStatusInfo::State CvsStatusJob::String2EnumState(const QString& stateAsString)
{
    KDevelop::VcsStatusInfo::State state;

    if (stateAsString == QLatin1String("Up-to-date"))
        return KDevelop::VcsStatusInfo::ItemUpToDate;
    else if (stateAsString == QLatin1String("Locally Modified"))
        return KDevelop::VcsStatusInfo::ItemModified;
    else if (stateAsString == QLatin1String("Locally Added"))
        return KDevelop::VcsStatusInfo::ItemAdded;
    else if (stateAsString == QLatin1String("Locally Removed"))
        return KDevelop::VcsStatusInfo::ItemDeleted;
    else if (stateAsString == QLatin1String("Unresolved Conflict"))
        return KDevelop::VcsStatusInfo::ItemHasConflicts;
    else if (stateAsString == QLatin1String("Needs Patch"))
        return KDevelop::VcsStatusInfo::ItemUpToDate;
    else
        return KDevelop::VcsStatusInfo::ItemUnknown;

    return state;
}

