#include "ktelephonecall.hpp"
#include "ktelephonetransfercall.hpp"

#include "ui_call.h"
#include <QDebug>
#include <QRegularExpression>

bool isDTMFDigit(QString digit) {
    QRegularExpression re("[0-9a-dA-D*#]");
    QRegularExpressionMatch match = re.match(digit);
    return match.hasMatch();
}

KTelephoneCall::KTelephoneCall(KTelephone *parent, QString direction, QString username) :
        QDialog(),
        ui(new Ui::call) {
    ui->setupUi(this);

    this->callDirection = direction;
    this->calleeUsername = username;

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_A), this,
                  SLOT(actionAnswer()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this,
                  SLOT(actionHangup()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_H), this,
                  SLOT(actionHold()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this,
                  SLOT(actionMute()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this,
                  SLOT(openTransferCallDialog()));

    connect(ui->answerButton,
            SIGNAL(clicked()), this,
            SLOT(actionAnswer()));
    connect(ui->cancelButton,
            SIGNAL(clicked()), this,
            SLOT(actionHangup()));

    connect(ui->muteButton,
            SIGNAL(clicked()), this,
            SLOT(actionMute()));
    connect(ui->holdButton,
            SIGNAL(clicked()), this,
            SLOT(actionHold()));
    connect(ui->transferButton,
            SIGNAL(clicked()), this,
            SLOT(openTransferCallDialog()));

    connect(this,
            SIGNAL(rejected()), this,
            SLOT(onWindowClose()));

    ui->dtmfInput->hide();
    ui->dtmfInput->hide();
    ui->callAction->hide();
    ui->dtmfInput->setReadOnly(true);
    ui->dtmfInput->setEnabled(false);
    ui->dtmfInput->setFrame(false);
    ui->dtmfInput->setStyleSheet("font-size: 10pt;background: transparent");

    if (callDirection == "outbound") {
        ui->answerButton->deleteLater();
    }
}

KTelephoneCall::~KTelephoneCall() {
    delete ui;
}

void KTelephoneCall::setInstance(MyCall *telephoneCall) {
    this->mCall = telephoneCall;
    CallInfo ci = this->mCall->getInfo();
    QString whoLabel;
    if (this->callDirection == "outbound") {
        whoLabel.append("To: ");
        whoLabel.append(QString::fromStdString(ci.remoteUri));
        QSound *outboundAudio = new QSound("data/sounds/outbound-ring.wav");
        outboundAudio->setLoops(QSound::Infinite);
        outboundAudio->play();
        this->outboundAudio = outboundAudio;
        this->contact = QString::fromStdString(ci.remoteUri);
    } else {
        // Ringing
        whoLabel.append("From: ");
        whoLabel.append(QString::fromStdString(ci.localContact));
        QSound *inboundAudio = new QSound("data/sounds/inbound-ring.wav");
        inboundAudio->setLoops(QSound::Infinite);
        inboundAudio->play();
        this->inboundAudio = inboundAudio;
        this->contact = QString::fromStdString(ci.localContact);
    }
    ui->whoLabel->setText(whoLabel);
}

void KTelephoneCall::callDestroy() {
    if (this->mCall) {
        delete this->mCall;
        this->mCall = NULL;
    }
    this->close();
}

void KTelephoneCall::closeEvent(QCloseEvent* event) {
    this->onWindowClose();
}

void KTelephoneCall::keyPressEvent(QKeyEvent *event) {
    if (isDTMFDigit(event->text())) {
        this->actionDtmf(event->text());
    }
}

void KTelephoneCall::onWindowClose() {
    if (this->outboundAudio) {
        this->outboundAudio->stop();
    }
    if (this->inboundAudio) {
        this->inboundAudio->stop();
    }
    if (this->mCall) {
        this->mCall->doHangup();
        delete this->mCall;
    }
    if (this->transferCall) {
        this->transferCall->close();
    }
}

void KTelephoneCall::callbackAnswer() {
    ui->dtmfInput->show();
    ui->callAction->show();
    this->answered = true;
    if (this->outboundAudio) {
        this->outboundAudio->stop();
    }
}

void KTelephoneCall::actionAnswer() {
    ui->answerButton->setEnabled(false);
    this->mCall->doAnswer();

    ui->dtmfInput->show();
    ui->callAction->show();
    if (this->inboundAudio) {
        this->inboundAudio->stop();
    }
}

void KTelephoneCall::actionHangup() {
    this->close();
}

void KTelephoneCall::actionHold() {
    if (!this->hold) {
        ui->holdButton->setText("Release");
    } else {
        ui->holdButton->setText("Hold");
    }
    this->mCall->doHold(this->hold);
    this->hold = !this->hold;
}

void KTelephoneCall::actionMute() {
    if (!this->mute) {
        ui->muteButton->setText("Unmute");
    } else {
        ui->muteButton->setText("Mute");
    }
    this->mCall->doMute(this->mute);
    this->mute = !this->mute;
}

void KTelephoneCall::actionDtmf(QString digit) {
    if (this->answered) {
        try {
            this->mCall->doDtmf(digit);
            ui->dtmfInput->insert(digit);
        } catch (Error &err) {
            qDebug() << "DTMF Error " << QString::fromStdString(err.reason);
        }
    }
}

void KTelephoneCall::actionTransfer(QString destinationNumber) {
    if (destinationNumber.isEmpty() || destinationNumber.isNull()) {
        return;
    }
    this->mCall->doTransfer(destinationNumber, this->calleeUsername);
    this->transferCall->close();
}

void KTelephoneCall::openTransferCallDialog() {
    if (!this->transferCall) {
        const auto onClickTransfer = std::bind(&KTelephoneCall::actionTransfer, this, std::placeholders::_1);
        this->transferCall = new KTelphoneTransferCall(onClickTransfer);
        this->transferCall->setWindowTitle(contact);
    }
    this->transferCall->show();
}
