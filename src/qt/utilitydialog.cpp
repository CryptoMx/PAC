// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "utilitydialog.h"

#include "ui_helpmessagedialog.h"

#include "bitcoingui.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "intro.h"
#include "paymentrequestplus.h"
#include "guiutil.h"

#include "clientversion.h"
#include "init.h"
#include "util.h"

#include <stdio.h>

#include <QCloseEvent>
#include <QLabel>
#include <QRegExp>
#include <QTextTable>
#include <QTextCursor>
#include <QVBoxLayout>

/** "Help message" or "About" dialog box */
HelpMessageDialog::HelpMessageDialog(QWidget *parent, HelpMode helpMode) :
    QDialog(parent),
    ui(new Ui::HelpMessageDialog)
{
    ui->setupUi(this);

    QString version = tr("$PAC Core") + " " + tr("version") + " " + QString::fromStdString(FormatFullVersion());
    /* On x86 add a bit specifier to the version so that users can distinguish between
     * 32 and 64 bit builds. On other architectures, 32/64 bit may be more ambigious.
     */
#if defined(__x86_64__)
    version += " " + tr("(%1-bit)").arg(64);
#elif defined(__i386__ )
    version += " " + tr("(%1-bit)").arg(32);
#endif

    if (helpMode == about)
    {
        setWindowTitle(tr("About $PAC Core"));

        /// HTML-format the license message from the core
        QString licenseInfo = QString::fromStdString(LicenseInfo());
        QString licenseInfoHTML = licenseInfo;

        // Make URLs clickable
        QRegExp uri("<(.*)>", Qt::CaseSensitive, QRegExp::RegExp2);
        uri.setMinimal(true); // use non-greedy matching
        licenseInfoHTML.replace(uri, "<a href=\"\\1\">\\1</a>");
        // Replace newlines with HTML breaks
        licenseInfoHTML.replace("\n\n", "<br><br>");

        ui->aboutMessage->setTextFormat(Qt::RichText);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        text = version + "\n" + licenseInfo;
        ui->aboutMessage->setText(version + "<br><br>" + licenseInfoHTML);
        ui->aboutMessage->setWordWrap(true);
        ui->helpMessage->setVisible(false);
    } else if (helpMode == cmdline) {
        setWindowTitle(tr("Command-line options"));
        QString header = tr("Usage:") + "\n" +
            "  paccoin-qt [" + tr("command-line options") + "]                     " + "\n";
        QTextCursor cursor(ui->helpMessage->document());
        cursor.insertText(version);
        cursor.insertBlock();
        cursor.insertText(header);
        cursor.insertBlock();

        std::string strUsage = HelpMessage(HMM_BITCOIN_QT);
        const bool showDebug = GetBoolArg("-help-debug", false);
        strUsage += HelpMessageGroup(tr("UI Options:").toStdString());
        if (showDebug) {
            strUsage += HelpMessageOpt("-allowselfsignedrootcertificates", strprintf("Allow self signed root certificates (default: %u)", DEFAULT_SELFSIGNED_ROOTCERTS));
        }
        strUsage += HelpMessageOpt("-choosedatadir", strprintf(tr("Choose data directory on startup (default: %u)").toStdString(), DEFAULT_CHOOSE_DATADIR));
        strUsage += HelpMessageOpt("-lang=<lang>", tr("Set language, for example \"de_DE\" (default: system locale)").toStdString());
        strUsage += HelpMessageOpt("-min", tr("Start minimized").toStdString());
        strUsage += HelpMessageOpt("-rootcertificates=<file>", tr("Set SSL root certificates for payment request (default: -system-)").toStdString());
        strUsage += HelpMessageOpt("-splash", strprintf(tr("Show splash screen on startup (default: %u)").toStdString(), DEFAULT_SPLASHSCREEN));
        strUsage += HelpMessageOpt("-resetguisettings", tr("Reset all settings changed in the GUI").toStdString());
        if (showDebug) {
            strUsage += HelpMessageOpt("-uiplatform", strprintf("Select platform to customize UI for (one of windows, macosx, other; default: %s)", BitcoinGUI::DEFAULT_UIPLATFORM));
        }
        QString coreOptions = QString::fromStdString(strUsage);
        text = version + "\n" + header + "\n" + coreOptions;

        QTextTableFormat tf;
        tf.setBorderStyle(QTextFrameFormat::BorderStyle_None);
        tf.setCellPadding(2);
        QVector<QTextLength> widths;
        widths << QTextLength(QTextLength::PercentageLength, 35);
        widths << QTextLength(QTextLength::PercentageLength, 65);
        tf.setColumnWidthConstraints(widths);

        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);

        Q_FOREACH (const QString &line, coreOptions.split("\n")) {
            if (line.startsWith("  -"))
            {
                cursor.currentTable()->appendRows(1);
                cursor.movePosition(QTextCursor::PreviousCell);
                cursor.movePosition(QTextCursor::NextRow);
                cursor.insertText(line.trimmed());
                cursor.movePosition(QTextCursor::NextCell);
            } else if (line.startsWith("   ")) {
                cursor.insertText(line.trimmed()+' ');
            } else if (line.size() > 0) {
                //Title of a group
                if (cursor.currentTable())
                    cursor.currentTable()->appendRows(1);
                cursor.movePosition(QTextCursor::Down);
                cursor.insertText(line.trimmed(), bold);
                cursor.insertTable(1, 2, tf);
            }
        }

        ui->helpMessage->moveCursor(QTextCursor::Start);
        ui->scrollArea->setVisible(false);
        ui->aboutLogo->setVisible(false);
    } else if (helpMode == pshelp) {
        setWindowTitle(tr("PrivatePAC information"));

        ui->aboutMessage->setTextFormat(Qt::RichText);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->aboutMessage->setText(tr("\
<h3>PrivatePAC Basics</h3> \
PrivatePAC gives you true financial privacy by obscuring the origins of your funds. \
All the $PAC in your wallet is comprised of different \"inputs\" which you can think of as separate, discrete coins.<br> \
PrivatePAC uses an innovative process to mix your inputs with the inputs of two other people, without having your coins ever leave your wallet. \
You retain control of your money at all times..<hr> \
<b>The PrivatePAC process works like this:</b>\
<ol type=\"1\"> \
<li>PrivatePAC begins by breaking your transaction inputs down into standard denominations. \
These denominations are 10 $PAC, 100 $PAC, 1000 $PAC and $10000 PAC -- sort of like the paper money you use every day.</li> \
<li>Your wallet then sends requests to specially configured software nodes on the network, called \"masternodes.\" \
These masternodes are informed then that you are interested in mixing a certain denomination. \
No identifiable information is sent to the masternodes, so they never know \"who\" you are.</li> \
<li>When two other people send similar messages, indicating that they wish to mix the same denomination, a mixing session begins. \
The masternode mixes up the inputs and instructs all three users' wallets to pay the now-transformed input back to themselves. \
Your wallet pays that denomination directly to itself, but in a different address (called a change address).</li> \
<li>In order to fully obscure your funds, your wallet must repeat this process a number of times with each denomination. \
Each time the process is completed, it's called a \"round.\" Each round of PrivatePAC makes it exponentially more difficult to determine where your funds originated.</li> \
<li>This mixing process happens in the background without any intervention on your part. When you wish to make a transaction, \
your funds will already be anonymized. No additional waiting is required.</li> \
</ol> <hr>\
<b>IMPORTANT:</b> Your wallet only contains 1000 of these \"change addresses.\" Every time a mixing event happens, up to 9 of your addresses are used up. \
This means those 1000 addresses last for about 100 mixing events. When 900 of them are used, your wallet must create more addresses. \
It can only do this, however, if you have automatic backups enabled.<br> \
Consequently, users who have backups disabled will also have PrivatePAC disabled. <hr>\
        "));
//For more info see <a href=\"https://paccoinpay.atlassian.net/wiki/display/DOC/PrivateSend\">https://paccoinpay.atlassian.net/wiki/display/DOC/PrivateSend</a> \
  //      "));
        ui->aboutMessage->setWordWrap(true);
        ui->helpMessage->setVisible(false);
        ui->aboutLogo->setVisible(false);
    } else if (helpMode == ovhelp) {
        setWindowTitle(tr("OverviewPAC information"));

        ui->aboutMessage->setTextFormat(Qt::RichText);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->aboutMessage->setText(tr("\
<h3>PAC Overview</h3> \
PAC is a digital currency that enables anonymous, instant payments to anyone, anywhere in the world. \
PAC uses peer-to-peer technology to operate with no central authority: managing transactions and issuing money are carried out collectively by the network. \
PAC is the name of the open source software which enables the use of this currency.<hr><br> \
On the left side of this window you can watch your balance, the available balance is the balance you can spend and do use of while the pending balance \
first need to be confirmed by the network, this means that it is yours but first to avoid security issues it is need to be confirmed, and at the bottom is the total \
(the sum of the available and pending). <br><br> \
On the right side you can see all of your transactions, the coins you send, receive or get rewarded, if you click on one of them it will redirect you to \
see all of the information of the transaction. <br> \
For more information see https://wallets.paccoin.net/.<hr><br> \
IMPORTANT: the convertion PAC to USD can have some fluctuation since the value is obtained from an external API and the value is obtained when opening the wallet."));
        
        ui->aboutMessage->setWordWrap(true);
        ui->helpMessage->setVisible(false);
        ui->aboutLogo->setVisible(false);
    }
    // Theme dependent Gfx in About popup
    QString helpMessageGfx = ":/images/" + GUIUtil::getThemeName() + "/about";
    QPixmap pixmap = QPixmap(helpMessageGfx);
    ui->aboutLogo->setPixmap(pixmap);
}

