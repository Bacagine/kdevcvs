/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsannotatejob.h"
#include "debug.h"

#include <QUrl>
#include <QDir>
#include <QLocale>
#include <QDateTime>

#include <vcs/vcsrevision.h>

CvsAnnotateJob::CvsAnnotateJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : CvsJob(parent, verbosity)
{
}

CvsAnnotateJob::~CvsAnnotateJob()
{
}

QVariant CvsAnnotateJob::fetchResults()
{
    // Convert job's output into KDevelop::VcsAnnotation
    KDevelop::VcsAnnotation annotateInfo;
    parseOutput(output(), getDirectory(), annotateInfo);

    QList<QVariant> lines;
    lines.reserve(annotateInfo.lineCount());
    for(int i=0; i < annotateInfo.lineCount(); i++) {
        KDevelop::VcsAnnotationLine line = annotateInfo.line(i);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        lines.append( qVariantFromValue( line ) );
#else
	lines.append( QVariant::fromValue( line ) );
#endif
    }

    return lines;
}

void CvsAnnotateJob::parseOutput(const QString& jobOutput, const QString& workingDirectory, KDevelop::VcsAnnotation& annotateInfo)
{
    // each annotation line looks like this:
    // 1.1 (kdedev 10-Nov-07): #include <QApplication>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    static QRegExp re(QStringLiteral("([^\\s]+)\\s+\\(([^\\s]+)\\s+([^\\s]+)\\):\\s(.*)"));

    // the file is pomoted like this:
    // Annotations for main.cpp
    static QRegExp reFile(QStringLiteral("Annotations for\\s(.*)"));
#else
    static QRegularExpression re(QStringLiteral("([^\\s]+)\\s+\\(([^\\s]+)\\s+([^\\s]+)\\):\\s(.*)"));
    static QRegularExpression reFile(QStringLiteral("Annotations for\\s(.*)"));
#endif

    QStringList lines = jobOutput.split(QLatin1Char('\n'));

    for (int i=0, linenumber=0; i<lines.count(); ++i) {
        QString s = lines[i];
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto match = re.match(s);
        auto matchFile = reFile.match(s);
#endif


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (re.exactMatch(s)) {
#else
        if (match.hasMatch()) {
#endif
            KDevelop::VcsAnnotationLine item;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            item.setLineNumber( linenumber );
            item.setText( re.cap(4) );
            item.setAuthor( re.cap(2)  );
#else
            item.setLineNumber( linenumber );
            item.setText( match.captured(4) );
            item.setAuthor( match.captured(2)  );
#endif

            KDevelop::VcsRevision rev;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            rev.setRevisionValue( re.cap(1), KDevelop::VcsRevision::FileNumber );
#else
            rev.setRevisionValue(match.captured(1), KDevelop::VcsRevision::FileNumber);
#endif
            item.setRevision( rev );

            // cvs annotate always prints the date with english month names.
            // Using QDate::fromString() directly would fail as it works with
            // localized month names. So we let QLocale do the work .
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QDate date(QLocale::c().toDate(re.cap(3), QStringLiteral("dd-MMM-yy")));
#else
            QDate date(QLocale::c().toDate(match.captured(3), QStringLiteral("dd-MMM-yy")));
#endif
            if (date.year() < 1970)
                date = date.addYears(100);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            item.setDate( QDateTime(date, QTime(), Qt::UTC) );
#else
            item.setDate( QDateTime(date, QTime(0, 0), QTimeZone::UTC) );
#endif

            annotateInfo.insertLine( linenumber, item );
            linenumber++;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        } else if (reFile.exactMatch(s)) {
            QUrl url = QUrl::fromLocalFile(workingDirectory + QDir::separator() + reFile.cap(1));
#else
        } else if ( matchFile.hasMatch() ) {
            QUrl url = QUrl::fromLocalFile(workingDirectory + QDir::separator() + matchFile.captured(1));
#endif
            annotateInfo.setLocation( url );
        } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            qCDebug(PLUGIN_CVS) << "Unmatched:"<<s<<endl;
#else
            qCDebug(PLUGIN_CVS) << "Unmatched:" << s << Qt::endl;
#endif
        }
    }
}

