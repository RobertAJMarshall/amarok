/* 
   Copyright (C) 2008 Alejandro Wainzinger <aikawarazuni@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef IPODCOLLECTION_H
#define IPODCOLLECTION_H

extern "C" {
  #include <gpod/itdb.h>
}


#include "MediaDeviceCollection.h"

#include "Debug.h"

#include "IpodHandler.h"

#include <QtGlobal>
#include <QMap>

#include <KIcon>

class IpodCollection;
class MediaDeviceInfo;

class IpodCollectionFactory : public MediaDeviceCollectionFactory<IpodCollection>
{
    Q_OBJECT
    public:
        IpodCollectionFactory();
        virtual ~IpodCollectionFactory();

    protected:
        //virtual Amarok::Collection* createCollection( MediaDeviceInfo* info );
};

class IpodCollection : public Amarok::Collection, public MemoryCollection
{
    Q_OBJECT
    public:
        IpodCollection( const QString &mountPoint, const QString &udi );
        IpodCollection( MediaDeviceInfo* info );
        virtual ~IpodCollection();

        virtual bool possiblyContainsTrack( const KUrl &url ) const;
        virtual Meta::TrackPtr trackForUrl( const KUrl &url );

        void copyTrackListToDevice( const Meta::TrackList tracklist );
        void removeTrack( const Meta::IpodTrackPtr &track );

        void setTrackToDelete( const Meta::IpodTrackPtr &track );

        virtual void deviceRemoved();

        virtual void startFullScan();
        virtual QueryMaker* queryMaker();

        QString udi() const;

        virtual CollectionLocation* location() const;

        virtual QString collectionId() const;
        virtual QString prettyName() const;
        virtual KIcon icon() const { return KIcon("multimedia-player-apple-ipod"); };

        virtual bool hasCapabilityInterface( Meta::Capability::Type type ) const;
        virtual Meta::Capability* createCapabilityInterface( Meta::Capability::Type type );

        virtual void collectionUpdated() { DEBUG_BLOCK emit updated(); }

        Ipod::IpodHandler* handler() { return m_handler; }

        void updateTags( Meta::IpodTrack *track);
        void writeDatabase();

    signals:
        void collectionReady();
        void collectionDisconnected( const QString &udi );

        void copyTracksCompleted( bool success );

    public slots:
        void connectDevice();
        void disconnectDevice();
        void deleteTracksSlot( Meta::TrackList tracklist );

        void slotDisconnect();

    private slots:
        void slotCopyTracksCompleted( bool success );
        void slotDeleteTracksCompleted();

    private:
        Meta::IpodTrackPtr m_trackToDelete;
        QString            m_mountPoint;
        QString            m_udi;
        Ipod::IpodHandler *m_handler;
};

#endif
