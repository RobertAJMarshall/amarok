// (c) 2004 Mark Kretschmann <markey@web.de>
// (c) 2004 Christian Muehlhaeuser <chris@chris.de>
// See COPYING file for licensing information.

#ifndef AMAROK_COLLECTIONDB_H
#define AMAROK_COLLECTIONDB_H

#include "engineobserver.h"

#include <qdir.h>            //stack allocated
#include <qobject.h>         //baseclass
#include <qstringlist.h>     //stack allocated

#ifdef USE_MYSQL
#include <qdatetime.h>
namespace mysql
{
#include <mysql/mysql.h>
}
#else
#include "sqlite/sqlite3.h"
#endif

class CollectionEmitter;
class CoverFetcher;
class MetaBundle;
class Scrobbler;
class ThreadWeaver;

class CollectionDB : public QObject
{
    Q_OBJECT

    public:
        CollectionDB();
        ~CollectionDB();

        static CollectionEmitter* emitter() { return s_emitter; }

        //sql helper methods
        QStringList query( const QString& statement, QStringList& names, bool debug = false);
        // no way to add a default argument for a Object& in gcc3, hack around it
        QStringList query( const QString& statement ) { QStringList sl; return query( statement, sl, false ); }
        int sqlInsertID();
        QString escapeString( QString string );

        //table management methods
        bool isValid();
        bool isEmpty();

        void createTables( const bool temporary = false );
        void dropTables( const bool temporary = false );
        void createStatsTable();
        void dropStatsTable();
        void moveTempTables();

        //general management methods
        void scan( const QStringList& folders, bool recursively, bool importPlaylists );
        void scanModifiedDirs( bool recursively, bool importPlaylists );

        bool isDirInCollection( QString path );
        bool isFileInCollection( const QString &url  );
        void removeDirFromCollection( QString path );
        void removeSongsInDir( QString path );
        void updateDirStats( QString path, const long datetime );
        void purgeDirCache();

        //song methods
        bool addSong( MetaBundle* bundle, const bool temporary = false );
        bool getMetaBundleForUrl( const QString& url , MetaBundle* bundle );
        void addAudioproperties( const MetaBundle& bundle );

        void updateTags( const QString &url, const MetaBundle &bundle, const bool updateView = true );
        void updateURL( const QString &url, const bool updateView = true );

        //statistics methods
        int addSongPercentage( const QString &url , const int percentage );
        int getSongPercentage( const QString &url  );
        void setSongPercentage( const QString &url , int percentage );

        //artist methods
        QStringList relatedArtists( const QString &artist, uint count );

        //album methods
        void checkCompilations( const QString &path );
        QString albumSongCount( const QString &artist_id, const QString &album_id );

        //list methods
        QStringList artistList( bool withUnknowns = true, bool withCompilations = true );
        QStringList albumList( bool withUnknowns = true, bool withCompilations = true );
        QStringList genreList( bool withUnknowns = true, bool withCompilations = true );
        QStringList yearList( bool withUnknowns = true, bool withCompilations = true );

        QStringList albumListOfArtist( const QString &artist, bool withUnknown = true, bool withCompilations = true );
        QStringList artistAlbumList( bool withUnknown = true, bool withCompilations = true );

        //cover management methods
        /** Saves images located on the user's filesystem */
        bool setAlbumImage( const QString& artist, const QString& album, const KURL& url );
        /** Saves images obtained from CoverFetcher */
        bool setAlbumImage( const QString& artist, const QString& album, QImage img, const QString& amazonUrl = QString::null );

        QString albumImage( const uint artist_id, const uint album_id, const uint width = 1 );
        QString albumImage( const QString &artist, const QString &album, const uint width = 1 );

        bool removeAlbumImage( const uint artist_id, const uint album_id );
        bool removeAlbumImage( const QString &artist, const QString &album );

        //local cover methods
        void addImageToAlbum( const QString& image, QValueList< QPair<QString, QString> > info, bool temporary );
        QString getImageForAlbum( const QString& artist, const QString& album, uint width = 0 );
        QString notAvailCover( int width = 0 );

        uint artistID( QString value, bool autocreate = true, bool useTempTables = false );
        QString artistValue( uint id );
        uint albumID( QString value, bool autocreate = true, bool useTempTables = false );
        QString albumValue( uint id );
        uint genreID( QString value, bool autocreate = true, bool useTempTables = false );
        QString genreValue( uint id );
        uint yearID( QString value, bool autocreate = true, bool useTempTables = false );
        QString yearValue( uint id );

        //member variables
        QString m_amazonLicense;

        QStringList m_values;
        QStringList m_names;

    protected:
        QCString md5sum( const QString& artist, const QString& album, const QString& file = QString::null );

    public slots:
        void fetchCover( QWidget* parent, const QString& artist, const QString& album, bool noedit );
        void stopScan();

    private slots:
        void dirDirty( const QString& path );
        void coverFetcherResult( CoverFetcher* );
        void relatedArtistsFetched( const QString& artist, const QStringList& suggestions );

    private:
        void customEvent( QCustomEvent* );

        uint IDFromValue( QString name, QString value, bool autocreate = true, bool useTempTables = false );
        QString valueFromID( QString table, uint id );

        static CollectionEmitter* s_emitter;

#ifdef USE_MYSQL
        mysql::MYSQL* m_db;
#else
        sqlite3* m_db;
#endif

        ThreadWeaver* m_weaver;
        bool m_monitor;
        QDir m_cacheDir;
        QDir m_coverDir;

        QString m_cacheArtist;
        uint m_cacheArtistID;
        QString m_cacheAlbum;
        uint m_cacheAlbumID;
};


class CollectionEmitter : public QObject, public EngineObserver
{
    Q_OBJECT

    friend class CollectionDB;

    public:
        CollectionEmitter();

    protected:
        void engineTrackEnded( int finalPosition, int trackLength );

    signals:
        void scanStarted();
        void scanDone( bool changed );

        void scoreChanged( const QString &url, int score );

        void coverFetched( const QString &artist, const QString &album );
        void coverFetcherError( const QString &error );
};


class QueryBuilder : public QObject
{
    Q_OBJECT

    public:
        //attributes:
        enum qBuilderTables  { tabAlbum = 1, tabArtist = 2, tabGenre = 4, tabYear = 8, tabSong = 32, tabStats = 64 };
        enum qBuilderOptions { optNoCompilations = 1, optOnlyCompilations = 2, optRemoveDuplicates = 4, optRandomize = 8 };
        enum qBuilderValues  { valID = 1, valName = 2, valURL = 4, valTitle = 8, valTrack = 16, valScore = 32 };

        QueryBuilder();

        void addReturnValue( int table, int value );
        uint countReturnValues();

        void addFilter( int tables, const QString& filter );
        void excludeFilter( int tables, const QString& filter );

        void addMatch( int tables, const QString& match );
        void addMatches( int tables, const QStringList& match );
        void excludeMatch( int tables, const QString& match );

        void setOptions( int options );
        void sortBy( int table, int value, bool descending = false );
        void setLimit( int startPos, int length );

        QStringList run();
        void clear();

    private:
        void linkTables( int tables );

        QString m_values;
        QString m_tables;
        QString m_where;
        QString m_sort;
        QString m_limit;

        int m_linkTables;
        uint m_returnValues;

        CollectionDB m_db;
};


#endif /* AMAROK_COLLECTIONDB_H */
