/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#include "Mp3tunesService.h"

#include "Mp3tunesConfig.h"

#include "Amarok.h"
#include "Debug.h"
#include "ContextStatusBar.h"


#include <KMessageBox>
#include <kpassworddialog.h>

#include <QDomDocument>

AMAROK_EXPORT_PLUGIN( Mp3tunesServiceFactory )

void Mp3tunesServiceFactory::init()
{

    Mp3tunesConfig config;
    
    ServiceBase* service = new Mp3tunesService( "MP3tunes.com", config.email(), config.password() );
    m_activeServices << service;
    emit newService( service );
}


QString Mp3tunesServiceFactory::name()
{
    return "MP3tunes.com";
}


KPluginInfo Mp3tunesServiceFactory::info()
{
    KPluginInfo pluginInfo(  "amarok_service_mp3tunes.desktop", "services" );
    pluginInfo.setConfig( config() );
    return pluginInfo;
}


KConfigGroup Mp3tunesServiceFactory::config()
{
    return Amarok::config( "Service_Mp3tunes" );
}





Mp3tunesService::Mp3tunesService(const QString & name, const QString &email, const QString &password )
 : ServiceBase( name )
 , m_email( email )
 , m_password( password )
 , m_partnerToken( "7359149936" )
 , m_apiOutputFormat( "xml")
 , m_authenticated( false )
 , m_sessionId ( QString() )
{

    setShortDescription( i18n( "The MP3Tunes Locker service. Access your stored music!" ) );
    setIcon( KIcon( "view-services-mp3tunes-amarok" ) );

}


Mp3tunesService::~Mp3tunesService()
{
}

void Mp3tunesService::polish()
{
    m_bottomPanel->hide();
    
    if ( !m_authenticated )
        authenticate( m_email, m_password );

}

void Mp3tunesService::authenticate( const QString & uname, const QString & passwd )
{

QString username, password;
   
    if ( uname.isEmpty() || passwd.isEmpty() ) {
        KPasswordDialog dlg( 0 , KPasswordDialog::ShowUsernameLine );  //FIXME 0x02 = KPasswordDialog::showUsername according to api, but that does not work
        dlg.setPrompt( i18n( "Enter a login and a password" ) );
        if( !dlg.exec() )
            return; //the user canceled

        username = dlg.username();
        password = dlg.password();

    } else {
        username = uname;
        password = passwd;
    }

    QString authenticationString = "https://shop.mp3tunes.com/api/v0/login?username=<username>&password=<password>&partner_token=<partner token>&output=<output format>";

    authenticationString.replace(QString("<username>"), username);
    authenticationString.replace(QString("<password>"), password);
    authenticationString.replace(QString("<partner token>"), m_partnerToken);
    authenticationString.replace(QString("<output format>"), m_apiOutputFormat);

    debug() << "Authenticating with string: " << authenticationString;



    m_xmlDownloadJob = KIO::storedGet( authenticationString, KIO::NoReload, KIO::HideProgressInfo );
    connect( m_xmlDownloadJob, SIGNAL(result(KJob *)), this, SLOT( authenticationComplete( KJob*) ) );
    Amarok::ContextStatusBar::instance() ->newProgressOperation( m_xmlDownloadJob )
    .setDescription( i18n( "Authenticating" ) );

}

void Mp3tunesService::authenticationComplete(KJob * job)
{

    if ( !job->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( job != m_xmlDownloadJob )
        return ; //not the right job, so let's ignore it


    QString xmlReply = ((KIO::StoredTransferJob* )job)->data();
    debug() << "Authentication reply: " << xmlReply;


    //so lets figure out what we got here:
    QDomDocument doc( "reply" );

    doc.setContent( m_xmlDownloadJob->data() );

   
    QDomElement root = doc.firstChildElement("mp3tunes");

    //find status code:
    QDomElement element = root.firstChildElement("status");

    if ( element.text() == "1" ) {

        element = root.firstChildElement("session_id");
        m_sessionId = element.text();
        m_authenticated = true;


        m_collection = new Mp3tunesServiceCollection( m_sessionId );
        QList<int> levels;
        levels << CategoryId::Artist << CategoryId::Album;
        setModel( new SingleCollectionTreeItemModel( m_collection, levels ) );

    } else {

        element = root.firstChildElement("errorMessage");
        KMessageBox::error ( this, element.text(), i18n( "Authentication Error!" ) );	

    }



    m_xmlDownloadJob->deleteLater();
}



#include "Mp3tunesService.moc"




