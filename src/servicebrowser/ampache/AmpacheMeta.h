/***************************************************************************
 *   Copyright (c) 2007  Casey Link <unnamedrambler@gmail.com>             *
 *                 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#ifndef AMPACHEMETA_H
#define AMPACHEMETA_H


#include "../servicemetabase.h"
#include "../ServiceAlbumCoverDownloader.h"

#include <KStandardDirs>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QList>


namespace Meta
{


class AmpacheTrack  : public ServiceTrack
{

    public:
        
    AmpacheTrack( const QString& title )
    : ServiceTrack( title )
    {
    }

    virtual QString sourceName() { return "Ampache"; }
    virtual QString sourceDescription() { return "The Ampache music server project: http://Ampache.org"; }
    virtual QPixmap emblem()  { return  KStandardDirs::locate( "data", "amarok/images/emblem-default.png" );  }
    virtual bool hasSourceInfo() const { return false; }

};


class AmpacheAlbum  : public ServiceAlbumWithCover
{
private:
    QString m_coverURL;


public:
    AmpacheAlbum( const QString &name );
    AmpacheAlbum( const QStringList &resultRow );

    ~AmpacheAlbum();
        
    virtual QString downloadPrefix() const { return "ampache"; }
    
    virtual void setCoverUrl( const QString &coverURL );
    virtual QString coverUrl() const;


};

}

#endif
