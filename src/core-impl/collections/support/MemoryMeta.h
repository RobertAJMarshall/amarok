/****************************************************************************************
 * Copyright (c) 2011 Bart Cerneels <bart.cerneels@kde.org>                             *
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

#ifndef MEMORYMETA_H
#define MEMORYMETA_H

#include "amarok_export.h"
#include "MemoryCollection.h"
#include "core/meta/Meta.h"

using namespace Collections;

/** These classes can be used with a MemoryCollection to populate the meta-type maps */
namespace MemoryMeta {

class Track;

class Base
{
    public:
        Base( const QString &name ) : m_name( name ) {}
        virtual ~Base() {}

        // Meta::{Artist,Album,Composer,Genre,Year} methods:
        virtual QString name() const { return m_name; }
        virtual Meta::TrackList tracks();

        // MemoryMeta::Base methods:
        void addTrack( Track *track ) { m_tracks << track; }
        void removeTrack( Track *track ) { m_tracks.removeOne( track ); }

    protected:
        QString m_name;
        /* We cannot easily store KSharedPtr to tracks, because it creates reference
         * counting cycle: MemoryMeta::Track::m_album -> MemoryMeta::Album::tracks() ->
         * MemoryMeta::Track. We therefore store plain pointers and rely on
         * MemoryMeta::Track to notify when it is destroyed. */
        QList<Track *> m_tracks;
};

class Artist : public Meta::Artist, public Base
{
    public:
        Artist( const QString &name ) : Base( name ) {}

        virtual QString name() const { return Base::name(); }
        virtual Meta::TrackList tracks() { return Base::tracks(); }

    protected:
        virtual void notifyObservers() const {}
};

class Album : public Meta::Album, public Base
{
    public:
        Album( const QString &name ) : Base( name ), m_isCompilation( false ) {}

        virtual QString name() const { return Base::name(); }

        /** Meta::Album virtual methods */
        virtual bool isCompilation() const { return m_isCompilation; }
        virtual bool hasAlbumArtist() const { return !m_albumArtist.isNull(); }
        virtual Meta::ArtistPtr albumArtist() const { return m_albumArtist; }
        virtual Meta::TrackList tracks() { return Base::tracks(); }

        virtual bool hasImage( int /* size */ = 0 ) const { return !m_image.isNull(); }
        virtual QImage image( int size = 0 ) const;
        /* We intentionally don't advertise canUpdateImage() - setting image here would not
         * currently do what the user expects */
        virtual void setImage( const QImage &image ) { m_image = image; }

        /* MemoryMeta::Album methods: */
        void setAlbumArtist( Meta::ArtistPtr artist ) { m_albumArtist = artist; }
        void setIsCompilation( bool isCompilation ) { m_isCompilation = isCompilation; }

    protected:
        virtual void notifyObservers() const {}

    private:
        bool m_isCompilation;
        Meta::ArtistPtr m_albumArtist;
        QImage m_image;
};

class Composer : public Meta::Composer, public Base
{
    public:
        Composer( const QString &name ) : Base( name ) {}

        virtual QString name() const { return Base::name(); }
        virtual Meta::TrackList tracks() { return Base::tracks(); }

    protected:
        virtual void notifyObservers() const {}
};

class Genre : public Meta::Genre, public Base
{
    public:
        Genre( const QString &name ) : Base( name ) {}

        virtual QString name() const { return Base::name(); }
        virtual Meta::TrackList tracks() { return Base::tracks(); }

    protected:
        virtual void notifyObservers() const {}
};

class Year : public Meta::Year, public Base
{
    public:
        Year( const QString &name ) : Base( name ) {}

        virtual QString name() const { return Base::name(); }
        virtual Meta::TrackList tracks() { return Base::tracks(); }

    protected:
        virtual void notifyObservers() const {}
};

class Track : public Meta::Track
{
    public:
        Track( const Meta::TrackPtr &originalTrack );
        virtual ~Track();

        /* Meta::Track virtual methods */
        virtual QString name() const { return m_track->name(); }

