/****************************************************************************************
 * Copyright (c) 2008 Peter ZHOU <peterzhoulei@gmail.com>                               *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "ScriptImporter.h"

#include "App.h"
#include "AmarokBookmarkScript.h"
#include "AmarokCollectionViewScript.h"
#include "config.h"
#include "core/support/Debug.h"
#include "ScriptingDefines.h"
#include "AmarokPlaylistManagerScript.h"
#include "scripting/scriptmanager/ScriptManager.h"

#include <KIO/Global>

#include <QFile>
#include <QSet>
#include <QUrl>
#include <QTextStream>

using namespace AmarokScript;

ScriptImporter::ScriptImporter( AmarokScriptEngine *scriptEngine, const QUrl &url )
    : QObject( scriptEngine )
    , m_scriptUrl( url )
    , m_scriptEngine( scriptEngine )
{
    QJSValue scriptObject = scriptEngine->newQObject( this );
    scriptEngine->globalObject().setProperty( QStringLiteral("Importer"), scriptObject );
}

void
ScriptImporter::loadExtension( const QString& src )
{
    DEBUG_BLOCK
    /* TODO - Use commented code once QT versions >= 5.12
    m_scriptEngine->importModule( "amarok/" + src );
    */
    QFile extensionFile( "amarok/" + src );
    m_scriptEngine->evaluate( QTextStream( &extensionFile ).readAll() );
}

bool
ScriptImporter::loadQtBinding( const QString& binding )
{
    Q_UNUSED( binding )

    error() << __PRETTY_FUNCTION__ << "Loading Qt bindings in scripts not available in Qt5!";
    return false;
}

bool
ScriptImporter::include( const QString& relativeFilename )
{
    QUrl includeUrl = KIO::upUrl(m_scriptUrl);
    includeUrl = includeUrl.adjusted(QUrl::StripTrailingSlash);
    includeUrl.setPath(includeUrl.path() + QLatin1Char('/') + ( relativeFilename ));
    QFile file( includeUrl.toLocalFile() );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        warning() << "cannot open the include file!";
        return false;
    }
    // TODO - Analyze impact of not loading context
    //m_scriptEngine->currentContext()->setActivationObject(
    //                        m_scriptEngine->currentContext()->parentContext()->activationObject() );
    m_scriptEngine->evaluate( file.readAll(), relativeFilename );
    return true;
}

QStringList
ScriptImporter::availableBindings() const
{
    // TODO - implement listing of imported modules at AmarokScriptEngine
    //return m_scriptEngine->availableExtensions();
    return QStringList();
}

bool
ScriptImporter::loadAmarokBinding( const QString &name )
{
    if( name == QLatin1String("bookmarks") )
        new AmarokBookmarkScript( m_scriptEngine );
    else if( name == QLatin1String("collectionview") )
        new AmarokCollectionViewScript( m_scriptEngine, ScriptManager::instance()->scriptNameForEngine( m_scriptEngine ) );
    else if( name == QLatin1String("playlistmanager") )
        new AmarokPlaylistManagerScript( m_scriptEngine );
    else
    {
        warning() << "\"" << name << "\" doesn't exist!";
        return false;
    }
    return true;
}
