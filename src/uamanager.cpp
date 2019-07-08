#include "uamanager.h"

#include <QDebug>
#include <QString>

UserAgentManager::UserAgentManager()
{
  try {
    Endpoint *ep = new Endpoint;
    ep->libCreate();

    // Initialize endpoint
    ep->libInit( ep_cfg );

    // Create SIP transport. Error handling sample is shown
    tcfg.port = 5060;

    ep->transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

    // Start the library (worker threads etc)
    ep->libStart();

    qDebug() << "*** PJSUA2 STARTED ***";
  } catch (Error &err) {
    qDebug() << "Error starting UserAgentManager";
    return;
  }
}

UserAgentManager::~UserAgentManager()
{
  //  ep->libDestroy();
  //  delete ep;
}

AccountConfig UserAgentManager::getAccountConfig(Telephone_t mTelephone)
{
  // Configure an AccountConfig
  AccountConfig acfg;
  QString sip = "sip:";
  QString sipUri = QString();
  acfg.idUri = sipUri.append(sip).append(mTelephone.domain).toStdString();
  QString domain = QString();
  domain.append(sip);
  if (!mTelephone.domain.split('@').isEmpty()) {
    domain.append(mTelephone.domain.split('@').takeLast());
  }
  acfg.regConfig.registrarUri = domain.toStdString();

  AuthCredInfo cred("digest", "*", mTelephone.username.toStdString(), 0, mTelephone.password.toStdString());
  acfg.sipConfig.authCreds.push_back( cred );
  return acfg;
}

void UserAgentManager::newUserAgent(QString domain, AccountConfig acfg)
{
  UserAgent *acc = new UserAgent;
  acc->create(acfg);
  mAccounts[domain] = acc;
}

void UserAgentManager::removeUserAgent(QString domain)
{
  if (mAccounts.contains(domain)) {
    delete mAccounts.value(domain);
    mAccounts.remove(domain);
  }
}