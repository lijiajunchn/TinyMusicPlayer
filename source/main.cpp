#include "../head/mainwindow.h"
#include <filesystem>
#include <QApplication>

int main(int argc, char *argv[])
{
    if (!std::filesystem::exists("Music"))
    {
        std::filesystem::create_directory("Music");
    }
    QApplication a(argc, argv);
    CMainWindow w;
    w.show();
    return a.exec();
}
