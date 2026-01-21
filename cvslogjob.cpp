/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvslogjob.h"
#include "debug.h"

#include <QRegExp>
#include <QDateTime>

#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>

CvsLogJob::CvsLogJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : CvsJob(parent, verbosity)
{
}

CvsLogJob::~CvsLogJob()
{
}

QVariant CvsLogJob::fetchResults()
{
    // Convert job's output into KDevelop::VcsEvent
    QList<QVariant> events;
    parseOutput(output(), events);

    return events;
}

void CvsLogJob::parseOutput(const QString& jobOutput, QList<QVariant>& events)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    static QRegExp rx_sep(QStringLiteral("[-=]+"));
    static QRegExp rx_rev(QStringLiteral("revision ((\\d+\\.?)+)"));
    static QRegExp rx_branch(QStringLiteral("branches:\\s+(.*)"));
    static QRegExp rx_date(QStringLiteral("date:\\s+([^;]*);\\s+author:\\s+([^;]*).*"));

#else
    static QRegularExpression rx_sep(QStringLiteral("[-=]+"));
    static QRegularExpression rx_rev(QStringLiteral("revision ((\\d+\\.?)+)"));
    static QRegularExpression rx_branch(QStringLiteral("branches:\\s+(.*)"));
    static QRegularExpression rx_date(QStringLiteral("date:\\s+([^;]*);\\s+author:\\s+([^;]*).*"));
#endif

    QStringList lines = jobOutput.split(QLatin1Char('\n'));

    KDevelop::VcsEvent item;
    bool firstSeperatorReached = false;
    QString log;

    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];
//         qCDebug(PLUGIN_CVS) << "line:" << s ;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto match_rev = rx_rev.match(s);
        auto match_branch = rx_branch.match(s);
        auto match_date = rx_date.match(s);
        auto match_sep = rx_sep.match(s);
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (rx_rev.exactMatch(s)) {
#else
        if (match_rev.hasMatch()) {
#endif
//             qCDebug(PLUGIN_CVS) << "MATCH REVISION" ;
            KDevelop::VcsRevision rev;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            rev.setRevisionValue( rx_rev.cap(1), KDevelop::VcsRevision::FileNumber );
            item.setRevision( rev );
        } else if (rx_branch.exactMatch(s)) {
//             qCDebug(PLUGIN_CVS) << "MATCH BRANCH" ;
        } else if (rx_date.exactMatch(s)) {
//             qCDebug(PLUGIN_CVS) << "MATCH DATE" ;
            QString date = rx_date.cap(1);
#else
            rev.setRevisionValue(match_rev.captured(1), KDevelop::VcsRevision::FileNumber);
            item.setRevision(rev);
        } else if (match_branch.hasMatch()) {
            // Apenas ignoramos como no original
        } else if (match_date.hasMatch()) {
            QString date = match_date.captured(1);
#endif
            // cut out the part that matches the Qt::ISODate format
            date.truncate(19);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            item.setDate( QDateTime::fromString( date, Qt::ISODate ) );
            item.setAuthor( rx_date.cap(2) );
        } else  if (rx_sep.exactMatch(s)) {
//             qCDebug(PLUGIN_CVS) << "MATCH SEPARATOR" ;
#else
            item.setDate(QDateTime::fromString(date, Qt::ISODate));
            item.setAuthor(match_date.captured(2));
        } else if (match_sep.hasMatch()) {
#endif
            if (firstSeperatorReached) {

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                item.setMessage( log );
#else
                item.setMessage(log.trimmed());
#endif
                log.clear();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                events.append( qVariantFromValue( item ) );
#else
                events.append(QVariant::fromValue(item));
#endif

                KDevelop::VcsEvent empty;
                item = empty;
            } else {
                firstSeperatorReached = true;
            }
        } else {
            if (firstSeperatorReached) {
//                 qCDebug(PLUGIN_CVS) << "ADDING LOG" ;
                log += s+QLatin1Char('\n');
            }
        }
    }
}

