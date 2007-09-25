/***************************************************************************
 * copyright            : (C) 2007 Ian Monroe <ian@monroe.nu>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2        *
 *   as published by the Free Software Foundation.                         *
 ***************************************************************************/

#include "debug.h"
#include "meta/MetaUtility.h"
#include "AmarokMimeData.h"
#include "PlaylistGraphicsItem.h"
#include "PlaylistGraphicsView.h"
#include "PlaylistDropVis.h"
#include "PlaylistModel.h"
#include "PlaylistTextItem.h"
#include "TheInstances.h"

#include <QBrush>
#include <QDrag>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QPen>
#include <QRadialGradient>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>

struct Playlist::GraphicsItem::ActiveItems
{
    ActiveItems()
    : foreground( 0 )
    , bottomLeftText( 0 )
    , bottomRightText( 0 )
    , topLeftText( 0 )
    , topRightText( 0 )
    , lastWidth( -5 )
     { }
     ~ActiveItems()
     {
        delete bottomLeftText;
        delete bottomRightText;
         delete foreground;
        delete topLeftText;
        delete topRightText;

      }

     QGraphicsRectItem* foreground;
     Playlist::TextItem* bottomLeftText;
     Playlist::TextItem* bottomRightText;
     Playlist::TextItem* topLeftText;
     Playlist::TextItem* topRightText;

     int lastWidth;

     QRectF preDragLocation;
     Meta::TrackPtr track;
 };


const qreal Playlist::GraphicsItem::ALBUM_WIDTH = 50.0;
const qreal Playlist::GraphicsItem::MARGIN = 2.0;
QFontMetricsF* Playlist::GraphicsItem::s_fm = 0;

Playlist::GraphicsItem::GraphicsItem()
    : QGraphicsItem()
    , m_items( 0 )
    , m_height( -1 )
    , m_groupMode( -1 )
    , m_groupModeChanged ( false )
{
    setZValue( 1.0 );
    if( !s_fm )
    {
        s_fm = new QFontMetricsF( QFont() );
        m_height =  qMax( ALBUM_WIDTH, s_fm->height() * 2 ) + 2 * MARGIN;
    }
    setFlag( QGraphicsItem::ItemIsSelectable );
    setFlag( QGraphicsItem::ItemIsMovable );
    setAcceptDrops( true );
   // setHandlesChildEvents( true ); // don't let drops etc hit the text items, doing stupid things
}

Playlist::GraphicsItem::~GraphicsItem()
{
    delete m_items;
}

