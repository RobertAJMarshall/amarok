/****************************************************************************************
 * Copyright (c) 2010 Bart Cerneels <bart.cerneels@kde.org                              *
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

#include "OpmlDirectoryModel.h"

#include "OpmlParser.h"

#include <ThreadWeaver/Weaver>

OpmlDirectoryModel::OpmlDirectoryModel( KUrl outlineUrl, QObject *parent )
    : QAbstractItemModel( parent )
    , m_rootOpmlUrl( outlineUrl )
{
    //init
    fetchMore( QModelIndex() );
}

OpmlDirectoryModel::~OpmlDirectoryModel()
{
}

QModelIndex
OpmlDirectoryModel::index( int row, int column, const QModelIndex &parent ) const
{
    if( !parent.isValid() )
    {
        if( m_rootOutlines.isEmpty() || m_rootOutlines.count() <= row )
            return QModelIndex();
        else
            return createIndex( row, column, m_rootOutlines[row] );
    }

    OpmlOutline *parentOutline = static_cast<OpmlOutline *>( parent.internalPointer() );
    if( !parentOutline )
        return QModelIndex();

    if( !parentOutline->hasChildren() || parentOutline->children().count() <= row )
        return QModelIndex();

    return createIndex( row, column, parentOutline->children()[row] );
}

QModelIndex
OpmlDirectoryModel::parent( const QModelIndex &idx ) const
{
    if( !idx.isValid() )
        return QModelIndex();
    OpmlOutline *outline = static_cast<OpmlOutline *>( idx.internalPointer() );
    if( outline->isRootItem() )
        return QModelIndex();

    OpmlOutline *parentOutline = outline->parent();
    int childIndex = parentOutline->children().indexOf( outline );
    return createIndex( childIndex, 0, parentOutline );
}

int
OpmlDirectoryModel::rowCount( const QModelIndex &parent ) const
{
    if( !parent.isValid() )
        return m_rootOutlines.count();

    OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );

    if( !outline || !outline->hasChildren() )
        return 0;
    else
        return outline->children().count();
}

bool
OpmlDirectoryModel::hasChildren( const QModelIndex &parent ) const
{
    if( !parent.isValid() )
        return !m_rootOutlines.isEmpty();

    OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );

    return outline && outline->hasChildren();
}

int
OpmlDirectoryModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant
OpmlDirectoryModel::data( const QModelIndex &index, int role ) const
{
    if( !index.isValid() )
        return QVariant();

    OpmlOutline *outline = static_cast<OpmlOutline *>( index.internalPointer() );
    if( !outline )
        return QVariant();

    switch( role )
    {
        case Qt::DisplayRole:
            return outline->attributes()["text"];
        default:
            return QVariant();
    }

    return QVariant();
}

//bool
//OpmlDirectoryModel::canFetchMore( const QModelIndex &parent ) const
//{
//    if( !parent.isValid() && !!m_rootOutlines.isEmpty() )
//        return true;

//    OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );
//    if( !outline )
//        return false;

//    if( outline->attributes().contains("link") &&
//        outline->attributes()["link"].endsWith("opml", Qt::CaseInsensitive ) )
//        return true;

//    return false;
//}

void
OpmlDirectoryModel::fetchMore( const QModelIndex &parent )
{
    KUrl urlToFetch;
    if( !parent.isValid() && !!m_rootOutlines.isEmpty() )
    {
        urlToFetch = m_rootOpmlUrl;
    }
    else
    {
        OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );
        if( !outline )
            return;
        if( !outline->attributes().contains("link") )
            return;
        urlToFetch = KUrl( outline->attributes()["link"] );
    }

    if( !urlToFetch.isValid() )
        return;

    m_currentFetchingIndex = parent;

    OpmlParser *parser = new OpmlParser( urlToFetch );
    connect( parser, SIGNAL( outlineParsed( OpmlOutline * ) ),
             SLOT( slotOpmlOutlineParsed( OpmlOutline * ) ) );
    connect( parser, SIGNAL( doneParsing() ), SLOT( slotOpmlParsingDone() ) );

    ThreadWeaver::Weaver::instance()->enqueue( parser );
}

void
OpmlDirectoryModel::slotOpmlOutlineParsed( OpmlOutline *outline )
{
    if( !m_currentFetchingIndex.isValid() )
        m_rootOutlines << outline;

    beginInsertRows( m_currentFetchingIndex, rowCount( m_currentFetchingIndex ),
                     rowCount( m_currentFetchingIndex ) + 1 );
    endInsertRows();
}

void
OpmlDirectoryModel::slotOpmlParsingDone()
{
    OpmlParser *parser = qobject_cast<OpmlParser *>( QObject::sender() );
    m_currentFetchingIndex = QModelIndex();
    parser->deleteLater();
}
