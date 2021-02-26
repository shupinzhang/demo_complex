#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
#ifdef _DEV_DEBUG_
    w.show();
#else
    w.showFullScreen();
#endif
    return a.exec();
}