HelpMessageDialog::~HelpMessageDialog()
{
    delete ui;
}

void HelpMessageDialog::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    fprintf(stdout, "%s\n", qPrintable(text));
}

void HelpMessageDialog::showOrPrint()
{
#if defined(WIN32)
    // On Windows, show a message box, as there is no stderr/stdout in windowed applications
    exec();
#else
    // On other operating systems, print help text to console
    printToConsole();
#endif
}

void HelpMessageDialog::on_okButton_accepted()
{
    close();
}


/** "Shutdown" window */
ShutdownWindow::ShutdownWindow(QWidget *parent, Qt::WindowFlags f):
    QWidget(parent, f)
{
    setStyleSheet("background: rgb(50,50,50); color:white;");
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(
        tr("$PAC Core is shutting down...") + "<br /><br />" +
        tr("Do not shut down the computer until this window disappears.")));
    setLayout(layout);
}

QWidget *ShutdownWindow::showShutdownWindow(BitcoinGUI *window)
{
    if (!window)
        return nullptr;

    // Show a simple window indicating shutdown status
    QWidget *shutdownWindow = new ShutdownWindow();
    shutdownWindow->setWindowTitle(window->windowTitle());

    // Center shutdown window at where main window was
    const QPoint global = window->mapToGlobal(window->rect().center());
    shutdownWindow->move(global.x() - shutdownWindow->width() / 2, global.y() - shutdownWindow->height() / 2);
    shutdownWindow->show();
    return shutdownWindow;
}

void ShutdownWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
}
