#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QAudioOutput>
#include <QMediaPlayer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow();

private slots:
    void on_dirBtn_clicked();

    void on_playBtn_clicked();

    void on_preBtn_clicked();

    void on_nextBtn_clicked();

    void on_volumeBtn_clicked();

    void on_playList_doubleClicked(const QModelIndex &);

    void on_playRuleBox_currentIndexChanged(int);

private:
    void nextMusic() const;

    void randomIndex();

    Ui::MainWindow *m_p_client;
    QList<QUrl> m_play_list;
    QAudioOutput *m_p_audio_output;
    QMediaPlayer *m_p_media_player;
    QSet<QString> m_dir_set;
    int m_cur_index = 0;
    int m_play_rule = 0;
    int m_cur_volume = 0;
};
#endif // MAINWINDOW_H
