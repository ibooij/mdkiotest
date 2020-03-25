#include <QCoreApplication>
#include <QUrl>

#include "mdksupport.h"
#include "mdk/Player.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    registerMediaIoClasses();

    MDK_NS::Player player;

//    player.setMedia("localfile:///D:\\BigRingVR\\NZ_Pori_back_4K.mp4");
#if defined Q_OS_MAC
    QUrl localfile = QUrl::fromLocalFile("/users/ibooij/Documents/Big-Ring/Videos/Italy_Passo_Sella_North.mp4");
#else
    QUrl localfile = QUrl::fromLocalFile("d:/BigRingVR/NZ_Pori_back_4K.mp4");
#endif
    player.setMedia(qPrintable(localfile.toString()));

    player.setState(MDK_NS::PlaybackState::Playing);

    return a.exec();
}