        virtual KUrl playableUrl() const { return m_track->playableUrl(); }
        virtual QString prettyUrl() const { return m_track->prettyUrl(); }
        virtual QString uidUrl() const { return m_track->uidUrl(); }
        virtual bool isPlayable() const { return m_track->isPlayable(); }

        //these functions return the proxy track values
        virtual Meta::AlbumPtr album() const { return m_album; }
        virtual Meta::ArtistPtr artist() const { return m_artist; }
        virtual Meta::ComposerPtr composer() const { return m_composer; }
        virtual Meta::GenrePtr genre() const { return m_genre; }
        virtual Meta::YearPtr year() const { return m_year; }

        //TODO:implement labels
        virtual Meta::LabelList labels() const { return Meta::LabelList(); }
        virtual qreal bpm() const { return m_track->bpm(); }
        virtual QString comment() const { return m_track->comment(); }
        virtual double score() const { return m_track->score(); }
        virtual void setScore( double newScore ) { m_track->setScore( newScore ); }
        virtual int rating() const { return m_track->rating(); }
        virtual void setRating( int newRating ) { m_track->setRating( newRating ); }
        virtual qint64 length() const { return m_track->length(); }
        virtual int filesize() const { return m_track->filesize(); }
        virtual int sampleRate() const { return m_track->sampleRate(); }
        virtual int bitrate() const { return m_track->bitrate(); }
        virtual QDateTime createDate() const { return m_track->createDate(); }
        virtual QDateTime modifyDate() const { return m_track->modifyDate(); }
        virtual int trackNumber() const { return m_track->trackNumber(); }
        virtual int discNumber() const { return m_track->discNumber(); }
        virtual QDateTime lastPlayed() const { return m_track->lastPlayed(); }
        virtual QDateTime firstPlayed() const { return m_track->firstPlayed(); }
        virtual int playCount() const { return m_track->playCount(); }

        virtual qreal replayGain( Meta::ReplayGainTag mode ) const
                { return m_track->replayGain( mode ); }
        virtual QString type() const { return m_track->type(); }

        virtual void prepareToPlay() { m_track->prepareToPlay(); }
        virtual void finishedPlaying( double fraction ) { m_track->finishedPlaying( fraction ); }

        virtual bool inCollection() const { return m_track->inCollection(); }
        virtual Collections::Collection *collection() const { return m_track->collection(); }

        virtual QString cachedLyrics() const { return m_track->cachedLyrics(); }
        virtual void setCachedLyrics( const QString &lyrics ) { m_track->setCachedLyrics( lyrics ); }

        //TODO: implement labels
        virtual void addLabel( const QString &label ) { Q_UNUSED( label ) }
        virtual void addLabel( const Meta::LabelPtr &label ) { Q_UNUSED( label ) }
        virtual void removeLabel( const Meta::LabelPtr &label ) { Q_UNUSED( label ) }

        /* MemoryMeta::Track methods.
         * All of these methods pass the pointer to KSharedPtr (thus memory-manage it),
         * remove this track from previous {Album,Artist,Composer,Genre,Year} entity (if any)
         * and add this track to newly set entity (if non-null) */
        void setAlbum( Album *album );
        void setArtist( Artist *artist );
        void setComposer( Composer *composer );
        void setGenre( Genre *genre );
        void setYear( Year *year );

    private:
        Meta::TrackPtr m_track;
        Meta::AlbumPtr m_album;
        Meta::ArtistPtr m_artist;
        Meta::ComposerPtr m_composer;
        Meta::GenrePtr m_genre;
        Meta::YearPtr m_year;
};

class AMAROK_EXPORT MapAdder
{
    public:
        MapAdder( MemoryCollection *memoryCollection );
        ~MapAdder();

        /**
         * Adds a track to memoryCollection by proxying it using @see MemoryMeta::Track
         * track artist, album, genre, composer and year are replaced in MemoryMeta::Track
         * by relevant MemoryMeta entities, based on their value.
         *
         * @return pointer to a newly created MemoryMeta::Track
         */
        Meta::TrackPtr addTrack( Meta::TrackPtr track );

    private:
        MemoryCollection *m_mc;
};

}
#endif
