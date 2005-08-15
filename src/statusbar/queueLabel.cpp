/***************************************************************************
 *   Copyright (C) 2005 by Gábor Lehel <illissius@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#include "queueLabel.h"
#include "metabundle.h"
#include "playlist.h"
#include "playlistitem.h"

#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kstringhandler.h>
#include <kglobal.h>

static const uint MAX_TO_SHOW = 20;

QueueLabel::QueueLabel( QWidget *parent, const char *name )
    : QLabel( parent, name )
{
    connect( this,                 SIGNAL( queueChanged( const PLItemList &, const PLItemList & ) ),
             Playlist::instance(), SIGNAL( queueChanged( const PLItemList &, const PLItemList & ) ) );
}


void QueueLabel::update() //SLOT
{
    Playlist *pl = Playlist::instance();
    const uint count = pl->m_nextTracks.count();
    setNum( count );
}

void QueueLabel::setNum( int num )
{
    if( num <= 0 )
    {
        clear();
        hide();
    }
    else
    {
        show();

        const QString text = QString::number( num );
        const int th = height() - 4;
        QFont f = font();
        f.setPixelSize( th );
        const int tw = QFontMetrics( f ).width( text );

        const int h = height(), w = kMax( h, tw + h/4 + 2 );
        QPixmap pix( w, h );
        QPainter p( &pix );

        p.setBrush( colorGroup().background() );
        p.setPen( colorGroup().background() );
        p.drawRect( pix.rect() );

        p.setBrush( colorGroup().highlight() );
        p.setPen( colorGroup().highlight().dark() );
        if( w > h )
        {
            p.drawPie( 0, 0, h, h, 90*16, 180*16 );
            p.drawPie( w-1 -h, 0, h, h, -90*16, 180*16 );
            p.drawLine( h/2-1, 0, w-1 - h/2, 0 );
            p.drawLine( h/2-1, h-1, w-1 - h/2, h-1 );
            p.setPen( colorGroup().highlight() );
            p.drawRect( h/2-1, 1, w - h + 1, h-2 );
        }
        else
            p.drawEllipse( pix.rect() );

        p.setFont( f );
        p.setPen( colorGroup().highlightedText() );
        p.setBrush( colorGroup().highlight().dark() );
        p.drawText( pix.rect(), Qt::AlignCenter | Qt::SingleLine, text );

        p.end();
        setPixmap( pix );
    }
}

void QueueLabel::enterEvent( QEvent* )
{
    if( !isHidden() )
        showToolTip();
}

void QueueLabel::leaveEvent( QEvent* )
{
    if( !isHidden() )
        tooltip->close();
}

void QueueLabel::mousePressEvent( QMouseEvent* mouseEvent )
{
    Playlist *pl = Playlist::instance();
    PLItemList &queue = pl->m_nextTracks;
    if( queue.isEmpty() )
        return;

    QPtrList<KPopupMenu> menus;
    menus.setAutoDelete( true );
    KPopupMenu *menu = new KPopupMenu;
    menus.append( menu );

    const uint count = queue.count();
    menu->insertTitle( i18n( "1 Queued Track", "%n Queued Tracks", count ) );
    menu->insertItem( SmallIconSet( "2leftarrow" ),
                      count > 1 ? i18n( "Dequeue All Tracks" ) : i18n( "&Dequeue Track" ), 0 );
    menu->insertSeparator();

    uint i = 1;
    QPtrListIterator<PlaylistItem> it( queue );
    it.toFirst();

    while( i <= count )
    {
        for( uint n = kMin( i + MAX_TO_SHOW - 1, count ); i <= n; ++i, ++it )
            menu->insertItem(
                KStringHandler::rsqueeze( i18n( "%1. %2" ).arg( i ).arg( veryNiceTitle( *it ) ), 50 ), i );

        if( i < count )
        {
            menus.append( new KPopupMenu );
            menu->insertSeparator();
            menu->insertItem( i18n( "%1 More Tracks" ).arg( count - i + 1 ), menus.getLast() );
            menu = menus.getLast();
        }
    }

    int id = menus.getFirst()->exec( mapToGlobal( mouseEvent->pos() ) );
    if( id == 0 ) //dequeue
    {
        const PLItemList dequeued = queue;
        while( !queue.isEmpty() )
            pl->queue( queue.getLast(), true );
        emit queueChanged( PLItemList(), dequeued );
    }
    else
    {
        PlaylistItem *selected = queue.at( id - 1 );
        if( selected )
            pl->ensureItemCentered( selected );
    }
}

void QueueLabel::showToolTip()
{
    Playlist     *pl    = Playlist::instance();
    const uint    count = pl->m_nextTracks.count();
    PlaylistItem *item  = pl->m_nextTracks.getFirst();

    if( !item )
        return;

    QString text = i18n( "1 track queued: %1", "%n tracks queued, next one is: %1", count )
                    .arg( veryNiceTitle( item ) );

    tooltip = new KDE::PopupMessage( parentWidget()->parentWidget(), this, 5000 );
    tooltip->showCloseButton( false );
    tooltip->showCounter( false );
    tooltip->setText( text );

    tooltip->move( this->x(), this->y() + tooltip->height() );
    tooltip->show();
}

QString QueueLabel::veryNiceTitle( PlaylistItem* item ) const
{
    const QString artist = item->exactText( PlaylistItem::Artist ).stripWhiteSpace(),
                  title =  item->exactText( PlaylistItem::Title  ).stripWhiteSpace();
    if( !artist.isEmpty() && !title.isEmpty() )
       return i18n( "%1 by %2" ).arg( title ).arg( artist );
    else
       return MetaBundle::prettyTitle( item->exactText( PlaylistItem::Filename ) );
}


#include "queueLabel.moc"