void 
Playlist::GraphicsItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
// ::paint RULES:
// 1) You do not talk about ::paint method
// 2) You DO NOT talk about ::paint method
// 3) Do not show or hide item that are already shown or hidden, respectively
// 4) Do not setBrush without making sure its hasn't already been set to that brush().
// 5) If this is your first night at ::paint method, you HAVE to paint.
    Q_UNUSED( painter ); Q_UNUSED( widget );
    const QModelIndex index = The::playlistModel()->index( m_currentRow, 0 );


    if( !m_items || ( option->rect.width() != m_items->lastWidth ) || m_groupModeChanged )
    {

        if( !m_items )
        {
            const Meta::TrackPtr track = index.data( ItemRole ).value< Playlist::Item* >()->track();
            m_items = new Playlist::GraphicsItem::ActiveItems();
            m_items->track = track;
            init( track );
        }
        m_groupModeChanged = false;
        resize( m_items->track, option->rect.width() );
    }

    // paint item background:
    QRectF trackRect;
    if ( m_groupMode == Head ) {

        //make the albu group header stand out
        painter->fillRect( option->rect, QBrush( Qt::darkCyan ) );
        trackRect = QRectF( option->rect.x(), ALBUM_WIDTH + 2 * MARGIN, option->rect.width(), s_fm->height() + MARGIN );

    } else {
        trackRect = option->rect;
    }

    if( option->state & QStyle::State_Selected )
        painter->fillRect( trackRect, option->palette.highlight() );
    else if( m_currentRow % 2 )
        painter->fillRect( trackRect, option->palette.base() );
    else
        painter->fillRect( trackRect, option->palette.alternateBase() );


   
     if ( m_groupMode < Body ) {
          //if we are not grouped, or are the head of a group, paint cover:
         QPixmap albumPixmap;
         if( m_items->track->album() )
            albumPixmap =  m_items->track->album()->image( int( ALBUM_WIDTH ) );
         painter->drawPixmap( MARGIN, MARGIN, albumPixmap );
         if ( m_groupMode == Head ) {
            QRectF markerRect( 0, trackRect.top(), MARGIN * 2, trackRect.height() );
            painter->fillRect( markerRect, QBrush( Qt::darkCyan ) );

         } 
     } else if ( ( m_groupMode == Body ) || ( m_groupMode == End ) ) {
         //pain a bit of color to mark this as part of a group:
         QRectF markerRect( 0, 0, MARGIN * 2, option->rect.height() );
         painter->fillRect( markerRect, QBrush( Qt::darkCyan ) );
    }




    //set overlay if item is active:
    if( index.data( ActiveTrackRole ).toBool() )
    {
        if( !m_items->foreground )
        {
            m_items->foreground = new QGraphicsRectItem( option->rect, this );
            m_items->foreground->setPos( 0.0, MARGIN );
            m_items->foreground->setZValue( 5.0 );
            QRadialGradient gradient(option->rect.width() / 2.0, option->rect.height() / 2.0, option->rect.width() / 2.0, 20 + option->rect.width() / 2.0, option->rect.height() / 2.0 );
            QColor start = option->palette.highlight().color().light();
            start.setAlpha( 51 );
            QColor end = option->palette.highlight().color().dark();
            end.setAlpha( 51 );
            gradient.setColorAt( 0.0, start );
            gradient.setColorAt( 1.0, end );
            QBrush brush( gradient );
            m_items->foreground->setBrush( brush );
            m_items->foreground->setPen( QPen( Qt::NoPen ) );
        }
        if( !m_items->foreground->isVisible() ) 
            m_items->foreground->show();
    }
    else if( m_items->foreground && m_items->foreground->isVisible() )
        m_items->foreground->hide();
}

void
Playlist::GraphicsItem::init( Meta::TrackPtr track )
{

    QFont font;
    font.setPointSize( font.pointSize() - 1 );
    #define NewText( X ) \
        X = new Playlist::TextItem( this ); \
        X->setTextInteractionFlags( Qt::TextEditorInteraction ); \
        X->setFont( font ); 
    NewText( m_items->topLeftText )       
    NewText( m_items->bottomLeftText )
    NewText( m_items->topRightText )
    NewText( m_items->bottomRightText )
    #undef NewText
}

void
Playlist::GraphicsItem::resize( Meta::TrackPtr track, int totalWidth )
{
    if( totalWidth == -1 /*|| totalWidth == m_items->lastWidth */) //no change needed
        return;
    if( m_items->lastWidth != -5 ) //this isn't the first "resize"
        prepareGeometryChange();
    m_items->lastWidth = totalWidth;
    QString prettyLength = Meta::secToPrettyTime( track->length() );
    QString album;
    if( track->album() )
        album = track->album()->name();

    const qreal lineTwoY = m_height / 2 + MARGIN;
    const qreal textWidth = ( ( qreal( totalWidth ) - ALBUM_WIDTH ) / 2.0 );
    const qreal leftAlignX = ALBUM_WIDTH + MARGIN;
    qreal rightAlignX;
    {
        qreal middle = textWidth + ALBUM_WIDTH + ( MARGIN * 2.0 );
        qreal rightWidth = totalWidth - qMax( s_fm->width( album )
            , s_fm->width( prettyLength ) );
        rightAlignX = qMax( middle, rightWidth );
    }


    qreal spaceForLeft = totalWidth - ( totalWidth - rightAlignX ) - leftAlignX;
    m_items->bottomLeftText->setEditableText( QString("%1 - %2").arg( QString::number( track->trackNumber() ), track->name() ) , spaceForLeft );
    m_items->bottomRightText->setEditableText( prettyLength, totalWidth - rightAlignX );

    if ( m_groupMode == None ) {

        m_items->topRightText->setPos( rightAlignX, MARGIN );
        m_items->topRightText->setEditableText( album, totalWidth - rightAlignX );

        {
            QString artist;
            if( track->artist() )
                artist = track->artist()->name();
            m_items->topLeftText->setEditableText( artist, spaceForLeft );
            m_items->topLeftText->setPos( leftAlignX, MARGIN );
        }

        m_items->bottomLeftText->setPos( leftAlignX, lineTwoY );
        m_items->bottomRightText->setPos( rightAlignX, lineTwoY );
    } else if ( m_groupMode == Head ) {

        int headingCenter = MARGIN + ( ALBUM_WIDTH - s_fm->height() ) / 2;

        m_items->topRightText->setPos( rightAlignX, headingCenter );
        m_items->topRightText->setEditableText( album, totalWidth - rightAlignX );

        {
            QString artist;
            if( track->artist() )
                artist = track->artist()->name();
            m_items->topLeftText->setEditableText( artist, spaceForLeft );
            m_items->topLeftText->setPos( leftAlignX, headingCenter );
        }

        int underImageY = MARGIN * 2 + ALBUM_WIDTH;

        m_items->bottomLeftText->setPos( MARGIN * 3, underImageY );
        m_items->bottomRightText->setPos( rightAlignX, underImageY );

    } else { 
        m_items->bottomLeftText->setPos( MARGIN * 3, MARGIN );
        m_items->bottomRightText->setPos( rightAlignX, MARGIN );
    }

    m_items->lastWidth = totalWidth;
}

QRectF
Playlist::GraphicsItem::boundingRect() const
{
    // the viewport()->size() takes scrollbars into account
    return QRectF( 0.0, 0.0, The::playlistView()->viewport()->size().width(), m_height );
}

void
Playlist::GraphicsItem::play()
{
    The::playlistModel()->play( m_currentRow );
}

void 
Playlist::GraphicsItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
    if( m_items )
    {
        event->accept();
        play();
        return;
    }
    QGraphicsItem::mouseDoubleClickEvent( event );
}

void
Playlist::GraphicsItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if( event->buttons() & Qt::RightButton || !m_items )
    {
        event->ignore();
        return;
    }
    m_items->preDragLocation = mapToScene( boundingRect() ).boundingRect();
    QGraphicsItem::mousePressEvent( event );
}

// With help from QGraphicsView::mouseMoveEvent()
void
Playlist::GraphicsItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    if( (event->buttons() & Qt::LeftButton) && ( flags() & QGraphicsItem::ItemIsMovable) && m_items )
    {
        QPointF scenePosition = event->scenePos();

        if( scenePosition.y() < 0 )
            return;

        bool dragOverOriginalPosition = m_items->preDragLocation.contains( scenePosition );

        //make sure item is drawn on top of other items
        setZValue( 2.0 );

        // Determine the list of selected items
        QList<QGraphicsItem *> selectedItems = scene()->selectedItems();
        if( !isSelected() )
            selectedItems << this;
        // Move all selected items
        foreach( QGraphicsItem *item, selectedItems )
        {
            if( (item->flags() & QGraphicsItem::ItemIsMovable) && (!item->parentItem() || !item->parentItem()->isSelected()) )
            {
                Playlist::GraphicsItem *above = 0;
                QPointF diff;
                if( item == this && !dragOverOriginalPosition )
                {
                    diff = event->scenePos() - event->lastScenePos();
                    QList<QGraphicsItem*> collisions = scene()->items( event->scenePos() );
                    foreach( QGraphicsItem *i, collisions )
                    {
                        Playlist::GraphicsItem *c = dynamic_cast<Playlist::GraphicsItem *>( i );
                        if( c && c != this )
                        {
                            above = c;
                            break;
                        }
                    }
                }
                else
                {
                    diff = item->mapToParent( item->mapFromScene(event->scenePos()))
                                              - item->mapToParent(item->mapFromScene(event->lastScenePos()));
                }

                item->moveBy( 0, diff.y() );
                if( item->flags() & ItemIsSelectable )
                    item->setSelected( true );
                
                if( dragOverOriginalPosition )
                    Playlist::DropVis::instance()->show( m_items->preDragLocation.y() );
                else
                    Playlist::DropVis::instance()->show( above );
            }
        }
    }
    else
    {
        QGraphicsItem::mouseMoveEvent( event );
    }
}

void 
Playlist::GraphicsItem::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
    foreach( QString mime, The::playlistModel()->mimeTypes() )
    {
        if( event->mimeData()->hasFormat( mime ) )
        {
            event->accept();
            Playlist::DropVis::instance()->show( this );
            break;
        }
    }
}

void
Playlist::GraphicsItem::dropEvent( QGraphicsSceneDragDropEvent * event )
{
    event->accept();
    setZValue( 1.0 );
    The::playlistModel()->dropMimeData( event->mimeData(), Qt::CopyAction, m_currentRow, 0, QModelIndex() );
    Playlist::DropVis::instance()->hide();
}

void 
Playlist::GraphicsItem::refresh()
{
    QPixmap albumPixmap;
    if( !m_items || !m_items->track )
        return;

    if( m_items->track->album() )
        albumPixmap =  m_items->track->album()->image( int( ALBUM_WIDTH ) );

    //m_items->albumArt->hide();
    //delete ( m_items->albumArt );
    //m_items->albumArt = new QGraphicsPixmapItem( albumPixmap, this );
    //m_items->albumArt->setPos( 0.0, MARGIN );
}

void Playlist::GraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    bool dragOverOriginalPosition = m_items->preDragLocation.contains( event->scenePos() );
    if( dragOverOriginalPosition )
    {
        setPos( m_items->preDragLocation.topLeft() );
        Playlist::DropVis::instance()->hide();
        return;
    }

    Playlist::GraphicsItem *above = 0;
    QList<QGraphicsItem*> collisions = scene()->items( event->scenePos() );
    foreach( QGraphicsItem *i, collisions )
    {
        Playlist::GraphicsItem *c = dynamic_cast<Playlist::GraphicsItem *>( i );
        if( c && c != this )
        {
            above = c;
            break;
        }
    }
    // if we've dropped ourself ontop of another item, then we need to shuffle the tracks below down
    if( above )
    {
        setPos( above->pos() );
        The::playlistView()->moveItem( this, above );
    }

    //make sure item resets its z value
    setZValue( 1.0 );
    Playlist::DropVis::instance()->hide();
}

void Playlist::GraphicsItem::setRow(int row)
{
    m_currentRow = row;

    const QModelIndex index = The::playlistModel()->index( m_currentRow, 0 );

    //figure out our group state and set height accordingly
    int currentGroupState = index.data( GroupRole ).toInt();
    if ( currentGroupState != m_groupMode ) {

        debug() << "Group changed for row " << row;

        prepareGeometryChange();
      
        
        m_groupMode = currentGroupState;
        m_groupModeChanged = true;

        switch ( m_groupMode ) {
        
            case None:
                m_height =  qMax( ALBUM_WIDTH, s_fm->height() * 2 ) + 2 * MARGIN;
                break;
            case Head:
                m_height =  qMax( ALBUM_WIDTH, s_fm->height() * 2 ) + 3 * MARGIN + s_fm->height();
                break;
            case Body:
                m_height =  s_fm->height() + 2 * MARGIN;
                break;
            case End:
                m_height =  s_fm->height() + 2 * MARGIN;
                break;
        }
    }
}
